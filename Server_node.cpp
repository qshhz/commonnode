#include "Server_node.h"
#include "Base_node.h"
#include "Util.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/utsname.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <iostream>
#include <string.h>


void Server_node::run()
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

int Server_node::init()
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


