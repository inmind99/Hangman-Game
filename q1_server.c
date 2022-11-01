#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>

int countLines(char*);
char** readFile(char*, int*);
void deallocate(char**,int);
void initWord(char*,char*);
int updateWord(char, char*, char*);
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

int main(int argc, char** argv)
{   
    signal(SIGUSR1, action); //associated action method with the signal
    int pid = getpid(); 

    srand(time(0));
    if(argc!=2)
    {
        printf("Error! Provide file name as command line arguments.");
    }

    int size;
    char** dict = readFile(argv[1],&size); //reading file dictionary and storing it in 2d array

    key_t key_;
    key_ = ftok("file", 65);
    int msgid = msgget(key_, 0666 | IPC_CREAT); //created message queue

    msgrcv(msgid, &message, sizeof(message), 1, 0); //receive hi from client
    printf("%s\n",message.d);
    char msg[100];
    strcpy(msg,message.d);
    
    struct Data* data;

    if(!strcmp(message.d,"hi"))
    {
        int key=shmget(13546,1024,IPC_CREAT|IPC_EXCL|0666); //creating shared memory
	    data=(struct Data*) shmat(key,NULL,0);

        strcpy(message.d,"13546"); 
        message.mesg_type=1;

        int r=rand()%100;
        printf("Word selected: %s\n",dict[r]); //selecting random word from dictionary
        data->letters_left=strlen(dict[r])-2;
        data->attempts_left=3;
        initWord(data->word,dict[r]);
        data->pid=pid;
        msgsnd(msgid, &message, sizeof(message), 0); //sending shared memory to client

        pause(); //waiting for signal from clients

        int client_pid=data->pid;
        int lettersGuessed=0;
        while(1==1)
        {
            lettersGuessed=0;
            lettersGuessed=updateWord(data->letter,data->word,dict[r]); //updating word if correct letter guessed

            if(lettersGuessed > 0)
            {
                data->letters_left-=lettersGuessed; //updating letters left if letters guessed correctly
            }
            else
            {
                data->attempts_left-=1; //otherwise decrementing attempts left
            }

            kill(client_pid, SIGUSR1 ); //signaling client for new guess

            if(data->attempts_left == 0 || data->letters_left == 0) 
            {
                break; //if user wins or loses then break the loop else continue doing it
            }

            pause();//waiting for signal from clients
        }
    }

    deallocate(dict,size); //deallocating memory assigned to double pointer of dictionary
    return 0;
}

int countLines(char* fileName) //just a helper function to count no of lines in the file so the pointer can be allocated memory
{
    FILE* ptr = fopen(fileName,"r");
    if(ptr == NULL)
    {
        printf("Error!");   
        exit(1);             
    }

    char* buffer=malloc(sizeof(char)*100);
    int count = 0;
    int max=100;

    while ((getline(&buffer, (size_t*)&max, ptr)) != -1) {
        count++;
    }

    fclose(ptr);
    free(buffer);

    return count;
}

char** readFile(char* fileName, int* size) //function to read file
{
    *size=countLines(fileName);

    FILE* ptr = fopen(fileName,"r");
    if(ptr == NULL)
    {
        printf("Error!");   
        exit(1);             
    }

    char** dictionary;
    dictionary=malloc(sizeof(char*) * (*size)); //allocating memory to the double pointer

    char* buffer=malloc(sizeof(char)*100);
    int x=0;
    int max=100;

    while((getline(&buffer, (size_t*)&max, ptr)) != -1)
    {
        if(buffer[strlen(buffer)-1]=='\n')
        {
            buffer[strlen(buffer)-1]='\0'; //adding null char instead of next line char
        }
        dictionary[x]=malloc(sizeof(char)*strlen(buffer)); //allocating memory to the pointer
        strcpy(dictionary[x],buffer); //copying in dictionary from the buffer
        x++;
    }

    fclose(ptr);

    return dictionary;
}

void deallocate(char** ptr,int size) //function to deallocate
{
    for(int x=0;x<size;x++)
    {
        free(ptr[x]);
    }
    free(ptr);
}

void initWord(char* wrd, char* word) //func to init word that is choosen randomly
{
    int size=strlen(word);
    for(int x=0;x<size;x++)
    {
        if(x==0 || x==size-1)
        {
            wrd[x]=word[x]; //last and first letter of the word will be the original letters
        }
        else
        {
            wrd[x]='_'; //all other will be masked as _
        }
    }
    wrd[size]='\0'; //adding null char at the end
}

int updateWord(char l, char* unfilled, char* original) //function to update the word
{
    int count = 0;
    for (int x=0;x<strlen(original);x++)
    {
        if(original[x]==l && unfilled[x]=='_') //write the original letter instead of _ if guessed correctly
        {
            unfilled[x]=l;
            count++;
        }
    }
    return count;
}

void action() //just a function for signal
{
    return;
}