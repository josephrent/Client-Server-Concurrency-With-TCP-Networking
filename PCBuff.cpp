
#include "PCBuff.hpp"
PCBuff::PCBuff(int _size) : full(0), empty(_size) {
    //m = Mutex();
}
string PCBuff::Consume() {
    full.P();
    m.Lock();
    string str = buffer.front();
    buffer.pop();
    m.Unlock();
    empty.V();
    return str;
}
void PCBuff::Produce(string data) {
    empty.P();
    m.Lock();
    buffer.push(data);
    m.Unlock();
    full.V();
}