#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <netdb.h>
#include <signal.h>
#include "client.h" 

#define PORT 9999

char message[200];
bool exitClient= false;
int sockfd;

void intHandle(int sig)
{
    bzero(message, sizeof(message));
    strcpy(message, "exit");
    send(sockfd , message , strlen(message) , 0);
    exitClient= true;
    exit(1);
}

void *sendM(void *sockfd)
{
    int nsockfd= *(int*)sockfd;
    printf("Enter message : \n");

    while (!exitClient)
    { 
        bzero(message, strlen(message));
        fgets(message, sizeof(message), stdin);
        fseek(stdin, 0, SEEK_END);
        if (strlen(message) > 106)
        {
            printf("invalid operation!\n");
            bzero(message, strlen(message));
            printf("Enter message : \n");
            sleep(2);
            continue;
        }
        if (strcmp(message, "exit\n")== 0)
        {
            send(nsockfd , message , strlen(message) , 0);
            exitClient= true;
            break;
        } 
        if (send(nsockfd , message , strlen(message) , 0) == -1)
        {
            perror("Send failed");
            exitClient= true;
            break;
        }

        sleep(2);
        printf("Enter message : \n");
    }
    pthread_exit(0);
}

void *recT(void *sockfd)
{
    int nsockfd= *(int*)sockfd;
    char server_reply[2000];

    while (!exitClient)
    {    
        if (recv(nsockfd , server_reply , 2000 , 0) <= 0)
        { 
            printf("you've been disconnected from the server\n"); 
            exitClient= true; 
            exit(0);
        }

        puts("Server reply :");
        puts(server_reply);
    }
    pthread_exit(0);
}

int main(int argc , char *argv[])
{
    struct sockaddr_in server;
    struct hostent *host;

    signal(SIGINT, intHandle);

    if (argc < 2 || (host= gethostbyname(argv[1]))== NULL)
    {
        perror("error retrieving hostname");
        return 0;
    }

    sockfd = socket(AF_INET , SOCK_STREAM , 0);
    if (sockfd == -1)
        perror("Could not create socket");
    else
    	printf("socket has been created!\n");
    
   // bzero(server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr = *((struct in_addr *)(host-> h_addr));
 
    while (connect(sockfd, (struct sockaddr *)&server, sizeof(struct sockaddr)) < 0)
    {
        printf("connect error, retrying in 3 seconds\n");
        sleep(3);
    }

    printf("connected!\n");

    pthread_t sendThread;
    pthread_t recThread;

    if (pthread_create(&sendThread, NULL, sendM, (void *) &sockfd) < 0)
    {
        perror("could not create thread");
    } 

    if (pthread_create(&recThread, NULL, recT, (void *) &sockfd) < 0)
    {
        perror("could not create thread");
    }

    pthread_join(recThread, NULL);
    pthread_join(sendThread, NULL);
  

    close(sockfd);
     
    return 0;
}