#include "errproc.h"
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(){
	write(STDOUT_FILENO, "Сервер запущен\n", 29);
	int server = Socket(AF_INET, SOCK_STREAM, 0);//присваиваем дескриптор сокета server
	struct sockaddr_in adr = {0};//инициализируем структуру данных
	adr.sin_family = AF_INET; //семейство протоколов IPv4
	adr.sin_port = htons(34543);//присваиваем сокету порт 34543
	Bind(server, (struct sockaddr *)&adr, sizeof adr);//привязываем сокет к структуре (порт)
	Listen(server, 5);//Прослушиваем порт до 5 клиентов
	socklen_t adrlen = sizeof adr;//инициализируем адрес структуры
	int fd = Accept(server, (struct sockaddr *)&adr, &adrlen);//присваиваем дескриптор сокета клиента
	ssize_t nread;//размер посланного клиентом сообщения
	char buf[256];
	nread = read(fd, buf, 256);//записываем сообщение клиента в буфер
	if(nread==-1){
		perror("read error");
		exit(1);
	}
	if(nread==0){
		printf("END OF FILE\n");
	}
	write(STDOUT_FILENO, buf, nread);
	write(fd, buf, nread);
	close(fd);//Закрываем сокет клиента
	close(server); //Закрываем сокет сервера
	
	return 0;
}
	
