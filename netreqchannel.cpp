/*
    File: netreqchannel.hpp

    Author: R. Bettati
            Department of Computer Science
            Texas A&M University
    Date  : 2019/11/19

*/


/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

//#include <iostream>
//#include <fstream>

#include <string>
#include "netreqchannel.hpp"
#include <netinet/in.h> 
#include <sys/socket.h>
#include <arpa/inet.h> 

#include <unistd.h> 
#include <stdio.h> 
#include <stdlib.h> 

#include <signal.h>
#include <pthread.h>
//#include "client.cpp"
#include "dataserver.cpp"


//#define PORT 8080 

using namespace std;

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */ 
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */ 
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CLASS  N e t w o r k R e q u e s t C h a n n e l */
/*--------------------------------------------------------------------------*/


NetworkRequestChannel::NetworkRequestChannel(const string _server_host_name, const unsigned short _port_no) {
    my_side = Side::CLIENT;
    struct sockaddr_in addr; 
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET; 
    //addr.sin_port = htons(PORT); 
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) { 
        printf("\n Socket creation error \n"); 
        return; 
    }
    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) { 
        printf("\nConnection Failed \n"); 
        return; 
    } 

}
  /* Creates a CLIENT-SIDE local copy of the channel. The channel is connected
     to the given port number at the given server host. 
     THIS CONSTRUCTOR IS CALLED BY THE CLIENT. */

NetworkRequestChannel::NetworkRequestChannel(const unsigned short _port_no, void * (*connection_handler) (int *), int backlog) {
    my_side = Side::SERVER;
    struct sockaddr_in address; 
    int addrlen = sizeof(address); 
    struct sockaddr_storage their_addr;
    char buffer[1024] = {0}; 
    memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) { 
        perror("socket failed"); 
        exit(EXIT_FAILURE); 
    } 
    //address.sin_port = htons( PORT ); 
    //bind(fd, (struct sockaddr *)&address, sizeof(address));
    bind(fd, (struct sockaddr *)&address, sizeof(address));

    if (listen(fd, backlog) < 0) { 
        perror("listen"); 
        exit(EXIT_FAILURE); 
    } 
    while(1) {  // main accept() loop
        pthread_t thread;
        pthread_attr_t attr; 
        pthread_attr_init(&attr);
        int slave_socket = accept(fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (slave_socket == -1) {
            perror("accept");
            continue;
        }
        pthread_create(&thread, &attr,handle_process_loop, (void*)slave_socket);
    }
    cout << "Connection Established" << endl;
}

  /* Creates a SERVER-SIDE local copy of the channel that is accepting connections
     at the given port number.
     NOTE that multiple clients can be connected to the same server-side end of the
     request channel. Whenever a new connection comes in, it is accepted by the 
     the server, and the given connection handler is invoked. The parameter to 
     the connection handler is the file descriptor of the slave socket returned
     by the accept call.
     NOTE that it is up to the connection handler to close the socket when the 
     client is done. */ 

NetworkRequestChannel::~NetworkRequestChannel() {
    close(fd);
}
int NetworkRequestChannel::get_fd() {
    return fd;
}
  /* Destructor of the local copy of the bus. By default, the Server Side deletes 
     any IPC mechanisms associated with the channel. */

/*string NetworkRequestChannel::send_request(string _request) {
    cwrite(_request);
    string s = cread();
    return s;
}*/
    /* Send a string over the channel and wait for a reply. */
string NetworkRequestChannel::cread() {
    char buf[255];
    
    if (read(fd, buf, 255) < 0) {
        perror("Can't read");
        exit(-1);
    }
    
    string s = buf;
    return s;
}
  /* Blocking read of data from the channel. Returns a string of characters
     read from the channel. Returns NULL if read failed. */

int NetworkRequestChannel::cwrite(string _msg) {
	if (_msg.length() >= 255) {
		cout << "Message exceeded 255";
		return -1;
    }
	const char * msgStr = _msg.c_str();

	if (write(fd, msgStr, strlen(msgStr)+1) < 0) {
		perror("Can't write.");
		exit(-1);
	}
  /* Write the data to the channel. The function returns the number of characters written
     to the channel. */

}




