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

//-------------------------------------------------------------------------------------

// Функция проверки вхождения подстроки pattern в строку string
int StringSearch(char* string, char* pattern)
{
    size_t string_length = strlen(string);
    size_t pattern_length = strlen(pattern);

    int Compare;

    Compare = 0;

    size_t i;
    size_t j;
    size_t count;

    if(string_length == 0)
        return 0;

    if(string_length < pattern_length)
        return 0;

    for(i = 0; i < string_length - pattern_length + 1; ++i)
    {
        count = 0;

        for(j = 0; j < pattern_length; ++j)
            if(string[i + j] == pattern[j])
                count++;

        if(count == pattern_length)
        {
            Compare = 1;
            break;
        }
    }

    return Compare;
}

//-------------------------------------------------------------------------------------

// Функция побайтового считывания из файла
int ReadFile(int _fd, void* _buf, int _size)
{
    int read_bytes = read(_fd, _buf, _size);

    if(read_bytes == -1)
    {
        perror("Read file");
        _exit(1);
    }

    if( (read_bytes != _size) && (read(_fd, _buf, _size) != 0) )
    {
        perror("Read file");
        _exit(1);
    }

    return read_bytes;
}

//-------------------------------------------------------------------------------------

void grep(char* file_name, char* str)
{
    // Проверка на корректность открытия файла
    int fd = open(file_name, O_RDONLY);
    if(fd == -1)
        perror("Open file");

    const int size = 1;         // Размер буфера
    void* buf;                  // Буфер для считывания символов
    char* StringBuffer;         // Буфер для хранения строки
    int read_bytes;             // Число прочитанных байтов

    int StringBuffer_size = 256;
    int i = 0;

    buf = malloc(size);
    StringBuffer = malloc(StringBuffer_size);

    for(;;)
    {
        read_bytes = ReadFile(fd, buf, size);

        if(read_bytes == 0)
            break;

        char* symbol = buf;
        char s = symbol[0];

        // Если это не символ конца строки
        if(s != 10)
        {
            // Если буфер для хранения строки переполнен
            if(i == StringBuffer_size)
            {
                StringBuffer_size = StringBuffer_size * 2;
                StringBuffer = realloc(StringBuffer, StringBuffer_size);
                i = 0;
            }

            StringBuffer[i] = s;
            i++;
        }
        // Если это символ конца строки
        else
        {
            if(StringSearch(StringBuffer, str) == 1)
                printf("%s\n", StringBuffer);

            StringBuffer = malloc(StringBuffer_size);
            i = 0;
        }
    }

    free(buf);
    free(StringBuffer);

    close(fd);
}

//-------------------------------------------------------------------------------------

void grep_l(char* dir_name, char* str)
{
    // Проверка на корректность открытия директории
    DIR* _dir = opendir(dir_name);

    if(_dir == NULL)
    {
        perror("Open directory");
        _exit(1);
    }

    // Считывание имен файлов в директории
    char* name;
    struct dirent* _dirent;

    for(;;)
    {
        _dirent = readdir(_dir);
        if(_dirent == NULL)
        {
            closedir(_dir);
            break;
        }

        name = _dirent->d_name;

        // Проверка на вхождение подстроки
        if(StringSearch(name, str) == 1)
            printf("%s\n", name);
    }
}

//-------------------------------------------------------------------------------------

int main(int argc, char** argv)
{
    // grep file_name string
    // grep -l directory string

    // Проверка на корректность числа аргументов
    if( (argc != 3) && (argc != 4) )
    {
        perror("Uncorrect arguments number");
        _exit(1);
    }

    char* file_name;
    char* str;

    // Выполнение функции grep
    if(strcmp(argv[1], "-l"))
    {
        file_name = argv[1];
        str = argv[2];

        grep(file_name, str);
    }
    // Выполнение функции grep -l
    else
    {
        file_name = argv[2];
        str = argv[3];

        grep_l(file_name, str);
    }

    return 0;
}
