
# uncomment the version of the compiler that you are using
#clang++ is for Mac OS 
#C++ = clang++ -std=c++11
# g++ is for most Linux
C++ = g++ -std=c++11

.DEFAULT_GOAL := all

clean: 
	rm -f *.o dataserver client

all: dataserver  client

netreqchannel.o: netreqchannel.hpp netreqchannel.cpp
	$(C++) -c -g netreqchannel.cpp

mutex.o: mutex.hpp mutex.cpp
	$(C++) -c -g mutex.cpp

mutex_guard.o: mutex_guard.hpp mutex_guard.cpp
	$(C++) -c -g mutex_guard.cpp

semaphore.o: semaphore.hpp semaphore.cpp
	$(C++) -c -g semaphore.cpp

PCBuff.o: PCBuff.hpp PCBuff.cpp # mutex.o semaphore.o 
	$(C++) -c -g PCBuff.cpp

dataserver: dataserver.cpp netreqchannel.o 
	$(C++) -o dataserver dataserver.cpp netreqchannel.o -lpthread

client: client.cpp  mutex.o semaphore.o netreqchannel.o PCBuff.o
	$(C++) -o client client.cpp mutex.o semaphore.o netreqchannel.o PCBuff.o -lpthread