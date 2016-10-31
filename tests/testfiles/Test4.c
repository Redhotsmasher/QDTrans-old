#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

pthread_mutex_t lock;
pthread_mutex_t lock2;
pthread_mutex_t lock3;
int value;
int value2;
int value3;

sem_t sem;

void *functionWithCriticalSection(int* v2) {
    // Do some work
    pthread_mutex_lock(&(lock));
    value = value + *v2;
    pthread_mutex_unlock(&(lock));
    // Do some more work
    sem_post(&sem);
}

void *functionWithOtherCriticalSection(int* v2) {
    // Do some work
    pthread_mutex_lock(&(lock2));
    value2 = value2 + *v2;
    pthread_mutex_unlock(&(lock2));
    // Do some more work
    sem_post(&sem);
}

void *functionWithAnotherCriticalSection(int* v2) {
    // Do some work
    pthread_mutex_lock(&(lock3));
    value3 = value3 + *v2;
    pthread_mutex_unlock(&(lock3));
    // Do some more work
    sem_post(&sem);
}

void *functionWithMultilockCriticalSection(int* v2) {
    // Do some work
    pthread_mutex_lock(&(lock));
    pthread_mutex_lock(&(lock2));
    pthread_mutex_lock(&(lock3));
    value = value2 + *v2;
    pthread_mutex_unlock(&(lock));
    pthread_mutex_unlock(&(lock2));
    pthread_mutex_unlock(&(lock3));
    // Do some more work
    sem_post(&sem);
}

void *functionWithReadingCriticalSection(int* v2) {
    // Do some work
    pthread_mutex_lock(&(lock));
    printf("%d\n", value);
    pthread_mutex_unlock(&(lock));
    // Do some more work
    sem_post(&sem);
}

void *functionWithOtherReadingCriticalSection(int* v2) {
    // Do some work
    pthread_mutex_lock(&(lock3));
    printf("%d\n", value3);
    pthread_mutex_unlock(&(lock3));
    // Do some more work
    sem_post(&sem);
}

int main() {
    sem_init(&sem, 0, 0);
    value = 0;
    value2 = 0;
    int v2 = 1;
    pthread_mutex_init((&(lock)), NULL);
    pthread_mutex_init((&(lock2)), NULL);
    pthread_t thread1;
    pthread_t thread2;
    pthread_t thread3;
    pthread_create (&thread1,NULL,functionWithCriticalSection,&v2);
    pthread_create (&thread2,NULL,functionWithOtherCriticalSection,&v2);
    pthread_create (&thread2,NULL,functionWithAnotherCriticalSection,&v2);
    sem_wait(&sem);
    sem_wait(&sem);
    sem_wait(&sem);
    pthread_create (&thread1,NULL,functionWithMultilockCriticalSection,&v2);
    pthread_create (&thread2,NULL,functionWithReadingCriticalSection,&v2);
    pthread_create (&thread3,NULL,functionWithOtherReadingCriticalSection,&v2);
    sem_wait(&sem);
    sem_wait(&sem);
    sem_wait(&sem);
    pthread_mutex_destroy(&(lock));
    pthread_mutex_destroy(&(lock2));
    pthread_mutex_destroy(&(lock3));
    sem_destroy(&sem);
    printf("%d\n", value); // Should print "2".
    printf("%d\n", value2); // Should print "1".
    if(value == 2 && value2 == 1) {
        return 0;
    } else {
        return 1;
    }
}
