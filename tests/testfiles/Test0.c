#include <pthread.h>

int main() {
    pthread_mutex_t lock;
    int num = 0;
    // Do some work
    pthread_mutex_lock(&(lock));
    num++;
    pthread_mutex_unlock(&(lock));
    // Do some more work
    return 0;
}
