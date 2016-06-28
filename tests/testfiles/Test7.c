#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

pthread_mutex_t lock;
pthread_mutex_t lock2;
int value;
int value2;

sem_t sem;

void *functionWithCriticalSection(int* v2) {
    // Do some work
    pthread_mutex_lock(&(lock));
    pthread_mutex_lock(&(lock2));
    value = value2 + *v2;
    value2 = value;
    pthread_mutex_unlock(&(lock2));
    pthread_mutex_unlock(&(lock));
    // Do some more work
    sem_post(&sem);
}

void *otherFunctionWithCriticalSection(int* v2) {
    // Do some work
    if (v2 != 0) {
        pthread_mutex_lock(&(lock));
        pthread_mutex_lock(&(lock2));
        value = value2 + 1;
	value2 = value;
        pthread_mutex_unlock(&(lock2));
        pthread_mutex_unlock(&(lock));
    }
    // Do some more work
    sem_post(&sem);
}

int main() {
    sem_init(&sem, 0, 0);
    value = 0;
    value2 = 0;
    int v2 = 2;
    pthread_mutex_init(&(lock), NULL);
    pthread_mutex_init(&(lock2), NULL);
    pthread_t thread1;
    pthread_t thread2;
    pthread_create (&thread1,NULL,functionWithCriticalSection,&v2);
    pthread_create (&thread2,NULL,otherFunctionWithCriticalSection,&v2);
    sem_wait(&sem);
    sem_wait(&sem);
    pthread_mutex_destroy(&(lock));
    pthread_mutex_destroy(&(lock2));
    sem_destroy(&sem);
    printf("%d\n", value); // Should print "3".
    return 0;
}
