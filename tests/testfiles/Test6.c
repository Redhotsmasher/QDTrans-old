#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct{
    pthread_mutex_t lock;
    int value;
}SharedInt;

sem_t sem;
SharedInt* sip;
_Thread_local int foo;

void *functionWithCriticalSection(int* v2) {
    foo = 7;
    // Do some work
    pthread_mutex_lock(&(sip->lock));
    sip->value = sip->value + foo;
    sip->value = sip->value + *v2;
    pthread_mutex_unlock(&(sip->lock));
    // Do some more work
    sem_post(&sem);
}

void *otherFunctionWithCriticalSection(int* v2) {
    foo = 3;
    // Do some work
    pthread_mutex_lock(&(sip->lock));
    sip->value = sip->value + foo;
    sip->value = sip->value + *v2;
    pthread_mutex_unlock(&(sip->lock));
    // Do some more work
    sem_post(&sem);
}

int main() {
    sem_init(&sem, 0, 0);
    SharedInt si;
    sip = &si;
    sip->value = 0;
    int v2 = 1;
    pthread_mutex_init(&(sip->lock), NULL);
    pthread_t thread1;
    pthread_t thread2;
    pthread_create (&thread1,NULL,functionWithCriticalSection,&v2);
    pthread_create (&thread2,NULL,otherFunctionWithCriticalSection,&v2);
    sem_wait(&sem);
    sem_wait(&sem);
    pthread_mutex_destroy(&(sip->lock));
    sem_destroy(&sem);
    printf("%d\n", sip->value); // Should print "12".
    return sip->value-12;
}