#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

//----------------------------------------------------------

int main(int argc, char** argv)
{
    int sockfd, newsockfd;                  // Дескрипторы слушающего и присоединенного сокетов
    int clilen;                             // Длина адреса клиента
    int n;                                  // Количествопринятых символов

    char sendline[1000], recvline[1000];    // Массивы для отсылаемой и принятой строки
    struct sockaddr_in servaddr, cliaddr;   // Структуры для размещения полных адресов сервера и клиента

    // Создаем TCP-сокет
    sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
    {
        perror("Creating TCP-socket");
        _exit(1);
    }

    /* Заполняем структуру для адреса сервера: семейство протоколов TCP/IP,
    сетевой интерфейс - любой, номер порта - 51000
    и обнуляем всю структуру */
    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = PF_INET;
    servaddr.sin_port = htons(51000);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Настраиваем адрес сокета
    if(bind(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr)) < 0)
    {
        perror("Bind socket");
        close(sockfd);
        _exit(1);
    }

    /* Переводим созданный сокет в слушающее состояние. Глубину очереди для
    установленных соединений описываем значением 5 */
    if(listen(sockfd,5) < 0)
    {
        perror("Listen socket");
        close(sockfd);
        _exit(1);
    }

    // Основной цикл сервера
    while(1)
    {
        /* В переменную clilen заносим максимальную длину
        ожидаемого адреса клиента */
        clilen = sizeof(cliaddr);

        /* Ожидаем полностью установленного соединения на слушающем сокете.
        При нормальном зарершении в структуре cliaddr будет лежать полный адреc клиента,
        установившего соединение, а в переменной clilen - его фактическая длина.
        Вызов вернет дескриптор присоединенного сокета, через который будет происходить
        общение с клиентом */
        if( (newsockfd = accept(sockfd, (struct sockaddr*) &cliaddr, &clilen)) < 0 )
        {
            perror("Creating new socket");
            close(sockfd);
            _exit(1);
        }

        /* В цикле принимаем информацию от клиента и пересылаем обратно до тех пор,
        пока не произойдет ошибки (вызов read вернет отрицательное значение)
        или клиент не закроет соединение (вызов read вернет значение 0).
        Максимальную длину одной порции данных от клиента ограничим 999 символами. */
        while(1)
        {
            if( (n = read(newsockfd, recvline, 999)) <= 0 )
                break;

            if( (n = write(newsockfd, recvline, strlen(recvline) + 1)) < 0)
            {
                perror("Error in writing");
                close(sockfd);
                close(newsockfd);
                _exit(1);
            }
        }

        // Если при чтении возникла ошибка - завершаем работу
        if(n < 0)
        {
            perror("Error in reading");
            close(sockfd);
            close(newsockfd);
            _exit(1);
        }

        // Закрываем дескриптор присоединенного сокета и уходим ожидать нового соединения
        close(newsockfd);
    }

    return 0;
}
