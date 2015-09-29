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
#include "Util.h"
#include "Base_node.h"
#include "Client_node.h"
#include "Server_node.h"
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
	ComNode cn;
	cn.start();

	return 0;
}

