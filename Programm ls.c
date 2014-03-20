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

void ShowST_MODE (mode_t st_mode)
{
// Вывод типа файла
    if(S_ISLNK(st_mode))  printf("%s", "l");
    if(S_ISREG(st_mode))  printf("%s", "-");
    if(S_ISDIR(st_mode))  printf("%s", "d");
    if(S_ISCHR(st_mode))  printf("%s", "c");
    if(S_ISBLK(st_mode))  printf("%s", "b");
    if(S_ISFIFO(st_mode)) printf("%s", "p");
    if(S_ISSOCK(st_mode)) printf("%s", "s");

// Вывод прав для владельца

    if(st_mode & S_IRUSR) printf("%s", "r"); else printf("%s", "-");
    if(st_mode & S_IWUSR) printf("%s", "w"); else printf("%s", "-");
    if(st_mode & S_IXUSR) printf("%s", "x"); else printf("%s", "-");

// Вывод прав для группы

    if(st_mode & S_IRGRP) printf("%s", "r"); else printf("%s", "-");
    if(st_mode & S_IWGRP) printf("%s", "w"); else printf("%s", "-");
    if(st_mode & S_IXGRP) printf("%s", "x"); else printf("%s", "-");

// Вывод прав для остальных пользователей

    if(st_mode & S_IROTH) printf("%s", "r"); else printf("%s", "-");
    if(st_mode & S_IWOTH) printf("%s", "w"); else printf("%s", "-");
    if(st_mode & S_IXOTH) printf("%s", "x"); else printf("%s", "-");
}

void ShowST_NLINK(nlink_t st_nlink)
{
    printf(" %d ", (int)st_nlink);
}

void ShowST_UID(uid_t st_uid)
{
    struct passwd* PASSWD = getpwuid(st_uid);
    char* user_name = PASSWD->pw_name;

    printf("%s ", user_name);
}

void ShowST_GID(gid_t st_gid)
{
    struct group* GROUP = getgrgid(st_gid);
    char* group_name = GROUP->gr_name;

    printf("%s ", group_name);
}

void ShowST_SIZE(off_t st_size)
{
    printf("%d ", (int)st_size);
}

void ShowST_MODTIME(time_t st_modtime)
{
    struct tm *timeinfo;
    char buffer[80];

    timeinfo = localtime(&st_modtime);
    strftime(buffer, 80, "%h. %d %R", timeinfo);

    printf("%s ", buffer);
}

void ShowST_NAME(char* st_name)
{
    printf("%s ", st_name);
}

//-------------------------------------------------------------------------------------

// Вывод информации о файле
int ShowFileInfo(const char* start_dir_name, char* file_name)
{
    char* dir_name = malloc(strlen(start_dir_name) + strlen(file_name) + 1);
    strcpy(dir_name, start_dir_name);

    char* full_name = strcat(strcat(dir_name, "/"), file_name);

    struct stat buf;

    int _stat = stat(full_name, &buf);

    if(_stat == -1)
    {
        perror("Open file");
        return 0;
    }

    mode_t st_mode = buf.st_mode;       // Режим доступа
    nlink_t st_nlink = buf.st_nlink;    // Количество жестких ссылок
    uid_t st_uid = buf.st_uid;          // Идентификатор пользователя-владельца
    gid_t st_gid = buf.st_gid;          // Идентификатор группы-владельца
    off_t st_size = buf.st_size;        // Размер (в байтах)
    time_t st_modtime = buf.st_mtime;   // Время последней модификации
    char* st_name = file_name;          // Имя файла

    ShowST_MODE (st_mode);
    ShowST_NLINK(st_nlink);
    ShowST_UID(st_uid);
    ShowST_GID(st_gid);
    ShowST_SIZE(st_size);
    ShowST_MODTIME(st_modtime);
    ShowST_NAME(st_name);

    free(dir_name);
    printf("\n");

    return 1;
}


//-------------------------------------------------------------------------------------

int main(int argc, char** argv)
{
    // Проверка на корректность количества аргументов
    // (1 аргумент - адрес директории)
    if(argc != 2)
    {
        perror("Uncorrect arguments number");
        _exit(1);
    }

    // Проверка на корректность открытия директории
    const char* start_dir_name = argv[1];

    DIR* _dir = opendir(start_dir_name);

    if(_dir == NULL)
    {
        perror("Open directory");
        _exit(1);
    }

    // Считывание файлов в директории
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

        if( (strcmp(name,".") != 0) && (strcmp(name,"..") != 0) )
            ShowFileInfo(start_dir_name, name);
    }

    return 0;
}
