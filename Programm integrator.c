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

#define MAX_PATH_LENGTH 256

//-------------------------------------------------------------------------------------

// Интеграл от функции f(x) = 1 на отрезке [a;b]
double GetIntegral(double a, double b)
{
    return b - a;
}

double GetFinalResults(int num)
{
    double Integral = 0;

    int i;
    for(i = 0; i < num; ++i)
    {
        // Формируем адрес файла для считывания результата
        char* Destination = malloc(MAX_PATH_LENGTH);
        char* TempBuf = malloc(20);

        strcpy(Destination, "./file");
        sprintf(TempBuf, "%d", i);
        strcat(Destination, TempBuf);

        int fd = open(Destination, O_RDONLY);
        int ReadResult = read(fd, TempBuf, 20);

        if(ReadResult == -1)
        {
            perror("Read file");
            _exit(1);
        }

        double TempResult = strtod(TempBuf, NULL);
        Integral += TempResult;
    }

    return Integral;
}

// Интеграл от функции f(x) = 1 на отрезке [SegmentBegin;SegmentEnd]
int Integrate(int num)
{
    double SegmentBegin = 0;
    double SegmentEnd = 10;

    // Массив для хранения pid дочерних процессов
    int* ProcessArray = (int*)malloc(num * sizeof(int));

    // Цикл по всем подотрезкам
    int i;
    for(i = 0; i < num; ++i)
    {
        int process = fork();

        if(process == -1)
        {
            perror("Creating process");
            _exit(1);
        }

        // В дочернем процессе
        if(process == 0)
        {
            double a;               // Начало подотрезка
            double b;               // Конец подотрезка
            double IntegrateResult; // Значение интеграла на подотрезке [a;b]

            double SegmentLength = SegmentEnd - SegmentBegin;

            a = SegmentBegin + i * (SegmentLength / num);
            b = SegmentBegin + (i + 1) * (SegmentLength / num);

            IntegrateResult = GetIntegral(a,b);

            // Формируем адрес файла для записи результата
            char* Destination = malloc(MAX_PATH_LENGTH);
            char* TempBuf = malloc(20);

            strcpy(Destination, "./file");
            sprintf(TempBuf, "%d", i);
            strcat(Destination, TempBuf);

            // Записываем результат в файл
            int fd = open(Destination, O_CREAT | O_WRONLY, S_IRWXU);

            if(fd == -1)
            {
                perror("Open file\n");
                _exit(1);
            }


            sprintf(TempBuf, "%f", IntegrateResult);

            // Записываем результат в файл
            int WriteResult = write(fd, TempBuf, strlen(TempBuf));

            if (WriteResult == -1)
            {
                perror("Write to file\n");
                _exit(1);
            }

            free(TempBuf);
            free(Destination);
            close(fd);

            return 0;
        }
        // Конец дочернего процесса

        ProcessArray[i] = process;

    }

    for(i = 0; i < num; i ++)
        waitpid(ProcessArray[i], NULL, 0);

    double Integral = GetFinalResults(num);

    printf("Integral = %f\n", Integral);

    return 0;
}

int main(int argc, char** argv)
{
    if(argc != 2)
    {
        perror("Uncorrect arguments number");
        _exit(1);
    }
    else
    {
        int NumProcess = atoi(argv[1]); // Количество процессов
        Integrate(NumProcess);
    }

    return 0;
}
