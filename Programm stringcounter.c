#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <unistd.h>

#include <errno.h>
#include <fcntl.h>
#include <wait.h>

#define STD_OUT 1

//-------------------------------------------------------------------------------------

int StringCount(char** argv)
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
        // Создаем дополнительный дескриптор для STD_OUT
        int oldstdout = dup(STD_OUT);

        // Перенаправляем стандартный вывод в файл
        dup2(fd[1], STD_OUT);

        close(fd[1]);

        int ExecResult = execvp(argv[1], argv + 1);

        if(ExecResult == -1)
        {
            perror("Error in launching programm");
            _exit(1);
        }

        // Закрыть второй дескриптор STD_OUT
        close(oldstdout);

        return 0;
    }

    int StringNumber = 0;
    char* TempBuf = malloc(1);

    while(1)
    {
        int ReadResult = read(fd[0], TempBuf, 1);

        if(ReadResult == -1)
        {
            perror("Read file");
            _exit(1);
        }

        if(ReadResult == 0)
        {
            // БЛОКИРОВКА!!!
            break;
        }

        if(TempBuf[0] == '\n')
            StringNumber++;
    }

    close(fd[0]);

    printf("StringNumber: %d\n", StringNumber);

    return 0;
}

//-------------------------------------------------------------------------------------

int main(int argc, char** argv)
{
    if(argc < 1)
    {
        perror("Incorrect argument number");
        _exit(1);
    }
    else
    {
        int Result = StringCount(argv);
    }

    return 0;
}

