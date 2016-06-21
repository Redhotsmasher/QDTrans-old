#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct{
}SharedInt;

_Thread_local int lint;
int* lintp;
int sint;
_Thread_local int * lintp2;
int** lintpp;

struct SharedInt2{
    pthread_mutex_t lock;
    int value;
};

typedef struct SharedInt2* SharedInt3;

_Thread_local int poop = 0;

sem_t sem;
SharedInt* sip;

struct SharedInt2* sip2;

void *functionWithCriticalSection(SharedInt* sip) {
    printf("lint: %i\n", lint);
    printf("sint: %i\n", sint);
    printf("lintp: %i\n", *lintp);
    printf("lintpp: %i\n", **lintpp);
    sem_post(&sem);
}

struct SharedInt2* nob() {
    //nob
}

int main() {
    sip = calloc(1, sizeof(SharedInt));
    lint = 1;
    lintp = &lint;
    sint = 123;
    lintp2 = &lint;
    lintpp = &lintp2;
    //pthread_mutex_init(&(sip->lock), NULL);
    pthread_t thread1;
    pthread_t thread2;
    pthread_create (&thread1,NULL,functionWithCriticalSection,sip);
    pthread_create (&thread2,NULL,functionWithCriticalSection,sip);
    sem_wait(&sem);
    sem_wait(&sem);
    //pthread_mutex_destroy(&(sip->lock));
    sem_destroy(&sem);
    //printf("%d\n", sip->value); // Should print "2".
    return 0;
}
