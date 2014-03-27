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

#define MAX_PATH_LENGTH 256

//-------------------------------------------------------------------------------------

// Считывание данных из файла в буфер
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

// Запись данных в файл из буфера
int WriteFile(int _fd, void* _buf, int _size)
{
    int write_bytes = write(_fd, _buf, _size);

    if(write_bytes == -1)
    {
        perror("Write file");
        return -1;
    }

    if(write_bytes != _size)
    {
        perror("Write file");
        return -1;
    }

    return write_bytes;
}

// Побайтовое копирование содержимого файла1 в файл2
void CopyFileContent(int fd1, int fd2)
{
    const int size = 20;        // Размер буфера
    void* buf;                  // Буфер
    int read_bytes;             // Текущее число считанных байтов
    int write_bytes;            // Текущее число записанных байтов

    buf = malloc(size);

    for(;;)
    {
        read_bytes = ReadFile(fd1, buf, size);

        if( (read_bytes == 0) || (read_bytes == -1) )
            break;

        write_bytes = WriteFile(fd2, buf, read_bytes);

        if(write_bytes == -1)
            break;
    }

    free(buf);
}

//------------------------------------------------------------------

// Проверка, является ли файл директорией
int IsDirectory(char* file_name)
{
    struct stat buf;
    int _stat = stat(file_name, &buf);

    if(_stat == -1)
        return -1;

    mode_t st_mode = buf.st_mode;

    if(S_ISDIR(st_mode))
        return 1;

    return 0;
}

//------------------------------------------------------------------

// Создание файла для записи
int CreateCopyFile(char* file1_name, char* file2_name)
{
    struct stat buf;

    stat(file1_name, &buf);

    mode_t st_mode = buf.st_mode;       // Режим доступа
    uid_t st_uid = buf.st_uid;          // Идентификатор пользователя-владельца
    gid_t st_gid = buf.st_gid;          // Идентификатор группы-владельца

    int fd2;

    fd2 = open(file2_name, O_RDWR | O_CREAT, st_mode);

    if(fd2 == -1)
        perror("Open file 2");

    if(fd2 == -1)
        return -1;

    chown(file2_name, st_uid, st_gid);

    return fd2;
}

// Формирование пути создаваемого файла
char* CreatePath(char* _argv1, char* _argv2, int _IsDir, char* _Destination)
{
    // Выделяем из полного имени файла название файла
    int length = strlen(_argv1);
    int size = length;

    while(_argv1[size] != '/' && size > 0)
        size--;

    int i;
    char file_name[length - size];

    for(i = 0; i < length - size; ++i)
        file_name[i] = _argv1[size + 1 + i];
    file_name[length - size] = '\0';

    // Если argv2 - директория
    if(_IsDir == 1)
    {
        strcpy(_Destination, _argv2);

        int length = strlen(_Destination);
        if(_Destination[length - 1] != '/')
            strcat(_Destination, "/");

        strcat(_Destination, file_name);
    }

    // Если argv2 - файл
    else
    {
        strcpy(_Destination, _argv2);
    }

    return _Destination;
}

//------------------------------------------------------------------

// Копирование одного файла в другой
// либо копирование нескольких файлов в директорию
void Copy(int argc, char** argv)
{
    int IsDir;  // Флаг директории последнего аргумента

    // Проверка, является ли последний аргумент директорией
    if(IsDirectory(argv[argc - 1]))
        IsDir = 1;
    else
        IsDir = 0;

    // В файл можно копировать только 1 файл
    if( (IsDir == 0) && (argc != 3) )
    {
        perror("Uncorrect arguments number");
        _exit(1);
    }

    int fd1;                    // Файловый дескриптор копируемого файла
    int fd2;                    // Файловый дескриптор конечного файла
    int i;                      // Счетчик

    // Цикл по всем файлам
    for(i = 1; i < argc - 1; ++i)
    {
        fd1 = open(argv[i], O_RDONLY);

        // Если текущий файл не удалось открыть
        if(fd1 == -1)
            printf("Error: Open file %d\n", i);

        // Если текущий файл удалось открыть
        else
        {
            // Адрес файла для копирования
            char* Destination = malloc(MAX_PATH_LENGTH);

            // Формирование адреса
            CreatePath(argv[i], argv[argc - 1], IsDir, Destination);

            fd2 = CreateCopyFile(argv[i], Destination);

            // Если конечный аргумент невозможно открыть
            if(fd2 == -1)
            {
                perror("Open destination file");
                _exit(1);
            }
            else
                CopyFileContent(fd1, fd2);

            free(Destination);
        }

        close(fd1);
    }
}

// Создание новой директории по уже существующей
int MakeDir(char* _dr1, char* _dr2)
{
    struct stat buf;
    stat(_dr1, &buf);

    mode_t st_mode = buf.st_mode;   // Режим доступа

    return mkdir(_dr2, st_mode);
}

//------------------------------------------------------------------

void CopyDir(char* _dir1, char* _dir2)
{
    // Адрес созданной директории
    // либо NULL, если директория уже существовала
    // необходимо для проверки на зацикливание
    char* NewDirectory = malloc(MAX_PATH_LENGTH);

    strcpy(NewDirectory, "");

    DIR* current_dir = opendir(_dir1);

    if(current_dir == NULL)
        perror("Open directory");

    else
    {
        // Создание новой директории
        if( opendir(_dir2) == NULL )
        {
            int make_dir = MakeDir(_dir1, _dir2);

            strcpy(NewDirectory, _dir2);

            if(make_dir == -1)
                perror("Make a directory");
        }

        // Считывание файлов в директории
        char* file_name;
        struct dirent* _dirent;

        for(;;)
        {
            _dirent = readdir(current_dir);
            if(_dirent == NULL)
            {
                closedir(current_dir);
                break;
            }

            file_name = _dirent->d_name;

            char* Destination = malloc(MAX_PATH_LENGTH);

            strcpy(Destination, _dir1);

            int length = strlen(Destination);
            if(Destination[length - 1] != '/')
                strcat(Destination, "/");
            strcat(Destination, file_name);

            // Копирование файла в директорию
            if(IsDirectory(Destination) == 0)
            {
                int argc = 3;
                char* argv[argc];

                argv[1] = Destination;
                argv[2] = _dir2;

                Copy(argc, argv);
            }
            // Копирование директории в директорию
            else
            {
                // Пропускаем директории . и ..
                // и совпадение старой и новой директорий (от зацикливания)
                if ( (strcmp(file_name,".") != 0) && (strcmp(file_name,"..") != 0)
                    && (strcmp(NewDirectory, Destination) != 0) )
                {
                    char* Destination2 = malloc(MAX_PATH_LENGTH);

                    strcpy(Destination2, _dir2);

                    int length = strlen(Destination2);
                    if(Destination2[length - 1] != '/')
                        strcat(Destination2, "/");
                    strcat(Destination2, file_name);

                    CopyDir(Destination, Destination2);

                    free(Destination2);
                }
            }

            free(Destination);
        }
    }
}

//------------------------------------------------------------------

void Copy_R(int argc, char** argv)
{
    if(argc != 4)
    {
        perror("Uncorrect arguments number");
        _exit(1);
    }

    if( (IsDirectory(argv[argc - 1]) == 0) || (IsDirectory(argv[argc - 2]) == 0) )
    {
        perror("File is not a directory");
    }
    else
    {
        char* dir1 = argv[argc - 2];
        char* dir2 = argv[argc - 1];

        CopyDir(dir1, dir2);
    }
}

//------------------------------------------------------------------

int main(int argc, char** argv)
{
    if(argc < 3)
    {
        perror("Uncorrect arguments number");
        _exit(1);
    }

    if(strcmp(argv[1], "-R"))
        Copy(argc, argv);
    else
        Copy_R(argc, argv);

    return 0;
}
