#ifndef _UTILT_H_
#define _UTILT_H_


#include <sys/utsname.h>
#include <arpa/inet.h>
#include <vector>
#include <set>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

using namespace std;


#define LOCALHOST "127.0.0.1"

class Util
{
public:
	static string getIpV3(string ipv4);

	static string getLocalIp();

	static void splitIpPort(string ipandport, vector<string>& v);
};



#endif
