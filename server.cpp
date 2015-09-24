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

#define BUFLEN 256*1024

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

	void run_server()
	{
		char buffer[BUFLEN];
		int n;
		listen(sockfd_s, 5);
		clilen = sizeof(cli_addr);
		while(true)
		{
			newsockfd = accept(sockfd_s, (struct sockaddr *) &cli_addr, &clilen);
			if (newsockfd < 0)
				cerr << ("server: ERROR on accept\n");
			memset(buffer, 0, BUFLEN);
			n = read(newsockfd, buffer, BUFLEN - 1);
			if (n < 0)
				cerr << ("server: ERROR reading from socket\n");
//			cerr<<"Here is the message:"<<buffer<<"\n";
			n = write(newsockfd, "I got your message", 18);
			if (n < 0)
				cerr << ("server: ERROR writing to socket\n");
			close(newsockfd);
		}
	}

	void close_server()
	{
		close(sockfd_s);
	}

	int init_server_side()
	{
		int portno = 50000;
		sockfd_s = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd_s < 0)
		{
			cerr << ("server: ERROR opening socket!\n");
			return -1;
		}
		memset((char *) &serv_addr, 0, sizeof(serv_addr));

		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = INADDR_ANY;
		serv_addr.sin_port = htons(portno);
		if (bind(sockfd_s, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
		{
			cerr << ("server: ERROR on binding! \n");
			return -1;
		}
		server_t = thread([this] {run_server(); });

		return 0;
	}


	void run_client()
	{
		char buffer[BUFLEN];
		int n;
		while(true)
		{
			int portno = 50000;
			sockfd_c = socket(AF_INET, SOCK_STREAM, 0);
			serv_addr.sin_port = htons(portno);
			if (sockfd_c < 0)
			{
				cerr<<("client: ERROR opening socket\n");
				perror(NULL);
				std::this_thread::sleep_for(chrono::seconds(5));
			}
			auto start = chrono::system_clock::now();

			if (connect(sockfd_c, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
				perror("client: ERROR connecting\n");
			auto end = chrono::system_clock::now();
			cout<<"10.0.10.231"<<"\tlatency: "<< (chrono::duration_cast<chrono::microseconds>(end - start).count() / 1000.0)<<"ms\t\t";
			start = end;

			n = write(sockfd_c, buffer, sizeof(buffer));
			if (n < 0)
				cerr<<("client: ERROR writing to socket\n");
			end = chrono::system_clock::now();
			cout<<"bandwidth: "<< BUFLEN*1000/1024.0/chrono::duration_cast<chrono::microseconds>(end - start).count()<<endl;

			close(sockfd_c);
			std::this_thread::sleep_for(chrono::seconds(2));
		}
	}

	int close_client()
	{
		return close(sockfd_c);
	}

	int init_client_side()
	{
		struct sockaddr_in serv_addr;
		struct hostent *server;

		server = gethostbyname("10.0.10.231");
		if (server == NULL)
		{
			cerr << "ERROR, no such host!\n";
			return -1;
		}
		memset((char *) &serv_addr, 0, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		memcpy((char *) server->h_addr, (char *) &serv_addr.sin_addr.s_addr,
				server->h_length);

		client_t = thread([this] {run_client(); });
		return 0;
	}

private:
	int sockfd_s, newsockfd;
	int sockfd_c;
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;

	string uuid;
	set<ComNode> nodes;

public:
	thread client_t;
	thread server_t;
};


int main(int argc, char *argv[])
{
	ComNode cn;
	cn.init_server_side();
	cn.init_client_side();
	cn.server_t.join();
	cn.client_t.join();

	cn.close_server();
	cn.close_client();

	return 0;
}

