#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <unistd.h>

#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include <errno.h>
#include <fcntl.h>
#include <wait.h>
#include <signal.h>

#define WORK_TIME 100000

//-------------------------------------------------------------------------------------

// Функция побайтового считывания из файла
int ReadFile(int _fd, void* _buf, int _size)
{
    int read_bytes = read(_fd, _buf, _size);

    if(read_bytes == -1)
    {
        perror("Read file");
       return -1;
    }

    if( (read_bytes != _size) && (read(_fd, _buf, _size) != 0) )
    {
        perror("Read file");
        return -1;
    }

    return read_bytes;
}

//-------------------------------------------------------------------------------------

int FileStringCounter(char* file_name)
{
    int count = 0;

    int fd = open(file_name, O_RDONLY);
    if(fd == -1)
    {
        perror("Open file");
        return -1;
    }

    const int size = 1;         // Размер буфера
    char* buf;                  // Буфер для считывания символов
    int read_bytes;             // Число прочитанных байтов

    buf = malloc(size);

    for(;;)
    {
        read_bytes = ReadFile(fd, buf, size);

        if(read_bytes == -1)
            return -1;
        if(read_bytes == 0)
            break;

        char s = buf[0];

        if(s == '\n')
            count++;
    }

    free(buf);

    close(fd);

    return count;
}

int StringCount(char* file_name)
{
    int fd[2];

    pipe(fd);

    int process = fork();

    if(process == -1)
    {
        perror("Creating process");
        _exit(1);
    }

    // В дочернем процессе
    if(process == 0)
    {
        int count = FileStringCounter(file_name);

        char* TempBuf = malloc(20);
        sprintf(TempBuf, "%d", count);

        write(fd[1], TempBuf, strlen(TempBuf));

        close(fd[1]);

        return -1;
    }

    waitpid(process, NULL, 0);

    char* TempBuf = malloc(20);

    int ReadResult = read(fd[0], TempBuf, 20);
    if(ReadResult == -1)
    {
        perror("Read file");
        _exit(1);
    }

    int StringsNumber = atoi(TempBuf);

    close(fd[0]);

    return StringsNumber;
}

//-------------------------------------------------------------------------------------

int main(int argc, char** argv)
{
    if(argc != 2)
    {
        perror("Incorrect argument number");
        _exit(1);
    }
    else
    {
        int count = StringCount(argv[1]);

        if(count >= 0)
            printf("%d\n", count);
    }

    return 0;
}
