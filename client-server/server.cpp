#include "errproc.h"
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

#define MAX_MESSAGE_LEN 128
#define MAX_CONNECTIONS 1024
#define BACKLOG 512

using namespace std;

typedef struct CQE_DAT{
	int fd;
	int type;
}CQE_DAT;

enum{
	ACCEPT,
	READ,
	FWRITE,
	SEND,
};

CQE_DAT CQE_DATAS[MAX_CONNECTIONS];
char BUFF[MAX_CONNECTIONS][MAX_MESSAGE_LEN];
int handle;

void SQE_ACCEPT(struct io_uring *ring, int fd, struct sockaddr *addr, socklen_t *addrlen);
void SQE_READ(struct io_uring *ring, int fd);
void SQE_FWRITE(struct io_uring *ring, int fd);
void SQE_SEND_ACCEPTED(struct io_uring *ring, int fd);


int main(int argc, char**argv){
	string PORTNUM;
	if(argc<2)
		PORTNUM = "8080";
	else
		PORTNUM = argv[1];
		
	int PORT = atoi(PORTNUM.c_str());
	
	string PORT_FILE = PORTNUM;
	PORT_FILE+=".txt";
	
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	
	struct sockaddr_in adr = {0};
	struct sockaddr_in cli_adr = {0};
	
	adr.sin_port = htons(PORT);
	adr.sin_family = AF_INET;
	
	socklen_t addrlen = sizeof(adr);
	socklen_t cli_addrlen = sizeof(cli_adr);
	
	int res = bind(sockfd, (struct sockaddr*)&adr, addrlen);
	assert(res!=-1);
	
	res = listen(sockfd, BACKLOG);
	assert(res == 0);
	
	FILE* fo;
	fo = fopen(PORT_FILE.c_str(), "w");
	fclose(fo);
	FILE* fo1;
	fo1 = fopen(PORT_FILE.c_str(), "a");
	handle = fileno(fo1);
	
	struct io_uring ring;
	struct io_uring_params params;
	memset(&params, 0, sizeof(params));
	
	res = io_uring_queue_init_params(1024, &ring, &params);
	assert(res==0);
	
	SQE_ACCEPT(&ring, sockfd, (struct sockaddr*)&cli_adr, &cli_addrlen);
	
	
	while(true){
		struct io_uring_cqe* cqe;
		
		res = io_uring_submit(&ring);
		
		res = io_uring_wait_cqe(&ring, &cqe);
		assert(res == 0);
		
		struct io_uring_cqe* cqes[BACKLOG];
		int cqe_count = io_uring_peek_batch_cqe(&ring, cqes, sizeof(cqes)/sizeof(cqes[0]));
		for(int i=0; i<cqe_count; i++){
			cqe = cqes[i];
			CQE_DAT* data = (CQE_DAT*)io_uring_cqe_get_data(cqe);
			int type = data->type;
			
			switch(type){
				case ACCEPT:{
					int fd = cqe->res;
					SQE_READ(&ring, fd);
					SQE_ACCEPT(&ring, sockfd, (struct sockaddr*)&cli_adr, &cli_addrlen);
					break;
				}
				case READ:{
					int fd = data->fd;
					//int bytes = cqe->res;
					SQE_FWRITE(&ring, fd);
					break;
				}
				case FWRITE:{
					int fd = data->fd;
					SQE_SEND_ACCEPTED(&ring, fd);
					break;
				}
				case SEND:{
					int fd = data->fd;
					SQE_READ(&ring, fd);
					break;
				}
			}
			io_uring_cqe_seen(&ring, cqe);
		}
	}
}

void SQE_ACCEPT(struct io_uring *ring, int fd, struct sockaddr *addr, socklen_t *addrlen){
	//cout<<"Accept now!"<<endl;
	struct io_uring_sqe *sqe = io_uring_get_sqe(ring);
	io_uring_prep_accept(sqe, fd, addr, addrlen, 0);
	
	CQE_DAT* data = &CQE_DATAS[fd];
	data->fd = fd;
	data->type = ACCEPT;
	
	io_uring_sqe_set_data(sqe, data);
}

void SQE_READ(struct io_uring *ring, int fd){
	struct io_uring_sqe *sqe = io_uring_get_sqe(ring);
	io_uring_prep_read(sqe, fd, &BUFF[fd], MAX_MESSAGE_LEN, 0);
	
	CQE_DAT* data = &CQE_DATAS[fd];
	data->fd = fd;
	data->type = READ;
	
	io_uring_sqe_set_data(sqe, data);
}

void SQE_FWRITE(struct io_uring *ring, int fd){
	struct io_uring_sqe *sqe = io_uring_get_sqe(ring);
	char *message = BUFF[fd];
	size_t message_len = strlen(message);
	io_uring_prep_write(sqe, handle, &BUFF[fd], message_len, 0);
	
	CQE_DAT* data = &CQE_DATAS[fd];
	data->fd = fd;
	data->type = FWRITE;
	
	io_uring_sqe_set_data(sqe, data);
}

void SQE_SEND_ACCEPTED(struct io_uring *ring, int fd){
	struct io_uring_sqe *sqe = io_uring_get_sqe(ring);
	char *ACCEPTED = "ACCEPTED\n";
	size_t size = strlen(ACCEPTED);
	
	io_uring_prep_write(sqe, fd, ACCEPTED, size, 0);
	
	CQE_DAT* data = &CQE_DATAS[fd];
	data->fd = fd;
	data->type = SEND;
	
	io_uring_sqe_set_data(sqe, data);
}


