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
#define DEFAULTPORT 50000
// newer com node stops if avaiable port number is greater MAXPORT
#define MAXPORT 60000
#define LOCALHOST "127.0.0.1"
#define SLEEPDURATION 8


#include <sys/utsname.h>
#include <arpa/inet.h>

class Util
{
public:
	static string getLocalIp()
	{
		struct utsname utsname;
		struct hostent *h;
		uname(&utsname);

		if (!(h = gethostbyname(utsname.nodename)))
		{
			return LOCALHOST; // default string
		}

		return string(inet_ntoa(*((struct in_addr *) h->h_addr)));
	}

	static void splitIpPort(string ipandport, vector<string>& v)
	{
		string ip = "";
		string port = "";

		bool flag = false;
		size_t i=0;
		while(ipandport[i] == ' ')
		{
			i ++;
		}
		for(; i<ipandport.size(); i++)
		{
			if (ipandport[i] == ' ')
			{
				flag = true;
				continue;
			}
			if(flag)
			{
				port += ipandport[i];
			}
			else
			{
				ip += ipandport[i];
			}
		}

		v.push_back(ip);
		v.push_back(port);
	}
};

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

class Server_node;
class Client_node:public Base_node{
private:
	void communicate(string serverip, int port)
	{
		struct hostent *server;
		int n;
		char buffer[BUFLEN];
		server = gethostbyname(serverip.c_str());
		string cn_info;
		if (server == NULL)
		{
			cerr << "ERROR, no such host! "<< serverip<<endl;
			return;
		}
		memset((char *) &addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		memcpy((char *) server->h_addr, (char *) &addr.sin_addr.s_addr,
				server->h_length);
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		addr.sin_port = htons(port);
		if (sockfd < 0)
		{
			cerr<<("client: ERROR opening socket\n");
			perror(NULL);
			std::this_thread::sleep_for(chrono::seconds(1));
			return;
		}
		auto start = chrono::system_clock::now();
		cn_info = serverip+" "+to_string(port);
		if (connect(sockfd, (struct sockaddr *) &addr, sizeof(addr)) < 0)
		{
			cerr<<"client: ERROR connecting to host:"<<serverip<<" port:"<<port;
			perror(NULL);
			neighbors.erase(cn_info);
			return;
		}
		auto end = chrono::system_clock::now();
		cout<<"host: "<<serverip<<"\t\tport: " <<port<<"\tlatency: "<< (chrono::duration_cast<chrono::microseconds>(end - start).count() / 1000.0)<<"ms\t";
		start = end;
//		cn_info = sn->getServerID();
		sprintf(buffer, "%s", serverID.c_str());
//		cout<<"          buffer_c:"<<buffer;
		n = write(sockfd, buffer, sizeof(buffer));
		if (n < 0)
		{
			cerr<<("client: ERROR writing to socket\n");
			return;
		}
		end = chrono::system_clock::now();
		cout<<"bandwidth: "<< BUFLEN*1000/1024.0/chrono::duration_cast<chrono::microseconds>(end - start).count()<<endl;

		close(sockfd);
	}

public:
	void run()
	{
//		char buffer[BUFLEN];
		portno = DEFAULTPORT;
//		sprintf(buffer, "%s %d", serverID.c_str(), portno);
//		serverID = Util::getLocalIp();
		insertNode(Util::getLocalIp()+" "+to_string(DEFAULTPORT));
		while(true)
		{
			for( auto i: this->neighbors)
			{
				vector<string> v;
				Util::splitIpPort(i, v);
//				cout<<"ip: "<<v[0] <<" port: "<< atoi(v[1].c_str())<<endl;
				communicate(v[0], atoi(v[1].c_str()));
			}
			std::this_thread::sleep_for(chrono::seconds(SLEEPDURATION));
		}
	}

	int init()
	{
		thd = thread([=] {run(); });
		return 0;
	}

	void insertNode(string neighborid)
	{
		neighbors.insert(neighborid);
	}

	void setLocalServer(string id)
	{
		serverID = id;
	}

private:
	string serverID;
	set<string> neighbors;
//	Server_node* sn;
};

class Server_node:public Base_node
{
public:
	void run()
	{
		char buffer[BUFLEN];
		int n;
		listen(sockfd, 5);
		struct sockaddr_in cli_addr;
		socklen_t clilen = sizeof(cli_addr);
		while(true)
		{
			int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
			if (newsockfd < 0)
				cerr << ("\nserver: ERROR on accept\n");
			memset(buffer, 0, BUFLEN);
			n = read(newsockfd, buffer, BUFLEN - 1);
			if (n < 0)
				cerr << ("server: ERROR reading from socket\n");
//			cout<<"buffer: "<<buffer<<"|"<<endl;
			if(cn)
				cn->insertNode(buffer);

			n = write(newsockfd, "I got your message", BUFLEN - 1);
			if (n < 0)
				cerr << ("server: ERROR writing to socket\n");
			close(newsockfd);
		}
	}

	int init()
	{
		portno = DEFAULTPORT;
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd < 0)
		{
			cerr << ("server: ERROR opening socket!\n");
			return -1;
		}
		memset((char *) &addr, 0, sizeof(addr));

		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = INADDR_ANY;
		addr.sin_port = htons(portno);
//		cout << "server try binding port:"<< portno<<"...\n";
		while (bind(sockfd, (struct sockaddr *) &addr, sizeof(addr)) < 0)
		{
			addr.sin_port = htons(++portno);
			cerr <<"server: ERROR on binding! retry with increnment port:"<< portno<<"...\n";
			if (portno > MAXPORT)
			{
				cerr << ("server: ERROR on binding! exceed maxport number, aborting...\n");
				exit(1); // newer com node stops if avaiable port number is greater MAXPORT
			}
		}
		UUID = Util::getLocalIp()+" "+to_string(this->portno);
		if(cn)
		{
			cout<<(Util::getLocalIp()+" "+to_string(portno));
			cn->insertNode(UUID);
			cn->setLocalServer(UUID);
		}
		cout << "server successfully binds port:"<< portno<<"...\n";

		thd = thread([=] {run(); });

		return 0;
	}

	void addNeighbor(Client_node* cnp)
	{
		cn = cnp;
		UUID = Util::getLocalIp()+" "+to_string(this->portno);
		cout<<"serverip: "<<UUID<<endl;
		cn->insertNode(UUID);
	}

	string getServerID()
	{
		return UUID;
	}

private:
	Client_node* cn;
	string UUID;
};

class ComNode
{
public:
	ComNode()//:port(DEFAULTPORT)
	{
		port = 0;
		sn = new Server_node();
		cn = new Client_node();
	}

	void start()
	{
		sn->addNeighbor(cn);
		sn->init();
		cn->init();

		sn->join();
		cn->join();
	}
private:
	Server_node* sn;
	Client_node* cn;
	int port;
};


int main(int argc, char *argv[])
{
	vector<string>v;
	ComNode cn;
	cn.start();

	return 0;
}

