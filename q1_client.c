#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

void action();

struct Data { //struct for the communication of server and client
    int letters_left;
    char word[100];
    int attempts_left;
    int pid;
    char letter;
};

struct mesg_buffer { //struct used in message queues
    long mesg_type;
    char d[1024];
} message; 

int main()
{
    signal(SIGUSR1, action); //associated action method with the signal
    key_t key;
    int msgid;

    int my_pid=getpid(); //getting process id of this process

    key = ftok("file", 65);
    msgid = msgget(key, 0666 | IPC_CREAT); //created message queue
    message.mesg_type=1;
    strcpy(message.d,"hi");

    msgsnd(msgid, &message, sizeof(message), 0); //sending hi to server

    msgrcv(msgid, &message, sizeof(message), 1, 0); //receiving shared memory key from the server
    printf("Data Received is : %s \n", message.d);
    int x_key=atoi(message.d);

    int key_=shmget(x_key,1024,0); //accessing shared memory
	struct Data* data=(struct Data *) shmat(key_,NULL,0); 

    printf("Letters Left: %d\n",data->letters_left);
    printf("Attempts Left: %d\n",data->attempts_left);
    printf("Word is: %s\n", data->word);
    int server_pid=data->pid;

    printf("Enter your guess: ");
    scanf("%c",&(data->letter)); //getting guess from the user
    getchar(); //just to ignore \n character
    data->pid=my_pid;
    kill( server_pid, SIGUSR1 ); //sending signal to server

    while(1==1)
    {
        pause(); //waiting for signal from the server

        if(data->attempts_left == 0) //if user loses then exit loop
        {
            break;
        }

        if(data->letters_left == 0) //if user wins then print congrats and exit loop
        {
            printf("You win! :D Congratulations!!!\n");
            break;
        }

        printf("Letters Left: %d\n",data->letters_left);
        printf("Attempts Left: %d\n",data->attempts_left);
        printf("Word is: %s\n", data->word);
        printf("Enter your guess: "); 
        scanf("%c",&(data->letter)); //getting guess from the user
        getchar();
        kill( server_pid, SIGUSR1 ); //signaling server that user has entered the guess and updated the shared memory
    }

    shmdt(data); 
	shmctl(key_,IPC_RMID,NULL);
    return 0;
}

void action() //just a function for signal
{
    return;
}