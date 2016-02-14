#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include "server.h"

bool printing= false;

int findAccount(char *name)
{
	int count= 0;
	while (count < 20)
	{
		if (strcmp(actArr[count].accountName, "\0")== 0)
		{
			return count;
		} 
		else if (strcmp(actArr[count].accountName, name)== 0)
		{
			return count;
		}
		count++;
	}
	//if returning 21, then you've reached the max number of accounts that can be created.
	return 21;
}

void readLine(char *input, int clientfd)
{
	char operation[6];
	char string[100];
	sscanf(input, "%s %s", operation, string);
	int i=0;

	if (strcmp(operation, "open")== 0)
	{
		pthread_mutex_lock(&mutex_open_1);
		while (printing)
		{
			sleep(1);
		}

		i= findAccount(string);
		if (i== 21) 
		{
			write(clientfd, "You've reached the max number of accounts!", 50);
		} 
		else if (strcmp(actArr[i].accountName, "\0")!= 0) 
		{
			write(clientfd, "There is already an account with this name!", 50);
		} 
		else 
		{
			strcpy(actArr[i].accountName, string);
			actArr[i].balance= 0;
			actArr[i].inSession= false;
			write(clientfd, "Account has been created!", 50);
		}
		pthread_mutex_unlock(&mutex_open_1);
	}
	else if (strcmp(operation, "exit")== 0) 
	{
		write(clientfd, "exiting", 10);
		pthread_exit(0);
	} 
	else 
	{
		write(clientfd, "this is not a valid operation!", 50);
	}
	bzero(string, strlen(string));
	bzero(operation, strlen(operation));
}

void *createThread(void* fd)
{
	int clientfd, currentAcct= 0;
	char buffer[106]; // max number or character should be accountname+" "+"start"
    char operation[6], string[100];
    int temp= 0;

    //mutex stuff
    pthread_mutex_init(&mutex_open_1, NULL);
    pthread_mutex_init(&mutex_credit_2, NULL);
    pthread_mutex_init(&mutex_debit_3, NULL);
    pthread_mutex_init(&mutex_balance_4, NULL);

    char buf[100];

	clientfd= *((int *)fd); //cast it back to an int
    
   	while (true)
   	{	
   		bzero(operation, sizeof(operation));
		bzero(string, sizeof(string));
		bzero(buffer, sizeof(buffer));
   		read(clientfd, buffer, 106);
   		sscanf(buffer, "%s %s", operation, string);

   		if (strcmp(operation, "start")== 0)
   		{
   			currentAcct= findAccount(string); // index of the current account
   			if (currentAcct== 21 || strcmp(actArr[currentAcct].accountName, "\0")== 0)
   			{
   				write(clientfd, "this account does not exist!", 50);
   				continue;
   			}
   			if (actArr[currentAcct].inSession== true)
   			{
   				write(clientfd, "there is already a client in session with this account!", 60);
   				continue;
   			}
   			actArr[currentAcct].inSession= true;
   			write(clientfd, "Account is now in session!", 50);
   			while (true)
   			{
   				bzero(buf, sizeof(buf));
   				bzero(buffer, sizeof(buffer));
   				read(clientfd, buffer, 106);
   				sscanf(buffer, "%s %s", operation, string);

   				if (strcmp(operation, "credit")== 0)
   				{
   					pthread_mutex_lock(&mutex_credit_2);
   					if (atof(string) <= 0)
   					{
   						write(clientfd, "you must enter a number greater than 0!", 60);
   					}
   					else 
   					{
	   					actArr[currentAcct].balance += atof(string);
	   					int temp= snprintf(buf, 100, "your balance is : $%.2f", actArr[currentAcct] .balance);
	   					buf[temp]= '\0';
	   					write(clientfd, buf, sizeof(buf));
   					}
   					pthread_mutex_unlock(&mutex_credit_2);
   				}
   				else if (strcmp(operation, "debit")== 0)
   				{
   					pthread_mutex_unlock(&mutex_debit_3);
   					if (atof(string) > actArr[currentAcct].balance)
   					{
   						write(clientfd, "You don't have enough funds to withdraw this amount!", 100);
   					} 
   					else if (atof(string) <= 0)
   					{
   						write(clientfd, "you must enter a number greater than 0!", 60);
   					}
   					else 
   					{
   						actArr[currentAcct] . balance -= atof(string);
   						int temp= snprintf(buf, 100, "your balance is : $%.2f", actArr[currentAcct] .balance);
	   					buf[temp]= '\0';
	   					write(clientfd, buf, sizeof(buf));
   					}
   					pthread_mutex_unlock(&mutex_debit_3);
   				}
   				else if (strcmp(operation, "balance")== 0)
   				{
   					pthread_mutex_lock(&mutex_balance_4);
   					temp= snprintf(buf, 100, "your balance is : $%.2f", actArr[currentAcct] .balance);
   					buf[temp]= '\0';
   					write(clientfd, buf, sizeof(buf));
   					pthread_mutex_unlock(&mutex_balance_4);
   				}
   				else if (strcmp(operation, "finish")== 0)
   				{
   					actArr[currentAcct].inSession= false;
   					write(clientfd, "your session has be disconnected.  Please start a session with another account", 100);
   					break;
   				}
   				else if (strcmp(operation, "exit")== 0)
   				{
   					actArr[currentAcct].inSession= false;
   					write(clientfd, "disconnecting from server", 50);
            pthread_exit(0);
   				}
   				else
   				{
   					write(clientfd, "this is not a valid operation while in session.", 50);
   				}
   			}
   		}
   		else
		{
			readLine(buffer, clientfd);
		}
    }
    pthread_exit(0);
}

void *printAccounts(void *stoof)
{	
    while (1)
    {
        int i= 0;
        bool first= true;
        while (i != 20)
        {
        	printing= true;
            if (strcmp(actArr[i].accountName, "\0")== 0)
            {
                i++;
                continue;
            }
            if (first)
            {
    	        	printf("\n");
    	        	first= false;
            }
                printf("Account Name:%s\tBalance:%.2f\tIn Session:", actArr[i].accountName, actArr[i].balance);
            if (actArr[i].inSession== true)
            {
                printf("true\n");
            }
            else
            {
                printf("false\n");
            }
            i++;
        }
        printing= false;
        int timer= 20;
        while (timer != 0) 
        {
            timer= sleep(timer);
        }
    }
}

void *acceptConnections(void *stuff)
{
	int sockfd, clientfd;
    struct sockaddr_in server, client;
    int len= sizeof(struct sockaddr_in);

    sockfd = socket(AF_INET , SOCK_STREAM , 0);
    if (sockfd == -1)
        perror("Could not create socket");
    else
    	printf("socket has been created!\n");

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(9999);

 	//bind socket to port
    if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
    	perror("error with bind"); 
    	return 0;
    }
    else
    {
    	printf("bind successful!\n");
    }

    int option= 0;
    if (setsockopt(sockfd, SOL_SOCKET, (SO_REUSEADDR), (char *)&option, sizeof(option)) < 0)
    {
    	perror("setsockopt failed");
    }
    
    if (listen(sockfd, 20) != 0)
    {
     	perror("error with listen");
     	exit(0);
    } 
    else
    {
    	puts("now listening...");
    }

   pthread_t thread; //maximum of 20 threads to be created
   //int i= 0; //the number of threads that needs to be opened

    while ((clientfd= accept(sockfd, (struct sockaddr*)&client, (socklen_t*)&len)))
    {
	   // max number or character should be accountname+" "+"start"
	   printf("connection successful!\n");
	    // if (i < 20)
	    // {
	      if (pthread_create(&thread, NULL, createThread, &clientfd) < 0)
	    	{
	            perror("could not create thread");
	    	}
	    	//i++;
	    //}
    }

    if (clientfd <= 0)
    {
    	perror("error accepting\n");
    }

   // int j= 0;

   //  while (j != i)
   //  {
  	// 	pthread_join(threadArr[j], NULL); 
  	// 	j++;
   //  }
    pthread_join(thread, NULL);

    close(clientfd);
    close(sockfd);
    pthread_exit(0);
}

int main(int argc, char **argv)
{
	signal(SIGPIPE, SIG_IGN);

	int m= 0;
    while (m != 20)
    {
   		actArr[m].accountName[0]= '\0';
   		m++;
    }

    pthread_t pAccounts;

    if (pthread_create(&pAccounts, NULL, printAccounts, (void *) NULL) < 0)
	{
        perror("could not create thread");
	}

    pthread_t acceptorThread;

    if (pthread_create(&acceptorThread, NULL, acceptConnections, (void *) NULL) < 0)
	{
        perror("could not create thread");
	}

	pthread_join(acceptorThread, NULL);
	
	return 0;
}



























