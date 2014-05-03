#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <errno.h>
#include <assert.h>

#include <pthread.h>
#include <time.h>

#define PHILOS 5
#define FOOD 100

//--------------------------------------------------------------------------------

void *philosopher (void *id);
void grab_chopstick (int, int, char*);
void down_chopsticks (int, int);
int food_on_table ();
int get_token ();
void return_token ();

pthread_mutex_t chopstick[PHILOS];
pthread_mutex_t print;
pthread_t philo[PHILOS];
pthread_mutex_t food_lock;
pthread_mutex_t num_can_eat_lock;
int num_can_eat = PHILOS - 1;

//--------------------------------------------------------------------------------

int main (int argc, char **argv)
{
    int i;

    pthread_mutex_init (&food_lock, NULL);
    pthread_mutex_init (&num_can_eat_lock, NULL);

    for (i = 0; i < PHILOS; i++)
        pthread_mutex_init (&chopstick[i], NULL);

    int* mas =  malloc(PHILOS);
    for (i = 0; i < PHILOS; i++)
        mas[i] = i;

    for (i = 0; i < PHILOS; i++)
        pthread_create (&philo[i], NULL, philosopher, &mas[i]);

    for (i = 0; i < PHILOS; i++)
        pthread_join (philo[i], NULL);

    return 0;
}

//--------------------------------------------------------------------------------

void* philosopher (void *num)
{
    struct timeval Start;
    struct timeval End;
    long Sum_Wait_Time;

    int id;
    int left_chopstick, right_chopstick;

    id = *(int*)num;

    printf ("Philosopher %d: ready to eat.\n", id);

    right_chopstick = id;
    left_chopstick = id + 1;

    // Вилки расположены по кругу
    if (left_chopstick == PHILOS)
        left_chopstick = 0;

    int f;
    int sleep_time;

    // Пока не закончилась еда
    while (f = food_on_table ())
    {
        gettimeofday(&Start, NULL);

        srand(time(NULL));
        sleep_time = 100000 * (rand() % 5 + 1);
        usleep(sleep_time);

        get_token ();

        grab_chopstick (id, right_chopstick, "right ");
        grab_chopstick (id, left_chopstick, "left");

        printf ("Philosopher %d: eating.\n", id);
        down_chopsticks (left_chopstick, right_chopstick);
        return_token ();

        gettimeofday(&End, NULL);

        long wait_time = 1000000 * (End.tv_sec - Start.tv_sec) + (End.tv_usec - Start.tv_usec);
        Sum_Wait_Time += wait_time;
        printf("Philosopher %d: waited %ld ms.\n", id, wait_time);
    }

    printf ("Philosopher %d: finished eating.\n", id);

    sleep(1);
    printf ("Philosopher %d: summary waited %ld ms.\n", id, Sum_Wait_Time);

    return (NULL);
}

//--------------------------------------------------------------------------------

int food_on_table ()
{
    static int food = FOOD;
    int myfood;

    pthread_mutex_lock (&food_lock);

    if (food > 0)
        food--;

    myfood = food;
    pthread_mutex_unlock (&food_lock);

    return myfood;
}

// Философ phil захватил вилку с номерном c и (левую/правую)
void grab_chopstick (int phil, int c, char* hand)
{
    pthread_mutex_lock (&chopstick[c]);
    printf ("Philosopher %d: got %s chopstick %d\n", phil, hand, c);
}

// Освободить вилки c1, c2
void down_chopsticks (int c1, int c2)
{
    pthread_mutex_unlock (&chopstick[c1]);
    pthread_mutex_unlock (&chopstick[c2]);
}

//--------------------------------------------------------------------------------

// Уменьшить число доступных философов
int get_token ()
{
    int successful = 0;

    while (successful == 0)
    {
        pthread_mutex_lock (&num_can_eat_lock);

        if (num_can_eat > 0)
        {
            num_can_eat--;
            successful = 1;
        }
        else
        {
            successful = 0;
        }

        pthread_mutex_unlock (&num_can_eat_lock);
    }
}

// Увеличить число доступных философов
void return_token ()
{
    pthread_mutex_lock (&num_can_eat_lock);
    num_can_eat++;
    pthread_mutex_unlock (&num_can_eat_lock);
}
