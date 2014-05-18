#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/mman.h>

#define SHARED_MEMORY_NAME /SHM_NAME_BUFFER
#define SHARED_MEMORY_SIZE 1000

#define EMPTY_BUF_SEMAPHORE_NAME /sem1
#define FULL_BUF_SEMAPHORE_NAME /sem2
#define MUTEX_SEMAPHORE_NAME /sem3

//-------------------------------------------------------------------------------------

int main(int argc, char** argv)
{
    shm_unlink("SHARED_MEMORY_NAME");

    sem_unlink("EMPTY_BUF_SEMAPHORE_NAME");
    sem_unlink("FULL_BUF_SEMAPHORE_NAME");
    sem_unlink("MUTEX_SEMAPHORE_NAME");

    // Создаем разделяемую память для хранения данных
    int shm_buf = shm_open("SHARED_MEMORY_NAME", O_CREAT | O_RDWR, 0777);
    if(shm_buf == -1)
    {
        perror("shm_create");
        return -1;
    }

    int ftr_buf = ftruncate(shm_buf, SHARED_MEMORY_SIZE);
    if(ftr_buf == -1)
    {
        perror("truncate");
        return -1;
    }

    // Общий размер буфера для записи
    int size = SHARED_MEMORY_SIZE / sizeof(int);

    // Создаем семафоры для исключения записи в переполненный буфер,
    // для считывания из пустого буфера и для взаимоисключения
    sem_t* empty_buf_sem = sem_open("EMPTY_BUF_SEMAPHORE_NAME", O_CREAT, 0777, size);
    sem_t* full_buf_sem = sem_open("FULL_BUF_SEMAPHORE_NAME", O_CREAT, 0777, 0);
    sem_t* mutex_sem = sem_open("MUTEX_SEMAPHORE_NAME", O_CREAT, 0777, 1);

    close(shm_buf);

    sem_close(empty_buf_sem);
    sem_close(full_buf_sem);
    sem_close(mutex_sem);

//-------------------------------------------------------------------------------------

    int process = fork();

    // Дочерний процесс (запись)
    if(process == 0)
    {
        // Открываем семафоры
        sem_t* empty_buf_sem = sem_open("EMPTY_BUF_SEMAPHORE_NAME", 0, 0777, size);
        sem_t* full_buf_sem = sem_open("FULL_BUF_SEMAPHORE_NAME", 0, 0777, 0);
        sem_t* mutex_sem = sem_open("MUTEX_SEMAPHORE_NAME", 0, 0777, 1);

        // Открываем разделяемую память
        int shm_buf = shm_open("SHARED_MEMORY_NAME", O_RDWR, 0777);
        if(shm_buf == 1)
        {
            perror("shm_open");
            return -1;
        }

        // Отображаем память
        void* addr = mmap(0, SHARED_MEMORY_SIZE, PROT_WRITE | PROT_READ, MAP_SHARED, shm_buf, 0);

        //--------------------------------------------------------------------------------------

        // Генерируем числа и записываем их в разделяемую память
        int i = 0;
        int number;

        void* TempBuf = malloc(sizeof(int));

        srand(time(NULL));

        for(i = 0; i < size; ++i)
        {
            number = 1 + rand() % 100;
            printf("write: %d\n", number);

            sem_wait(empty_buf_sem);
            sem_wait(mutex_sem);

            sprintf(TempBuf, "%d", number);
            memcpy( (int*)addr + i, (int*)TempBuf, sizeof(int) );

            sem_post(mutex_sem);
            sem_post(full_buf_sem);
        }

        printf("Finish writing\n");

        //--------------------------------------------------------------------------------------

        // Снимаем отображение
        munmap(addr, SHARED_MEMORY_SIZE);

        // Закрываем семафоры
        sem_close(empty_buf_sem);
        sem_close(full_buf_sem);
        sem_close(mutex_sem);

        return 0;
    }

    // Родительский процесс (чтение)
    else
    {
        // Открываем семафоры
        sem_t* empty_buf_sem = sem_open("EMPTY_BUF_SEMAPHORE_NAME", 0, 0777, size);
        sem_t* full_buf_sem = sem_open("FULL_BUF_SEMAPHORE_NAME", 0, 0777, 0);
        sem_t* mutex_sem = sem_open("MUTEX_SEMAPHORE_NAME", 0, 0777, 1);

        // Открываем разделяемую память
        int shm_buf = shm_open("SHARED_MEMORY_NAME", O_RDWR, 0777);
        if(shm_buf == 1)
        {
            perror("shm_open");
            return -1;
        }

        //--------------------------------------------------------------------------------------

        // Считываем числа и выводим на экран
        int number;
        char* TempBuf = malloc(sizeof(int));
        int i = 0;

        // Общий размер буфера
        int size = SHARED_MEMORY_SIZE / sizeof(int);

        // Отображаем память
        void* addr = mmap(0, SHARED_MEMORY_SIZE, PROT_WRITE | PROT_READ, MAP_SHARED, shm_buf, 0);

        for(i = 0; i < size; ++i)
        {
            sem_wait(full_buf_sem);
            sem_wait(mutex_sem);

            memcpy( (int*)TempBuf, (int*)addr + i, sizeof(int) );

            sem_post(mutex_sem);
            sem_post(empty_buf_sem);

            number = atoi(TempBuf);
            printf("read: %d\n", number);
        }

        printf("Finish reading\n");

        //--------------------------------------------------------------------------------------

        // Закрываем семафоры
        sem_close(empty_buf_sem);
        sem_close(full_buf_sem);
        sem_close(mutex_sem);
    }

    wait();

    shm_unlink("SHARED_MEMORY_NAME");

    sem_unlink("EMPTY_BUF_SEMAPHORE_NAME");
    sem_unlink("FULL_BUF_SEMAPHORE_NAME");
    sem_unlink("MUTEX_SEMAPHORE_NAME");


    return 0;
}
