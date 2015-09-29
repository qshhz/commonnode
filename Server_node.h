#ifndef _SERVER_NODE_H_
#define _SERVER_NODE_H_


#include <string>
#include <set>
#include <iostream>
#include "Client_node.h"
#include "Util.h"
using namespace std;

class Server_node:public Base_node
{
public:
	void run();

	int init();

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

#endif
