/* 
    File: simpleclient.cpp

    Author: R. Bettati
            Department of Computer Science
            Texas A&M University
    Date  : 2019/09/23

    Simple client main program for MP2 in CSCE 313
*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include <cassert>
#include <string>
#include <iostream>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include "reqchannel.hpp"
using namespace std;
/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */ 
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* LOCAL FUNCTIONS -- SUPPORT FUNCTIONS */
/*--------------------------------------------------------------------------*/

std::string int2string(int number) {
  std::stringstream ss;//create a stringstream
   ss << number;//add number to the stream
   return ss.str();//return a string with the contents of the stream
}
std::string generate_data() {
  // Generate the data to be returned to the client.
  return int2string(rand() % 100);
}
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

/*--------------------------------------------------------------------------*/
/* MAIN FUNCTION */
/*--------------------------------------------------------------------------*/

int main(int argc, char * argv[]) {
    
    pid_t pid = fork();
    struct timeval start,end;
    if(pid == 0) {
        execvp("./dataserver",NULL);
    }
    else {
        std::cout << "CLIENT STARTED:" << std::endl;

        std::cout << "Establishing control channel... " << std::flush;
        RequestChannel chan("control", RequestChannel::CLIENT_SIDE);
        std::cout << "done." << std::endl;


        /* -- Start sending a sequence of requests */

        std::string reply1 = chan.send_request("hello");
        std::cout << "Reply to request 'hello' is '" << reply1 << "'" << std::endl;

        std::string reply2 = chan.send_request("data Joe Smith");
        std::cout << "Reply to request 'data Joe Smith' is '" << reply2 << "'" << std::endl;

        std::string reply3 = chan.send_request("data Jane Smith");
        std::cout << "Reply to request 'data Jane Smith' is '" << reply3 << "'" << std::endl;
        gettimeofday(&start,NULL);//starts timing for sending request
        for(int i = 0; i < 10000; i++) {

            std::string request_string("data TestPerson" + int2string(i));
            std::string reply_string = chan.send_request(request_string);
            std::cout << "reply to request " << i << ":" << reply_string << std::endl;;
        }
        gettimeofday(&end,NULL);//ends timing for send request
        print_time_diff("Time taken for sending requests over dataserver: ", start, end);

        gettimeofday(&start,NULL);//starts timing for generate_data locally
        for(int i = 0; i < 10000; i++) {
            std::string reply_string = generate_data();
        }
        gettimeofday(&end,NULL);//ends timing
        print_time_diff("Time taken for generating data locally: ", start, end);

        std::string reply4 = chan.send_request("quit");
        std::cout << "Reply to request 'quit' is '" << reply4 << std::endl;

        usleep(1000000);
    }
  

}
