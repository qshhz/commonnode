
#include <sys/utsname.h>
#include <arpa/inet.h>
#include <vector>
#include <set>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "Util.h"

using namespace std;

string Util::getIpV3(string ipv4)
{
	string ipv3 = "";
	int v = 0;
	int i = 0;

	while(v < 3)
	{
		char c = ipv4[i++];
		if(c == '.')
		{
			v++;
		}
		ipv3 += c;
	}


	return ipv3;
}

string Util::getLocalIp()
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

void Util::splitIpPort(string ipandport, vector<string>& v)
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
