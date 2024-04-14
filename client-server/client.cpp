#include <sys/types.h>
#include <sys/socket.h>
#include "errproc.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <iostream>
#include <unistd.h>

int main(){
	
	int fd = Socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in adr = {0};
	adr.sin_family = AF_INET;
	adr.sin_port = htons(34543);
	Inet_pton(AF_INET, "127.0.0.1", &adr.sin_addr);
	Connect(fd, (struct sockaddr *)&adr, sizeof adr);
	write(fd, "Hello\n", 6);
	char buf[256];
	ssize_t nread;
	nread = read(fd, buf, 256);
	if(nread == -1){
		perror("read failed");
		exit(1);
	}
	if(nread == 0){
		std::cout<<"eof\n";
	}
	
	close(fd);
	return 0;
}
