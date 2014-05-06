#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

//----------------------------------------------------------

int main(int argc, char** argv)
{
    int sockfd;                             // Дескриптор сокета
    int n;                                  // Количество переданных или прочитанных символов
    char sendline[1000], recvline[1000];    // Массивы для отсылаемой и принятой строки
    struct sockaddr_in servaddr;            // Структура для адреса сервера

    if(argc != 2)
    {
        perror("Incorrect arguments number");
        _exit(1);
    }

    // Обнуляем символьные массивы
    bzero(sendline, 1000);
    bzero(recvline, 1000);

    // Создаем TCP-сокет
    if( (sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Creating TCP-socket");
        _exit(1);
    }

    /* Заполняем структуру для адреса сервера: семейство протоколов TCP/IP,
    сетевой интерфейс - из аргумента командной строки, номер порта - 51000
    и обнуляем всю структуру */
    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = PF_INET;
    servaddr.sin_port = htons(51000);

    if(inet_aton(argv[1], &servaddr.sin_addr) == 0)
    {
        perror("Invalid IP address");
        close(sockfd);
        _exit(1);
    }

    /* Устанавливаем логическое соединение через созданный сокет с сокетом
    сервера, адрес которого мы занесли в структуру */
    if(connect(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr)) < 0)
    {
        perror("Error in connecting to server");
        close(sockfd);
        _exit(1);
    }

    while(1)
    {
        printf("Client: ");
        fflush(stdin);
        fgets(sendline, 1000, stdin);

        if( (n = write(sockfd, sendline, strlen(sendline) + 1)) < 0)
        {
            perror("Error in writing");
            close(sockfd);
            _exit(1);
        }

        if( (n = read(sockfd, recvline, 999)) < 0)
        {
            perror("Error in reading");
            close(sockfd);
            _exit(1);
        }

        printf("Server: %s\n", recvline);
    }

    // Завершаем соединение
    close(sockfd);

    return 0;
}
