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

bool printing= false;

#define SEM "/sem"

void readLine(char *input, int clientfd, actNode *actArr);
void readOperations(actNode *actArr, int clientfd);
void *printAccounts(void *arr);
actNode * mmapAccounts();
int findAccount(char *name, actNode* shared);

void *printAccounts(void *);

#endif