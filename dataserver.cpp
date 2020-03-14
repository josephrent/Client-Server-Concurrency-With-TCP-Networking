/* 
    File: dataserver.cpp

    Author: R. Bettati
            Department of Computer Science
            Texas A&M University
    Date  : 2016/07/14

    Dataserver main program for MP2 in CSCE 313
*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include <cassert>
#include <cstring>
#include <sstream>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>

#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include "netreqchannel.hpp"

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
/* VARIABLES */
/*--------------------------------------------------------------------------*/

static int nthreads = 0;

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

void* handle_process_loop(void* args);

/*--------------------------------------------------------------------------*/
/* LOCAL FUNCTIONS -- SUPPORT FUNCTIONS */
/*--------------------------------------------------------------------------*/

std::string int2string(int number) {
  std::stringstream ss;//create a stringstream
   ss << number;//add number to the stream
   return ss.str();//return a string with the contents of the stream
}
string serverRead(int * fd) {
  char buf[255];
  
  if (read(*fd, buf, 255) < 0) {
    cout << "Error reading";
    exit(-1);
  }
  
  string s = buf;
  return s;
}

//SERVER WRITE
int serverWrite(int * fd, string info){
  if (info.length() >= 255) {
    cout << "Message too long";
    exit(-1);
  }
  
  if (write(*fd, info.c_str(), info.length()+1) < 0) {
    cout << "Error writing.";
    exit(-1);
  }
}
/*--------------------------------------------------------------------------*/
/* LOCAL FUNCTIONS -- GENERATE THE DATA */
/*--------------------------------------------------------------------------*/

std::string generate_data() {
  // Generate the data to be returned to the client.
  return int2string(rand() % 100);
}


/*--------------------------------------------------------------------------*/
/* LOCAL FUNCTIONS -- INDIVIDUAL REQUESTS */
/*--------------------------------------------------------------------------*/

void process_hello(int* fd, const std::string & _request) {
  serverWrite(fd,"hello to you too");
}

void process_data(int* fd, const std::string &  _request) {
  string data = generate_data();
  serverWrite(fd,data);
}


/*--------------------------------------------------------------------------*/
/* LOCAL FUNCTIONS -- THE PROCESS REQUEST LOOP */
/*--------------------------------------------------------------------------*/

void process_request(int* fd, const std::string & _request) {

  if (_request.compare(0, 5, "hello") == 0) {
    process_hello(fd, _request);
  }
  else if (_request.compare(0, 4, "data") == 0) {
    process_data(fd, _request);
  }
  else {
    serverWrite(fd,"unknown request");
  }

}
/*--------------------------------------------------------------------------*/
/* LOCAL FUNCTIONS -- THREAD FUNCTIONS */
/*--------------------------------------------------------------------------*/
void* handle_process_loop(int* arg) {
  int * fd = (int*)arg;
  if (fd == NULL)
    cout << "No file descriptor!" << endl;
  for(;;)  {    
    string request = serverRead(fd);
    cout << "New request is " << request << endl;

    if (request.compare("quit") == 0) {
      serverWrite(fd,"bye");
      usleep(100000);
      break;
    }

    process_request(fd, request);
  }
  
  close(*fd);
  
}

/*--------------------------------------------------------------------------*/
/* MAIN FUNCTION */
/*--------------------------------------------------------------------------*/

int main(int argc, char * argv[]) {

  //  std::cout << "Establishing control channel... " << std::flush;
  int PORT = 8080;
  const unsigned short port = 8080;
  int c; 
  unsigned short _port = 0;
  int backlog = 50;
  
  while ((c = getopt(argc, argv, "p:b:")) != -1) {
    switch(c){
      case 'p':
        PORT = atoi(optarg);
        break;
      case 'b':
        backlog = atoi(optarg);
        break;
    }
  }
  NetworkRequestChannel server(port,handle_process_loop,backlog);
  //  std::cout << "done.\n" << std::endl;

}

