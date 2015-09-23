
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




#define BUFLEN 256*1024

int client()
{
	int sockfd, portno, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;

	char buffer[BUFLEN] = "test send";
//	if (argc < 3) {
//		fprintf(stderr, "usage %s hostname port\n", argv[0]);
//		exit(0);
//	}
	portno = 50000;// atoi(argv[2]);
	server = gethostbyname("10.0.10.231");//gethostbyname(argv[1]);
	if (server == NULL) {
		fprintf(stderr, "ERROR, no such host\n");
		exit(0);
	}
	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	memcpy((char *) server->h_addr, (char *) &serv_addr.sin_addr.s_addr,
			server->h_length);

	while(true)
	{
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		serv_addr.sin_port = htons(portno);
		if (sockfd < 0)
			cerr<<("ERROR opening socket");
		auto start = chrono::system_clock::now();
//		auto start = clock();

		if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
			perror("ERROR connecting");
		auto end = chrono::system_clock::now();
		cout<<"time elapsed: "<< chrono::duration_cast<chrono::microseconds>(end - start).count() / 1000.0<<endl;
		start = end;
//	//	printf("Please enter the message: ");
//	//	memset(buffer, 0, BUFLEN);
//	//	fgets(buffer, BUFLEN-1, stdin);
		n = write(sockfd, buffer, sizeof(buffer));
		if (n < 0)
			cerr<<("ERROR writing to socket");
		end = chrono::system_clock::now();
		cout<<"bytes: "<< BUFLEN*1000/1024.0/chrono::duration_cast<chrono::microseconds>(end - start).count()<<endl;
//	//	memset(buffer, 0, BUFLEN);
//		n = read(sockfd, buffer, BUFLEN-1);
//		if (n < 0)
//			cerr<<("ERROR reading from socket");
//		cout<<"time elapsed: "<< chrono::duration_cast<chrono::microseconds>((chrono::system_clock::now() - start)).count() / 1000.0<<endl;
//		cout<<"time elapsed: "<< chrono::duration_cast<chrono::microseconds>((chrono::system_clock::now() - start)).count() / 1000.0<<endl;
		close(sockfd);
		std::this_thread::sleep_for(chrono::seconds(20));
	}
	printf("%s\n", buffer);
	return 0;
}


int main(int argc, char *argv[])
{
	return client();
}
