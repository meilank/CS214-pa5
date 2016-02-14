#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <ctype.h>

#ifndef CLIENT_H
#define CLIENT_H

typedef int bool;
#define true 1
#define false 0

void *sendM(void *sockfd);
void *recT(void *sockfd);
void intHandle(int sig);

void intHandle(int);
void *sendM(void *);
void *recT(void *);

#endif