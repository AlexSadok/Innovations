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

#define WORK_TIME 100

//-------------------------------------------------------------------------------------

void StopAction(int i)
{
    _exit(1);
}

void SleepController(int _Percent, char** argv)
{
    if((_Percent < 0) || (_Percent > 100))
    {
        perror("Incorrect arguments");
        _exit(1);
    }

    int process = fork();

    if(process == -1)
    {
        perror("Creating process");
        _exit(1);
    }

    // В дочернем процессе
    if(process == 0)
    {
        printf("Here1\n");

        int ExecResult = execvp(argv[2], argv + 2);

        if(ExecResult == -1)
        {
            perror("Error in launching programm");
            _exit(1);
        }
    }

    // В родительском процессе
    else
    {
        printf("Here2\n");

        int time_wait_parent = WORK_TIME * _Percent;
        int time_wait_child = WORK_TIME * (100 - _Percent);

        struct sigaction q;
        q.sa_handler = StopAction;
        q.sa_flags = SA_NOCLDSTOP;

        sigaction(SIGCHLD, &q, NULL);

        while(1)
        {
            // Приостановить дочерний процесс
            kill(process, SIGSTOP);
            usleep(time_wait_child);

            // Возобновить дочерний процесс
            kill(process, SIGCONT);

            // Приостановить родительский процесс
            usleep(time_wait_parent);
        }
    }
}

//-------------------------------------------------------------------------------------

int main(int argc, char** argv)
{
    if(argc < 3)
    {
        perror("Incorrect argument number");
        _exit(1);
    }
    else
        SleepController(atoi(argv[1]), argv);

    return 0;
}
