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

void *sendM(void *);
void *recT(void *);

#endif