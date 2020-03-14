#ifndef _PCBuff_H_
#define _PCBuff_H_
#include <queue>
#include <string>
#include "semaphore.hpp"
#include "mutex.hpp"
using namespace std;
class PCBuff {
private: 

    Semaphore full;
    Semaphore empty;
    Mutex m;
public:
    queue <string> buffer;
    PCBuff(int _size);
    void Produce(string data);
    string Consume();
};
#endif