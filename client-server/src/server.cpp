#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <cstring>
#include <liburing.h>
#include <assert.h>
#include <ctime>

#define MAX_MESSAGE_LEN 128 //Максимальная длина сообщения
#define MAX_CONNECTIONS 1024//Максимальное количество подключений
#define BACKLOG 512//Максимальное кол-во одновременных подключений

using namespace std;



typedef struct CQE_DAT{//Создаем структуру для пользовательской информации о выполненых задачах для подключения
	int fd;//сокет подключения
	int type;//тип операции, которую выполняет подключение
}CQE_DAT;

enum{//типы операций
	ACCEPT,//подтверждение подключения
	READ,//Запись сообщения в буфер
	FWRITE,//Запись из буфера в файл
	SEND,//отправляем клиенту "ACCEPTED"
};

CQE_DAT CQE_DATAS[MAX_CONNECTIONS];//Массив пользовательской информации о выполненных задачах размером в макс. кол-во подключений
char BUFF[MAX_CONNECTIONS][MAX_MESSAGE_LEN];//Для каждого подключения создаем буфер
int handle;//файловый дескриптор файла, в который будем вести запись

void SQE_ACCEPT(struct io_uring *ring, int fd, struct sockaddr *addr, socklen_t *addrlen);//задаем sqe подтверждения клиента
void SQE_READ(struct io_uring *ring, int fd);//читаем сообщения о клиенте в буфер
void SQE_FWRITE(struct io_uring *ring, int fd);//записываем сообщение от клиента в файл
void SQE_SEND_ACCEPTED(struct io_uring *ring, int fd);//отправляем клиенту accepted

int main(int argc, char**argv){
	/*Получаем из agrv[1] номер порта, создаем из полученного номера порта строку для создания файла
	 * и целочисленное значение для инициализации порта*/
	string PORTNUM;						
	if(argc<2)
		PORTNUM = "8080";//Если пользователь забыл внести номер порта, порт = 8080
	else
		PORTNUM = argv[1];
		
	int PORT = atoi(PORTNUM.c_str());
	
	string PORT_FILE = PORTNUM;
	PORT_FILE+=".txt";
	
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);//Создаем сокет
	
	
	struct sockaddr_in adr = {0};//Адрес серверного сокета
	struct sockaddr_in cli_adr = {0};//Адрес клиентского сокета
	
	adr.sin_port = htons(PORT);//устанавливаем порт
	adr.sin_family = AF_INET;//используем IPv4 для передачи данных
	
	socklen_t addrlen = sizeof(adr);//Размер адреса серверного сокета
	socklen_t cli_addrlen = sizeof(cli_adr);//Размер адреса клиентского сокета
	
	int res = bind(sockfd, (struct sockaddr*)&adr, addrlen);//Присваиваем адрес серверного сокета дескриптору сокета
	assert(res!=-1);
	
	res = listen(sockfd, BACKLOG);//Уведомляем ОС о прослушке
	assert(res == 0);
	
	FILE* fo;
	fo = fopen(PORT_FILE.c_str(), "w");//Создаем новый файл <port_num>.txt (для того, чтобы данные с предыдущего запуска стерлись
	fclose(fo);//Закрываем поток
	FILE* fo1;
	fo1 = fopen(PORT_FILE.c_str(), "a");//Открываем файл для дозаписи
	handle = fileno(fo1);//получаем файловый дескриптор файла <port_num.txt>
	
	struct io_uring ring;//создаем структуру io_uring
	struct io_uring_params params = {0};//создаем структуру io_uring_params и присваиваем всем полям 0
	
	res = io_uring_queue_init_params(1024, &ring, &params);//устанавливаем для нашего io_uring параметры
	assert(res==0);
	
	SQE_ACCEPT(&ring, sockfd, (struct sockaddr*)&cli_adr, &cli_addrlen);//Отправляем в SQ SQE задачу прослушивания сокета
	
	
	while(true){
		struct io_uring_cqe* cqe;//Создаем структуру cqe(Выполненные SQE)
		
		io_uring_submit(&ring);//Отправляем операционной системе все SQE для выполнения из прошлой итерации
								//Или если итерация была нулевой то отправляем SQE, который был записан до начала цикла
		
		res = io_uring_wait_cqe(&ring, &cqe);//Ожидаем первый выполненный экземпляр
		assert(res == 0);
		
		struct io_uring_cqe* cqes[BACKLOG];//Создаем массив указателей cqe
		int cqe_count = io_uring_peek_batch_cqe(&ring, cqes, sizeof(cqes)/sizeof(cqes[0]));//Присваиваем массиву все полученные cqe
																						   //функция возвращает кол-во полученных cqe
																						   //добавляем результат функции в переменную
		for(int i=0; i<cqe_count; i++){//для каждого полученного cqe 
			cqe = cqes[i];
			CQE_DAT* data = (CQE_DAT*)io_uring_cqe_get_data(cqe);//инициализируем пользовательскую информацию
			int type = data->type;//получаем тип выполненной операции
			
			switch(type){
				case ACCEPT:{//Если тип выполненной операции accept
					int fd = cqe->res;//возвращенное cqe из sqe accept вводит в поле res файловый дескриптор сокета
					SQE_READ(&ring, fd);//вносим в sq операцию чтения от клиента в буфер по указанному сокету 
					SQE_ACCEPT(&ring, sockfd, (struct sockaddr*)&cli_adr, &cli_addrlen);//вносим в sq операцию подтверждения сокета
					break;
				}
				case READ:{//Если тип выполненной операции read
					int fd = data->fd;//в поле получаем из data файловый дескриптор сокета из которого производилось чтение в буфер
					int bytes = cqe->res;//cqe передает информацию о прочитанных байтах
					if(bytes<=0){//Если прочитанных байтов нет
						shutdown(fd, SHUT_RDWR);//Выключаем сокет
					}else{//иначе
						SQE_FWRITE(&ring, fd);//вносим в sq операцию записи в файл сообщение из буфера
					}
					break;
				}
				case FWRITE:{//Если тип выполненной операции fwrite
					int fd = data->fd;//Получаем файловый дескриптор сокета из дата
					SQE_SEND_ACCEPTED(&ring, fd);//вносим в sq операцию отправки сообщения ACCEPTED клиенту
					break;
				}
				case SEND:{//Если тип выполненной операции SEND
					int fd = data->fd; 
					SQE_READ(&ring, fd);//вносим в sq операцию чтения в буфер из клиентского сокета
					break;
				}
			}
			io_uring_cqe_seen(&ring, cqe);//помечаем все cqe как просмотренные
		}
	}
}

void SQE_ACCEPT(struct io_uring *ring, int fd, struct sockaddr *addr, socklen_t *addrlen){
	struct io_uring_sqe *sqe = io_uring_get_sqe(ring);//инициализируем sqe первым полученным sqe
	io_uring_prep_accept(sqe, fd, addr, addrlen, 0);//настраиваем sqe как функцию accept
	
	CQE_DAT* data = &CQE_DATAS[fd]; //Записываем в данные подключения
	data->fd = fd;					//файловый дескриптор клиента, с которым совершена инструкция
	data->type = ACCEPT;			//вносим тип операции ACCEPT
	
	io_uring_sqe_set_data(sqe, data);//Заносим данные подключения в данные sqe, выполненное cqe будет иметь те же данные
}

void SQE_READ(struct io_uring *ring, int fd){
	struct io_uring_sqe *sqe = io_uring_get_sqe(ring);//инициализируем sqe первым полученным sqe
	io_uring_prep_read(sqe, fd, &BUFF[fd], MAX_MESSAGE_LEN, 0);//настраиваем sqe как функцию read, читаем сообщение от клиента
															   //в буфер[файловый дескриптор сокета]
	CQE_DAT* data = &CQE_DATAS[fd];//Записываем в данные подключения 
	data->fd = fd;				   //файловый дескриптор клиента, с которым совершена инструкция
	data->type = READ;			   //вносим тип операции READ
	
	io_uring_sqe_set_data(sqe, data);//Заносим данные подключения в данные sqe, выполненное cqe будет иметь те же данные
}

void SQE_FWRITE(struct io_uring *ring, int fd){
	struct io_uring_sqe *sqe = io_uring_get_sqe(ring);//инициализируем sqe первым полученным sqe
	char *message = BUFF[fd];//присваиваем message строку из буфера подключения
	size_t message_len = strlen(message);//длина строки
	io_uring_prep_write(sqe, handle, &BUFF[fd], message_len, 0);//записываем в файл строку
	
	CQE_DAT* data = &CQE_DATAS[fd];//Записываем в данные подключения 
	data->fd = fd;				   //Файловый дескриптор клиента, с которым совершена инструкция
	data->type = FWRITE;           //вносим тип операции FWRITE
	
	io_uring_sqe_set_data(sqe, data);//Заносим данные подключения в данные sqe, выполненное cqe будет иметь те же данные
}

void SQE_SEND_ACCEPTED(struct io_uring *ring, int fd){
	struct io_uring_sqe *sqe = io_uring_get_sqe(ring);//инициализируем sqe первым полученным sqe
	char *ACCEPTED = "ACCEPTED\n";
	size_t size = strlen(ACCEPTED);
	
	io_uring_prep_write(sqe, fd, ACCEPTED, size, 0);//настраиваем sqe как функцию write, отправляем клиенту ACCEPTED
	
	CQE_DAT* data = &CQE_DATAS[fd];//Записываем в данные подключения
	data->fd = fd;				   //Файловый дескриптор клиента, с которым совершена инструкция
	data->type = SEND;             // вносим тип операции SEND
	
	io_uring_sqe_set_data(sqe, data);//Заносим данные подключения в данные sqe, выполненное cqe будет иметь те же данные
}


