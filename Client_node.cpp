#include "Client_node.h"
#include "Base_node.h"
#include "Util.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/utsname.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <iostream>
#include <string.h>
#include <sys/select.h>

using namespace std;

void Client_node::communicate(string serverip, int port)
{
	int n;
	char buffer[BUFLEN];
	struct sockaddr_in addrin;
	addrin.sin_family = AF_INET;
	addrin.sin_addr.s_addr = inet_addr(serverip.c_str());
	addrin.sin_port = htons(port);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		std::this_thread::sleep_for(chrono::seconds(1));
		close(sockfd);
		return;
	}
	string cn_info = serverip+" "+to_string(port);
	auto start = chrono::system_clock::now();
	if (connect(sockfd, (struct sockaddr *) &addrin, sizeof(addrin)) < 0)
	{
		cerr<<"client:  REMOVE DEAD NODE, ERROR connecting to host:"<<serverip<<" port:"<<port<<"";
		perror(NULL);
		this->removeNode(cn_info);
		close(sockfd);
		return;
	}
	auto end = chrono::system_clock::now();
	cout<<"host: "<<serverip<<"\t\tport: " <<port<<"\tlatency: "<< (chrono::duration_cast<chrono::microseconds>(end - start).count() / 1000.0)<<"ms\t";
	start = end;
	sprintf(buffer, "%s", serverID.c_str());
	n = write(sockfd, buffer, sizeof(buffer));
	if (n < 0)
	{
		cerr<<("client: ERROR writing to socket\n");
		close(sockfd);
		return;
	}
	end = chrono::system_clock::now();
	cout<<"bandwidth: "<< BUFLEN*1000/1024.0/chrono::duration_cast<chrono::microseconds>(end - start).count()<<endl;

	close(sockfd);
}

void Client_node::run()
{
	portno = DEFAULTPORT;
	insertNode(Util::getLocalIp()+" "+to_string(DEFAULTPORT));
	while(true)
	{
		for( auto i: this->neighbors)
		{
			vector<string> v;
			Util::splitIpPort(i, v);
			communicate(v[0], atoi(v[1].c_str()));
		}
		std::this_thread::sleep_for(chrono::seconds(SLEEPDURATION));
	}
}

int Client_node::init()
{
	thd = thread([=] {run(); });
	pingthd = thread([=] {pingOtherNodes(); });

	return 0;
}


void Client_node::pingOtherNodes()
{
	string pingserver_v3 = Util::getIpV3(Util::getLocalIp());

	int startaddr = 110;
	while(true)
	{
		if (startaddr == 255)
		{
			startaddr = 1;
			continue;
		}
		std::this_thread::sleep_for(chrono::milliseconds(500));

		string pingserver = pingserver_v3 + to_string(startaddr++);

		int sockfd;
		string cn_info;
		struct sockaddr_in addrin;
		addrin.sin_family = AF_INET;
		addrin.sin_addr.s_addr = inet_addr(pingserver.c_str());
		addrin.sin_port = htons(DEFAULTPORT);

		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		fcntl(sockfd, F_SETFL, O_NONBLOCK);
		if (sockfd < 0)
		{
			std::this_thread::sleep_for(chrono::seconds(1));
			close(sockfd);
			continue;
		}
	//	auto start = chrono::system_clock::now();
		cn_info = pingserver+" "+to_string(DEFAULTPORT);
		fd_set fdset;
		FD_ZERO(&fdset);
		FD_SET(sockfd, &fdset);
	    struct timeval tv;
		tv.tv_sec = 0; /* 10 second timeout */
		tv.tv_usec = 1000*500;
		connect(sockfd, (struct sockaddr *) &addrin, sizeof(addrin));
		if (select(sockfd + 1, NULL, &fdset, NULL, &tv) == 1)
		{
			int so_error;
			socklen_t len = sizeof so_error;
			getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &so_error, &len);

			if (so_error == 0)
			{
//				client: successfully connected to host
				this->insertNode(cn_info);
				cerr<<cn_info <<" is open\n";
			}
		}
		else
		{
//			client: ERROR connecting to host
//			cerr<<"closed\n";
		}

		close(sockfd);
	}
}
