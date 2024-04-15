#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <iostream>
#include <unistd.h>

using namespace std;

int main(){
		
        int fd = socket(AF_INET, SOCK_STREAM, 0); //Присваиваем дескриптор сокета fd
        struct sockaddr_in adr = {0};//инициализируем структуру соккета 0
        adr.sin_family = AF_INET;//Семейство протоколов IPv4
        adr.sin_port = htons(8080);//Порт 8080
        inet_pton(AF_INET, "127.0.0.1", &adr.sin_addr);//Присваиваем ip адрес нашего компьютера структуре
        int err = connect(fd, (struct sockaddr *)&adr, sizeof adr);//Подключаемся к серверу
        if(err==-1){//Если подключение не удается выходим из программы
			perror("Не удалось подключиться к серверу");
			exit(1);
		}
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


