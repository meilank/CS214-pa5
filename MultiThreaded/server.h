#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <ctype.h>

#ifndef SERVER_H
#define SERVER_H

typedef int bool;
#define true 1
#define false 0

struct accountNode {
	char accountName[100];
	float balance;
	bool inSession;
}; 
typedef struct accountNode actNode;

static actNode actArr[20];
static pthread_mutex_t mutex_open_1;
static pthread_mutex_t mutex_credit_2;
static pthread_mutex_t mutex_debit_3;
static pthread_mutex_t mutex_balance_4;

void readLine(char *input, int clientfd);
void *hasStarted(void *input);
void *createThread(void* sockfd);
void *printAccounts(void *stoof);
void *acceptConnections(void *stuff);

void *printAccounts(void *);
void *createThread(void *);
void *acceptConnections(void *);

#endif