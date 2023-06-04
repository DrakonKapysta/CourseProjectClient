#include <iostream>
#include <WinSock2.h>
#include "Client.h"

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

int main(int argc, char* argv[]) {
	Net::Client client("27015");
	client.connectDefault();
	client.sendSystemStatus();
	while (true)
	{
		if (client.receiveTask() == -1)
		{
			break;
		}

		if (client.selectTaskEnum() == -1)
		{
			break;
		}
	}
	client.closeConnection();
}
