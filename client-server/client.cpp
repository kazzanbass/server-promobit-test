#include "errproc.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <iostream>
#include <unistd.h>

using namespace std;

int main(){
		
        int fd = Socket(AF_INET, SOCK_STREAM, 0); //Присваиваем дескриптор сокета fd
        struct sockaddr_in adr = {0};//инициализируем структуру соккета 0
        adr.sin_family = AF_INET;//Семейство протоколов IPv4
        adr.sin_port = htons(54343);//Порт 8080
        Inet_pton(AF_INET, "127.0.0.1", &adr.sin_addr);//Присваиваем ip адрес нашего компьютера структуре
        Connect(fd, (struct sockaddr *)&adr, sizeof adr);//Подключаемся к серверу      
        char message[128];
        cout<<"Введите сообщение(128 байт) ==> ";
        cin>>message;//Сообщение серверу
        write(fd, message, 128); //отправляем сообщение
		char buf[128];
        read(fd, buf, 128);//Читаем сообщение от сервера
        cout<<buf<<endl;//Выводим ответ на консоль
        close(fd);
        
        return 0;
}


