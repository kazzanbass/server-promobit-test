#include "errproc.h"
#include <liburing.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <cstring>
#include <cassert>
#include <ctime>

#define MAX_CONNECTIONS 4096 // Максимальное количество подключений
#define BACKLOG 512			 // Кол-во подключений к сокету
#define MAX_MESSAGE_LEN 128	 // Максимальная длина сообщения

using namespace std;

void add_accept(struct io_uring *ring, int fd, struct sockaddr *client_addr, socklen_t *client_len); // слушаем подключения к сокету

void add_socket_read(struct io_uring *ring, int fd, size_t size, string PORT); // читаем сообщение от клиента

void add_socket_write(struct io_uring *ring, int fd); // отправляем клиенту ACCEPTEDz

typedef struct conn_info
{				   // информация о сокете
	int fd;		   // дескриптор сокета
	unsigned type; // тип состояния в котором находится сокет(Accept, read, write)
} conn_info;

enum
{
	ACCEPT,
	READ,
	WRITE,
};

conn_info conns[MAX_CONNECTIONS]; // буфер для соединений

void timer()
{ // таймер
	clock_t end_time = clock() + 3 * CLOCKS_PER_SEC;
	while (clock() < end_time)
	{
	}
}

int main(int argc, char **argv)
{

	if (argc < 2)
	{
		perror("PORT ERROR"); // Если порт в argv[1] не указан завершаем программу
		exit(1);
	}

	const int PORT = atoi(argv[1]); // Сохраняем номер порта в PORT
	std::string PORT_STR = argv[1];
	PORT_STR += ".txt"; // Переделываем в текст для файла

	// std::ofstream mes; //создаем поток ввода

	int servfd = Socket(AF_INET, SOCK_STREAM, 0); // Создаем сокет сервера

	struct sockaddr_in adr = {0};	   // Создаем структуру адреса сервера
	struct sockaddr_in cliadr = {0};   // создаем структуру адреса клиента
	socklen_t clilen = sizeof(cliadr); // размер cliadr

	adr.sin_family = AF_INET;	// IPv4
	adr.sin_port = htons(PORT); // Порт

	Bind(servfd, (struct sockaddr *)&adr, sizeof(adr)); // Присваиваем сокету адрес

	Listen(servfd, 5); // сообщаем ОС о прослушке

	// Создаем инстанс io_uring
	struct io_uring ring;
	struct io_uring_params params = {0};

	assert(io_uring_queue_init_params(4096, &ring, &params) >= 0); // Устанавливаем параметры вхождения, емкость как 4096

	// Добавляем в SQ первую операцию - слушаем сокет сервера для приема входящих соединений
	add_accept(&ring, servfd, (struct sockaddr *)&cliadr, &clilen);

	while (true)
	{ // Пока нет сигнала выключению сервера

		struct io_uring_cqe *cqe;
		int ret;

		io_uring_submit(&ring);				  // Отправляем ядру все SQE, которые были добавлены в прошлой итерации
		ret = io_uring_wait_cqe(&ring, &cqe); // ждем когда в буфере появится cqe
		assert(ret == 0);

		struct io_uring_cqe *cqes[BACKLOG];													  // буфер для выполненных cqe
		int cqe_count = io_uring_peek_batch_cqe(&ring, cqes, sizeof(cqes) / sizeof(cqes[0])); // количество cqe

		for (int i = 0; i < cqe_count; i++)
		{
			cqe = cqes[i];

			struct conn_info *user_data = (struct conn_info *)io_uring_cqe_get_data(cqe); // Заранее присвоили
			//																				//user_data указатель на структуру с
			// информацией о сокете
			unsigned type = user_data->type; // Определяем операцию к которой относится CQE(accept/recv/send)

			switch (type)
			{
			case ACCEPT:
			{
				int sock_conn_fd = cqe->res;
				// Если появилось новое соединение: добавляем в SQ операцию recv - читаем из клиентского сокета
				add_socket_read(&ring, sock_conn_fd, MAX_MESSAGE_LEN, PORT_STR);
				add_accept(&ring, servfd, (struct sockaddr *)&cliadr, &clilen); // продолжаем слушать серверный сокет
			}
			break;
			case READ:
			{
				add_socket_write(&ring, user_data->fd); // добавляем в sq операцию send - отправляем accept клиенту
			}
			break;
			case WRITE:
			{ // Запись в клиентский сокет окончена: добавляем в sq операцию recv - читаем из клиентского сокета
				add_socket_read(&ring, user_data->fd, MAX_MESSAGE_LEN, PORT_STR);
				break;
			}
			}

			io_uring_cqe_seen(&ring, cqe); // помечаем cqe как прочитанное
		}
	}

	return 0;
}

// Помещаем операцию accept в SQ, fd - дескриптор сокета, на котором принимаем соединение
void add_accept(struct io_uring *ring, int fd, struct sockaddr *client_addr, socklen_t *client_len)
{ // слушаем подключения к сокету
	cout << "Accept NOW!!!" << endl;
	// получаем указатель на первый доступный sqe
	struct io_uring_sqe *sqe = io_uring_get_sqe(ring);

	io_uring_prep_accept(sqe, fd, client_addr, client_len, 0); // помещаем операцию accept в sqe

	// устанавливаем состояние серверного сокета в ACCEPT
	conn_info *conn_i = &conns[fd];
	conn_i->fd = fd;
	conn_i->type = ACCEPT;

	// устанавливаем в поле user_data указатель на socketInfo соответсвующий серверному сокету
	io_uring_sqe_set_data(sqe, conn_i);
	cout << "Accept END!!!" << endl;
}

// Помещаем операцию recv в SQ, записываем данные в файл
void add_socket_read(struct io_uring *ring, int fd, size_t size, string PORT)
{
	cout << endl
		 << "Read NOW!" << endl;
	ofstream out;									   // Создаем поток вывода
	struct io_uring_sqe *sqe = io_uring_get_sqe(ring); // получаем указатель на первый доступный sqe
	char buffer[MAX_MESSAGE_LEN];
	io_uring_prep_recv(sqe, fd, &buffer, size, 0); // читаем сообщение в буфер
	
	FILE *out;
	
	fo = fopen(PORT.cstr(), "w")
	fwrite(fo, buffer);
	fclose(fo);

	// устанавливаем состояние серверного сокета в read
	conn_info *conn_i = &conns[fd];
	conn_i->fd = fd;
	conn_i->type = READ;

	// Устанавливаем в поле user_data указатель на socketInfo, соответствующий клиентскому сокету
	io_uring_sqe_set_data(sqe, conn_i);

	cout << "Read END!" << endl;
}

void add_socket_write(struct io_uring *ring, int fd)
{
	cout << "Write NOW!" << endl;
	const char *ACCEPTED = "ACCEPTED\n\0";			   // Строка, которая будет передаваться клиенту
	struct io_uring_sqe *sqe = io_uring_get_sqe(ring); // Получаем указатель на первый доступный sqe

	// Устанавливаем состояния серверного сокета в write
	conn_info *conn_i = &conns[fd];
	conn_i->fd = fd;
	conn_i->type = WRITE;

	timer();									  // ожидаем 3 секунды
	io_uring_prep_send(sqe, fd, ACCEPTED, 10, 0); // отправляем accept
	io_uring_sqe_set_data(sqe, conn_i);			  // устанавливаем в поле user_data указатель на socketInfo соответствующий клиентскому сокету
	cout << "Write END!" << endl;
}
