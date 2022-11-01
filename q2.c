#include <sys/shm.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

struct Data{
    char buffer[11];
    sem_t semaphore[2];
};

bool checkDot(char*);

int main()
{
    int key=shmget(789456,1024,IPC_CREAT|IPC_EXCL|0666);
    struct Data* data=(struct Data*) shmat(key,NULL,0);

    sem_init(&(data->semaphore[0]),1,1);
    sem_init(&(data->semaphore[1]),1,0);

    int pid=fork();

//-----------------------------------This part is the child process!--------------------------------------//
    if(pid==0)
    {
        data->buffer[10]='\0';
        bool flg=true;
        while(flg)
        {
            sem_wait(&(data->semaphore[0]));
            for(int x=0;x<10;x++)
            {
                data->buffer[x]=getchar();
                if(data->buffer[x]=='.'){
                    data->buffer[x+1]='\0';
                    flg=false;
                    break;
                }
            }
            sem_post(&(data->semaphore[1]));
        }
        return 0;
    }
//---------------------------------------------------------------------------------------------------------//
    FILE* ptr;
    ptr = fopen("file.txt","w");

    bool flag=true;

    while(flag)
    {
        sem_wait(&(data->semaphore[1]));
        fprintf(ptr,"%s",data->buffer);
        flag=checkDot(data->buffer);
        sem_post(&(data->semaphore[0]));
    }

    fclose(ptr);
    shmdt(data);
	shmctl(key,IPC_RMID,NULL);
    return 0;
}

bool checkDot(char* arr)
{
    for(int x=0;x<strlen(arr);x++)
    {
        if(arr[x]=='.')
        {
            return false;
        }
    }
    return true;
}