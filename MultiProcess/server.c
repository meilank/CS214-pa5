#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/shm.h> 
#include <errno.h> 
#include <semaphore.h> 
#include <ctype.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "server.h"

void *printAccounts(void *arr)
{	
	actNode *actArr= (actNode *) arr;
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

int findAccount(char *name, actNode* actArr)
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

void readLine(char *input, int clientfd, actNode *actArr)
{
	char operation[6];
	char string[100];
	sscanf(input, "%s %s", operation, string);
	int i=0;

	sem_t *sem = sem_open(SEM, 0);

	if (strcmp(operation, "open")== 0)
	{
		while (printing)
		{
			sleep(1);
		}
		sem_wait(sem);
		i= findAccount(string, actArr);
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
		sem_post(sem);
	}
	else if (strcmp(operation, "exit")== 0) 
	{
		write(clientfd, "exiting", 10);
		return;
	} 
	else 
	{
		write(clientfd, "this is not a valid operation!", 50);
	}
	bzero(string, strlen(string));
	bzero(operation, strlen(operation));
}

void readOperations(actNode *actArr, int clientfd)
{
	char buffer[106]; // max number or character should be accountname+" "+"start"
	char operation[6], string[100];
	int temp= 0;

	char buf[100];

	sem_t *sem = sem_open(SEM, 0);

	while (true)
	{	
		bzero(operation, sizeof(operation));
		bzero(string, sizeof(string));
		bzero(buffer, sizeof(buffer));
		read(clientfd, buffer, 106);
		sscanf(buffer, "%s %s", operation, string);

		if (strcmp(operation, "start")== 0)
		{
			int currentAcct= findAccount(string, actArr); // index of the current account
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
					sem_wait(sem);
					if (atof(string) <= 0)
					{
						write(clientfd, "you must enter a number greater than 0!", 60);
					}
					else 
					{
						actArr[currentAcct].balance += atof(string);
						int temp= snprintf(buf, 100, "your balance is : $%.2f", actArr[currentAcct].balance);
						buf[temp]= '\0';
						write(clientfd, buf, sizeof(buf));
					}
					sem_post(sem);
				}
				else if (strcmp(operation, "debit")== 0)
				{
					sem_wait(sem);
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
						actArr[currentAcct].balance -= atof(string);
						int temp= snprintf(buf, 100, "your balance is : $%.2f", actArr[currentAcct].balance);
						buf[temp]= '\0';
						write(clientfd, buf, sizeof(buf));
					}
					sem_post(sem);
				}
				else if (strcmp(operation, "balance")== 0)
				{
					sem_wait(sem);
					temp= snprintf(buf, 100, "your balance is : $%.2f", actArr[currentAcct].balance);
					buf[temp]= '\0';
					write(clientfd, buf, sizeof(buf));
					sem_post(sem);
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
					return;
				}
				else
				{
					write(clientfd, "this is not a valid operation while in session.", 50);
				}
			}
		}
		else
		{
			readLine(buffer, clientfd, actArr);
		}
	}

	if (clientfd <= 0)
	{
		perror("error accepting\n");
	}
}

actNode *mmapAccounts()
{
	int fd; 
	if ((fd= open("accounts", O_RDWR, 0644))== -1)
	{
		perror("issue opening accounts!");
		exit(0);
	}
	actNode *actArr = mmap((caddr_t) 0, (size_t) 100*(sizeof(actNode)), PROT_WRITE, MAP_SHARED, fd, 0);
	int count= 0;

	while (count != 20)
	{
		int i= 0;
		while (i!= 100)
		{
			actArr[count].accountName[i]= '\0'; //make everything null instead of just the first character
			i++;
		}
		actArr[count].balance= 0;
		actArr[count].inSession= 0;
		count++;
	}

	close(fd);
	return actArr;
}

int main(int argc, char **argv)
{
	signal(SIGPIPE, SIG_IGN);

	pid_t cpid;
	int sockfd, clientfd;
	struct sockaddr_in server, client;
	int len= sizeof(struct sockaddr_in);

	actNode *actArr= mmapAccounts();

	sem_t *sem;
	sem= sem_open(SEM, O_CREAT, 0644, 4); 

	pthread_t pAccounts;

	if (pthread_create(&pAccounts, NULL, printAccounts, (void *)actArr) < 0)
	{
		perror("could not create thread");
	}

	sockfd = socket(AF_INET , SOCK_STREAM , 0);
	if (sockfd == -1)
		perror("Could not create socket");
	else
		printf("socket has been created!\n");

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(9999);

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

	while ((clientfd= accept(sockfd, (struct sockaddr*)&client, (socklen_t*)&len)))
	{
	  	cpid= fork();
	    if (cpid== 0)
	    {
	    	int newFD= clientfd;
	    	printf("Fork was successful!  Child's PID is: %d\n", (int)getpid());
			readOperations(actArr, newFD);
		}
	}

	if (cpid!= 0) // this is the parent process
	{
		int status;
		waitpid(cpid, &status, 0);
	}

	close(clientfd);
	close(sockfd);
	return 0;
}






