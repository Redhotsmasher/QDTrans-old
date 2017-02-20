#include <pthread.h>

int main() {
    pthread_mutex_t lock;
    int num = 0;
    pthread_mutex_init(&(lock), NULL);
    // Do some work
    pthread_mutex_lock(&(lock));
    num++;
    pthread_mutex_unlock(&(lock));
    // Do some more work
    pthread_mutex_destroy(&(lock));
    if(num == 1) {
        return 0;
    } else {
        return 1;
    }
}
