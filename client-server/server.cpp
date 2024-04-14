#include "errproc.h"
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(){
	write(STDOUT_FILENO, "Сервер запущен\n", 29);
	int server = Socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in adr = {0};
	adr.sin_family = AF_INET;
	adr.sin_port = htons(34543);
	Bind(server, (struct sockaddr *)&adr, sizeof adr);
	Listen(server, 5);
	socklen_t adrlen = sizeof adr;
	int fd = Accept(server, (struct sockaddr *)&adr, &adrlen);
	ssize_t nread;
	char buf[256];
	nread = read(fd, buf, 256);
	if(nread==-1){
		perror("read error");
		exit(1);
	}
	if(nread==0){
		printf("END OF FILE\n");
	}
	write(STDOUT_FILENO, buf, nread);
	write(fd, buf, nread);
	close(fd);
	close(server);
	
	return 0;
}
	
