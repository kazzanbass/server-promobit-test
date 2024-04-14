#include <sys/types.h>
#include <sys/socket.h>
#include "errproc.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <iostream>
#include <unistd.h>

int main(){
	
	int fd = Socket(AF_INET, SOCK_STREAM, 0); //Присваиваем дескриптор сокета fd
	struct sockaddr_in adr = {0};//инициализируем структуру соккета 0
	adr.sin_family = AF_INET;//Семейство протоколов IPv4
	adr.sin_port = htons(34543);//Порт 34543
	Inet_pton(AF_INET, "127.0.0.1", &adr.sin_addr);//Присваиваем ip адрес нашего компьютера структуре
	Connect(fd, (struct sockaddr *)&adr, sizeof adr);//Подключаемся к сокету(структуре) сервера
	write(fd, "Hello\n", 6); //отправляем сообщение
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
