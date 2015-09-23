//============================================================================
// Name        : testcpp.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C, Ansi-style
//============================================================================

#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <string>
#include <chrono>
#include <vector>
#include <set>
#include <string.h>
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

using namespace std;


/*
a simple base "Communications Node" (CN) for a distributed communications system
running across multiple machines on a mixed (fast, slow, wired, WiFi, etc...) "local" network. In
its most basic form, each CN should have a unique ID and should come up and continuously
enumerate all the other nodes it can see on its network:
 Update every 10-20 seconds
 List all other CNs on the reachable LAN (don’t worry about firewalls, NAT, etc…)
 Show round trip comms latency to every other node
 Show an estimate of "to" and "from" bandwidth with each other CN
 Implementation should allow an arbitrary number of CNs
 Implementation should allow multiple CNs to smoothly coexist on one machine
C or C++ with standard TCP and UDP socket calls is probably going to be the fastest route to
success on this task but we will accept alternative approaches.
*/

class ComNode{
public:
	void list()
	{

	}

	friend ostream& operator<<(ostream &os, ComNode& node)
	{
		os<<node.uuid;
		return os;
	}

	void init_server_side()
	{
//		struct sockaddr_in serv_addr, cli_addr;
//		int n;
//		sockfd = socket(AF_INET, SOCK_STREAM, 0);
//		if (sockfd < 0)
//			cerr << ("ERROR opening socket");
//		memset((char *) &serv_addr, 0, sizeof(serv_addr));
//		portno = 50000;//atoi(argv[1]);
//		serv_addr.sin_family = AF_INET;
//		serv_addr.sin_addr.s_addr = INADDR_ANY;
//		serv_addr.sin_port = htons(portno);
	}
private:
	string uuid;
	set<ComNode> nodes;
};


struct Node
{
	Node():sockfd(socket(AF_INET, SOCK_STREAM, 0)),portno(5000)
	{
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = INADDR_ANY;
		serv_addr.sin_port = htons(portno);
	}
	int sockfd, newsockfd, portno;
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;
};

#define BUFLEN 256*1024


int main(int argc, char *argv[])
{
	int sockfd, newsockfd, portno;
	socklen_t clilen;
	char buffer[BUFLEN];
	struct sockaddr_in serv_addr, cli_addr;
	int n;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		cerr << ("ERROR opening socket");
	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	portno = 50000;//atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
		cerr << ("ERROR on binding");
	while(true)
	{
		listen(sockfd, 5);
		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if (newsockfd < 0)
			cerr << ("ERROR on accept");
		memset(buffer, 0, BUFLEN);
		n = read(newsockfd, buffer, BUFLEN - 1);
		if (n < 0)
			cerr << ("ERROR reading from socket");
		printf("Here is the message: %s\n", buffer);
		n = write(newsockfd, "I got your message", 18);
		if (n < 0)
			cerr << ("ERROR writing to socket");
		close(newsockfd);
	}
	close(sockfd);
	return 0;
}




/*
#define BLKSIZE 128*1024

using namespace std;

thread t;
mutex wmt;
mutex rmt;
condition_variable cv;

int gp;
int gp_hd;
//bool cv_f = false;

template<typename T=size_t>
struct Arg
{
	Arg():exit(false), arg(0)
	{}

	Arg(T t, bool b=false):exit(b), arg(t)
	{}

	friend ostream& operator<<(ostream&os, Arg& arg)
	{
		os <<"Arg: " << arg.arg << " / " << arg.exit<<endl;
		return os;
	}

	bool exit;
	T arg;
};

vector<Arg<>> g_v;
bool exit_f = false;
const char* fn = "/media/SamsungSSD/test";
const char* fn1 = "/px/mfs/10.0.10.150/media/gai_nfs_1/test";

void tfun(vector<Arg<>> &arg)
{
	while(true)
	{
		unique_lock<mutex> lock(wmt);
		while(g_v.size() < 1)
		{
			if(exit_f && g_v.size() == 0)
			{
				break;
			}
			cv.wait(lock);
		}

		if(exit_f && g_v.size() == 0)
		{
			cout<<"\nexiting ...\n";
			lock.unlock();
			break;
		}
		Arg<> arg = g_v[0];
		g_v.erase(g_v.begin());
		lock.unlock();

		char str[BLKSIZE];
		ssize_t re = pread(gp, str, BLKSIZE, BLKSIZE*(arg.arg));
		assert(re == BLKSIZE);
		re = pwrite(gp_hd, str, BLKSIZE, BLKSIZE*(arg.arg));
		cout<<"thread "<<arg;
		cout.flush();
		assert(re == BLKSIZE);
	}
}

#include <unistd.h>
#include <fcntl.h>
int main(void)
{
	gp = open(fn, O_CREAT | O_RDWR, S_IRUSR|S_IWUSR);
	gp_hd = open(fn1, O_CREAT | O_WRONLY, S_IRUSR|S_IWUSR);
	char str[BLKSIZE]="sdfasd";

//	Arg arg;
	t = thread(tfun, ref(g_v));

	for(size_t i=0; i<111150; i++)
	{
		Arg<> arg(i);
		cout<<"main "<<arg;
		cout.flush();
		ssize_t re = pwrite(gp, str, BLKSIZE, BLKSIZE*(i));
		assert(re == BLKSIZE);
		unique_lock<mutex> lock(wmt);
		g_v.push_back(arg);
		cv.notify_one();
		lock.unlock();
	}
//	sleep(1);

	{
		unique_lock<mutex> lock(wmt);
		Arg<> arg(1, true);
		exit_f = true;
		cv.notify_one();
		lock.unlock();
	}

	t.join();

	return 0;
}
*/
