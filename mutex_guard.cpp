#include "mutex_guard.hpp"
#include "mutex.hpp"
MutexGuard::MutexGuard(Mutex & m) {
    this->m = &m;
    m.Lock();
}
MutexGuard::~MutexGuard() {
    m->Unlock();
}
