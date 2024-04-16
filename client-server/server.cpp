#include "errproc.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <iostream>
#include <fstream>
#include <unistd.h>

using namespace std;

int main(int argc, char**argv){
	
	if(argc<2){
		perror("PORT ERROR");//Если порт в argv[1] не указан завершаем программу
		exit(1);
	}
	
	const int PORT = atoi(argv[1]); //Сохраняем номер порта в PORT
	std::string PORT_STR = argv[1];
	PORT_STR+=".txt";//Переделываем в текст для файла
	
	std::ofstream mes; //создаем поток ввода
	
	int servfd = Socket(AF_INET, SOCK_STREAM, 0);//Создаем сокет сервера
	struct sockaddr_in adr = {0};//Создаем структуру адреса
	adr.sin_family = AF_INET;//IPv4
	adr.sin_port = htons(PORT);//Порт
	Bind(servfd, (struct sockaddr *) &adr, sizeof(adr));//Присваиваем сокету адрес
	socklen_t addrlen = sizeof(adr);//для accept
	Listen(servfd, 5);//сообщаем ОС о прослушке
	
	while(true){//Пока нет сигнала выключению сервера
		int clifd = Accept(servfd, (struct sockaddr *) &adr, &addrlen); //Ожидаем подключение от клиента
		char buf[128];
		read(clifd, buf, 128);//Считываем сообщение клиента в буфер
		mes.open(PORT_STR); //
		mes << buf;			//Записываем сообщение последнего клиента в файл
		mes.close();		//
		
		sleep(3);//Ожидаем 3 секунды
		write(clifd, "ACCEPTED\0", 9);//Отправляем клиенту сообщение
	}
		
	close(servfd);//Закрываем сокет сервера
	
	return 0;
}
