#include "mutex.hpp"
#include <pthread.h>

Mutex::Mutex() {
    m = PTHREAD_MUTEX_INITIALIZER;
}

Mutex::~Mutex() {
    pthread_mutex_destroy(&m);
}

/* -- MUTEX OPERATIONS */

void Mutex::Lock() {
    pthread_mutex_lock(&m);
}
void Mutex::Unlock() {
    pthread_mutex_unlock(&m);
}