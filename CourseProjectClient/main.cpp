#include <iostream>
#include <WinSock2.h>
#include "Client.h"

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

int main(int argc, char* argv[]) {
	Net::Client client("27015");
	client.connectDefault();
	client.receiveTask();
	//client.selectTask();
	client.selectTaskEnum();
	client.closeConnection();
}