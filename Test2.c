#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct{
    pthread_mutex_t lock;
    int value;
}SharedInt;

struct SharedInt2{
    pthread_mutex_t lock;
    int value;
};

typedef struct SharedInt2* SharedInt3;

_Thread_local int poop = 0;

sem_t sem;
SharedInt* sip;

struct SharedInt2* sip2;

struct SharedInt2* nob() {
    //nob
}

void *functionWithCriticalSection(int* v2) {
    // Do some work
    1+1;
    pthread_mutex_lock(&(sip->lock));
    sip->value = sip->value + *v2;
    poop = 1;
    nob();
    sip2->value = sip2->value + 1;
    pthread_mutex_unlock(&(sip->lock));
    // Do some more work
    sem_post(&sem);
    //return poop + sip->value;
    return poop + 7;
}

int main() {
    poop = 0;
    sem_init(&sem, 0, 0);
    SharedInt si;
    sip = &si;
    printf("ASDFASDFASDF\n");
    sip->value = 0;
    sip2->value = 0;
    int v2 = 1;
    pthread_mutex_init(&(sip->lock), NULL);
    pthread_t thread1;
    pthread_t thread2;
    pthread_create (&thread1,NULL,functionWithCriticalSection,&v2);
    pthread_create (&thread2,NULL,functionWithCriticalSection,&v2);
    sem_wait(&sem);
    sem_wait(&sem);
    pthread_mutex_destroy(&(sip->lock));
    sem_destroy(&sem);
    printf("%d\n", sip->value); // Should print "2".
    return 0;
}
