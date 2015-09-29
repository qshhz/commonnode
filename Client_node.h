#ifndef _CLIENT_NODE_H_
#define _CLIENT_NODE_H_

#include <string>
#include <mutex>
#include <set>
#include <iostream>
#include "Base_node.h"
using namespace std;


class Client_node:public Base_node{
private:
	void communicate(string serverip, int port);

public:
	void run();
	int init();
	void pingOtherNodes();

	void insertNode(string neighborid)
	{
		unique_lock<mutex> lock(mt);
		if(!neighborid.empty())
			this->neighbors.insert(neighborid);
		lock.unlock();
	}
	void removeNode(string neighborid)
	{
		unique_lock<mutex> lock(mt);
		this->neighbors.erase(neighborid);
		lock.unlock();
	}

	void setLocalServer(string id)
	{
		serverID = id;
	}

private:
	string serverID;
	set<string> neighbors;
	mutex mt;

	thread pingthd;
};

#endif
