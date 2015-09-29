#ifndef _BASE_NODE_H_
#define _BASE_NODE_H_

#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/utsname.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
using namespace std;


#define BUFLEN 256*1024
#define DEFAULTPORT 50000
// newer com node stops if avaiable port number is greater MAXPORT
#define MAXPORT 60000
//#define LOCALHOST "127.0.0.1"
#define SLEEPDURATION 20

class Base_node{
public:
	virtual ~Base_node()
	{
		join();
	}
	virtual void run()=0;
	virtual int init()=0;
	virtual int close_node()
	{
		return close(sockfd);
	}
	virtual void join()
	{
		thd.join();
	}

protected:
	int sockfd, portno;
	struct sockaddr_in addr;
	thread thd;
};


#endif
