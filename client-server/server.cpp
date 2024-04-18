//Без задержки времени
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

#define MAX_CONNECTIONS 4096
#define BACKLOG 512
#define MAX_MESSAGE_LEN 128

using namespace std;

void add_accept(struct io_uring *ring, int fd, struct sockaddr *client_addr, socklen_t *client_len);

void add_socket_read(struct io_uring *ring, int fd, size_t size, string PORT);

void add_socket_write(struct io_uring *ring, int fd);

typedef struct conn_info{
	int fd;
	unsigned type;
} conn_info;

enum {
	ACCEPT,
	READ,
	WRITE,
};

conn_info conns[MAX_CONNECTIONS];

//char bufs[MAX_CONNECTIONS][MAX_MESSAGE_LEN];

void timer(){
	clock_t end_time = clock() + 3*CLOCKS_PER_SEC;
	while(clock()<end_time){}
}

int main(int argc, char**argv){
	
	if(argc<2){
		perror("PORT ERROR");//Если порт в argv[1] не указан завершаем программу
		exit(1);
	}
	
	const int PORT = atoi(argv[1]); //Сохраняем номер порта в PORT
	std::string PORT_STR = argv[1];
	PORT_STR+=".txt";//Переделываем в текст для файла
	
	//std::ofstream mes; //создаем поток ввода
	
	int servfd = Socket(AF_INET, SOCK_STREAM, 0);//Создаем сокет сервера
	
	struct sockaddr_in adr = {0};//Создаем структуру адреса сервера
	struct sockaddr_in cliadr = {0}; // создаем структуру адреса клиента
	socklen_t clilen = sizeof(cliadr);
	
	adr.sin_family = AF_INET;//IPv4
	adr.sin_port = htons(PORT);//Порт
	
	Bind(servfd, (struct sockaddr *) &adr, sizeof(adr));//Присваиваем сокету адрес
	//socklen_t addrlen = sizeof(adr);//для accept
	Listen(servfd, 5);//сообщаем ОС о прослушке
	
	struct io_uring ring;
	struct io_uring_params params = {0};;
	
	assert(io_uring_queue_init_params(4096, &ring, &params)>=0);//Устанавливаем параметры вхождения емкость как 4096
	
	add_accept(&ring, servfd, (struct sockaddr *) &cliadr, &clilen);
	
	while(true){//Пока нет сигнала выключению сервера
	//	int clifd = Accept(servfd, (struct sockaddr *) &adr, &addrlen); //Ожидаем подключение от клиента
		struct io_uring_cqe *cqe;
		int ret;
		
		io_uring_submit(&ring);
		ret = io_uring_wait_cqe(&ring, &cqe);
		assert(ret==0);
		
		struct io_uring_cqe *cqes[BACKLOG];
		int cqe_count = io_uring_peek_batch_cqe(&ring, cqes, sizeof(cqes)/sizeof(cqes[0]));
		
		for(int i=0; i<cqe_count; i++){
			cqe = cqes[i];
			
			struct conn_info *user_data = (struct conn_info *) io_uring_cqe_get_data(cqe);
			
			unsigned type = user_data->type;
			
			switch(type){
				case ACCEPT:{
					int sock_conn_fd = cqe->res;
					add_socket_read(&ring, sock_conn_fd, MAX_MESSAGE_LEN, PORT_STR);
					add_accept(&ring, servfd, (struct sockaddr *) &cliadr, &clilen);
					}break;
				case READ:{
						add_socket_write(&ring, user_data->fd);
					}break;
				case WRITE:{
					add_socket_read(&ring, user_data->fd, MAX_MESSAGE_LEN, PORT_STR);
					break;
				}
			}
			
			io_uring_cqe_seen(&ring, cqe);
		}
		//read(clifd, buf, 128);//Считываем сообщение клиента в буфер
	/*	mes << buf;			//Записываем сообщение последнего клиента в файл
		mes.close();		//
		sleep(3);//Ожидаем 3 секунды
		write(clifd, "ACCEPTED\0", 9);//Отправляем клиенту сообщение
		*/
	}
	
	return 0;
}

void add_accept(struct io_uring *ring, int fd, struct sockaddr *client_addr, socklen_t *client_len){
		cout<<"Accept NOW!!!"<<endl;
		struct io_uring_sqe *sqe = io_uring_get_sqe(ring);
		io_uring_prep_accept(sqe, fd, client_addr, client_len, 0);
		
		conn_info *conn_i = &conns[fd];
		conn_i->fd = fd;
		conn_i->type = ACCEPT;
		
		io_uring_sqe_set_data(sqe, conn_i);
		cout<<"Accept END!!!"<<endl;
}

void add_socket_read(struct io_uring *ring, int fd, size_t size, string PORT){
	cout<<endl<<"Read NOW!"<<endl;
	ofstream out;
	struct io_uring_sqe *sqe = io_uring_get_sqe(ring);
	char buffer[MAX_MESSAGE_LEN];
	io_uring_prep_recv(sqe, fd, &buffer, size, 0);
	
	//out.open(PORT, ios::app);
	//out<<buffer;
	//out.close();
	
	conn_info *conn_i = &conns[fd];
	conn_i->fd = fd;
	conn_i->type = READ;
	
	
	io_uring_sqe_set_data(sqe, conn_i);
	
	cout<<"Read END!"<<endl;
}

void add_socket_write(struct io_uring *ring, int fd){
	cout<<"Write NOW!"<<endl;
	const char* ACCEPTED = "ACCEPTED\n\0";
	struct io_uring_sqe *sqe = io_uring_get_sqe(ring);
	
	conn_info *conn_i = &conns[fd];
	conn_i->fd = fd;
	conn_i->type = WRITE;
	
	timer();
	io_uring_prep_send(sqe, fd, ACCEPTED, 10, 0);
	io_uring_sqe_set_data(sqe, conn_i);
	cout<<"Write END!"<<endl;
}
