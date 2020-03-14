#include <cassert>
#include <string>
#include <iostream>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <vector>
#include "PCBuff.hpp"
#include "netreqchannel.hpp"
#include <map>

using namespace std;

void print_time_diff(const string _label, 
                     const struct timeval & _tp1, 
                     const struct timeval & _tp2) {
  /* Prints to stdout the difference, in seconds and museconds, between
     two timevals. 
  */

  cout << _label;
  long sec = _tp2.tv_sec - _tp1.tv_sec;
  long musec = _tp2.tv_usec - _tp1.tv_usec;
  if (musec < 0) {
    musec += 1000000;
    sec--;
  }
  cout << " [sec = " << sec << ", musec = " << musec << "]" << endl;

}
struct RTArgs {
    PCBuff* WPC;
    string patientName;
    int nReq;
};
void* ReqThreadFunction(void * _address) {
    RTArgs* args = (RTArgs*)_address;
    for(int i=0; i < args->nReq; ++i) {
        args->WPC->Produce("data " + args->patientName);
    }
    return nullptr;
}

struct WTArgs {
    PCBuff* WPC;
    vector<NetworkRequestChannel*> channels;
    map<string,PCBuff*> pcMap;
    int numRC;
    Mutex* print;
};
void* EventHandler(void* addr) {
    WTArgs* args = (WTArgs*)addr;
    vector<string> requests(args->numRC);
    int doneCount = 0;
    for(int i = 0; i < args->numRC; i++) {
        string req = args->WPC->Consume();
        requests[i] = req;
        if(requests[i].compare("done")==0) {
            args->WPC->Produce("done");
            doneCount++;
        }
        else {
            args->channels[i]->cwrite(req);
        }
    }
    cout << "Done creating all chans\n";
    while(doneCount<args->channels.size()) {
        fd_set read;
        FD_ZERO(&read);
        int m = 0;
        for(int i = 0; i < args->numRC; ++i) {
            FD_SET(args->channels[i]->get_fd(),&read);
            if(args->channels[i]->get_fd() > m) {//max fd
                m = args->channels[i]->get_fd();
            }
        }
        int n = select(/*args->numRC*/m+1,&read,NULL,NULL,NULL); // max fd + 1
        for(int i = 0; i < args->numRC; ++i) {
            if(FD_ISSET(args->channels[i]->get_fd(),&read)) {
                if(requests[i].compare("done")!=0) {
                    string resp = args->channels[i]->cread();
                    string name = requests[i].substr(5, requests[i].length()-5);
                    args->pcMap[name]->Produce(resp);
                    requests[i] = args->WPC->Consume();
                    if(requests[i].compare("done")==0) {
                        args->WPC->Produce("done");
                        doneCount ++;
                    }
                    else {
                        cout << "I should not come here\n";
                        args->channels[i]->cwrite(requests[i]);
                    }
                }
            }
        }
    }

    for (int i = 0; i < args->numRC; ++i) {
        //args->channels[i]->send_request("quit");
        args->channels[i]->cwrite("quit");
        delete args->channels[i];
    }
    return nullptr;


}

struct STArgs {
    PCBuff* PCB;
    string name;
    int numReq;
    Mutex* printing;
};
void* STFunc(void* addr) {
    //create histogram variable
    STArgs* args = (STArgs*)addr;
    vector<int> hist(10);
    for(int i = 0; i < args->numReq; ++i) {//meant to count 
        string rString = args->PCB->Consume();
        int res = stoi(rString);
        hist[res/10]++;
    }
    args->printing->Lock();
    for(int i = 0; i < 10 ; ++i) {
        cout << hist[i] << " ";
    }
    cout << endl;
    args->printing->Unlock();
    return nullptr;
    //cout << "wtf" << endl;
}
int main(int argc, char * argv[]) {
    //command line args, 
    int buffSize = 50; //b
    int numRC = 110;// w
    int n = 100; // n num of Requests
    struct timeval start,end;
    int c=0;
    int PORT = 8080;
    string HOST = "localhost";
    
   	while((c = getopt(argc, argv, "n:b:w:h:p")) != -1) {
    	switch(c) {
			case 'n':
				n = atoi(optarg);
                cout << "n is " << n << endl;
				break;
			case 'b':
				buffSize = atoi(optarg);
                cout << "buffSize is " << buffSize << endl;
				break;
			case 'w':
				numRC = atoi(optarg);
                cout << "numRC is " << numRC << endl;
				break;
			case 'h':
				HOST = atoi(optarg);
                cout << "host is " << HOST << endl;
				break;
			case 'p':
				PORT = atoi(optarg);
                cout << "port is " << PORT << endl;
				break;
		}
    }

    pid_t pid = fork();
    if(pid == 0) {
        execvp("./dataserver",NULL);
    }
    else {

        gettimeofday(&start,NULL);//starts timing for sending request

        PCBuff WPC = PCBuff(buffSize); //make our WPC buffer

        RTArgs P1RTArgs;
        P1RTArgs.WPC = &WPC;
        P1RTArgs.patientName = "Joe Smith";
        P1RTArgs.nReq = n;
        pthread_t P1RT;
        pthread_create(&P1RT, NULL, ReqThreadFunction, (void*)&P1RTArgs);
        RTArgs P2RTArgs;
        P2RTArgs.WPC = &WPC;
        P2RTArgs.patientName = "Jane Smith";
        P2RTArgs.nReq = n;
        pthread_t P2RT;
        pthread_create(&P2RT, NULL, ReqThreadFunction, (void*)&P2RTArgs);

        RTArgs P3RTArgs;
        P3RTArgs.WPC = &WPC;
        P3RTArgs.patientName = "John Doe";
        P3RTArgs.nReq = n;
        pthread_t P3RT;
        pthread_create(&P3RT, NULL, ReqThreadFunction, (void*)&P3RTArgs);

        PCBuff stbuff1 = PCBuff(buffSize);
        PCBuff stbuff2 = PCBuff(buffSize);
        PCBuff stbuff3 = PCBuff(buffSize);

        pthread_t wThread;
        map<string,PCBuff*> nMap;
        nMap[P1RTArgs.patientName] = &stbuff1;
        nMap[P2RTArgs.patientName] = &stbuff2;
        nMap[P3RTArgs.patientName] = &stbuff3;

        vector<NetworkRequestChannel*> channels;
        for (int i = 0; i < numRC; i++) {
			NetworkRequestChannel * channel = new NetworkRequestChannel(HOST,PORT);
			channels.push_back(channel);
		}
        //RequestChannel controlChannel("control",RequestChannel::CLIENT_SIDE);// DONT NEED CONTROL CHANNEL

        Mutex print;
        WTArgs workerArgs;
        workerArgs.numRC = numRC;
        workerArgs.channels = channels;//puts all networkrequestchannels into struct to be used in the event handler
        workerArgs.pcMap = nMap;
        workerArgs.print = &print;
        workerArgs.WPC = &WPC; //////////
        pthread_create(&wThread, NULL, EventHandler, (void*)&workerArgs);

        STArgs P1STArgs;
        P1STArgs.PCB = &stbuff1;
        P1STArgs.name = "Joe Smith";
        P1STArgs.numReq = n;
        P1STArgs.printing = &print;
        pthread_t P1ST;
        pthread_create(&P1ST, NULL, STFunc, (void*)&P1STArgs);
        pthread_t P2ST;
        STArgs P2STArgs;
        P2STArgs.PCB = &stbuff2;
        P2STArgs.name = "Jane Smith";
        P2STArgs.numReq = n;
        P2STArgs.printing = &print;
        pthread_create(&P2ST,NULL,STFunc,(void*)&P2STArgs);
        pthread_t P3ST;
        STArgs P3STArgs;
        P3STArgs.PCB = &stbuff3;
        P3STArgs.name = "John Doe";
        P3STArgs.numReq = n;
        P3STArgs.printing = &print;
        pthread_create(&P3ST,NULL,STFunc,(void*)&P3STArgs);

        pthread_join(P1RT,NULL);
        pthread_join(P2RT,NULL);
        pthread_join(P3RT,NULL);
        WPC.Produce("done");

        pthread_join(wThread,NULL);
        pthread_join(P1ST,NULL);
        pthread_join(P2ST,NULL);
        pthread_join(P3ST,NULL);
        print.Lock();
        //controlChannel.send_request("quit");
        print.Unlock();
        gettimeofday(&end,NULL);//ends timing for send request
        print_time_diff("Time taken for sending requests over dataserver: ", start, end);
    }
    cout << "end" << endl;
    return 0;
    //
}
    