#include "semaphore.hpp"
#include <pthread.h>
Semaphore::Semaphore(int _val) {
    pthread_cond_init(&c, NULL);
    pthread_mutex_init(&m, NULL);
    value = _val;
}

Semaphore::~Semaphore() {

}

int Semaphore::P() {
    pthread_mutex_lock(&m);
    value -=1;
    if(value < 0) { 
        pthread_cond_wait(&c,&m);
    }
    pthread_mutex_unlock(&m);
    return 1;
}
int Semaphore::V() {
    pthread_mutex_lock(&m);
    value++;
    if(value<=0) {
        pthread_cond_signal(&c);
    }
    pthread_mutex_unlock(&m);
    return 1;

}