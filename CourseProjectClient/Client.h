#pragma once
#include <windows.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <pdh.h>
#include <pdhmsg.h>
#include <comdef.h>
#include <Wbemidl.h>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <string>

#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "pdh.lib")
#pragma comment (lib, "Ws2_32.lib")
using namespace std;

namespace Net {
	class Client
	{
	public:
		Client(string ServerPort);
		~Client();
		double getFreeMemory();
		int getCpuUsage();
		void connectDefault();
		void connectTo(string ServerPort);
	private:
		addrinfo hints, * p, * res;
		int rv;
		string msg;
		int sockfd, numbytes;
		string port;
		char s[INET6_ADDRSTRLEN];
		WSADATA wsa;
		void* get_in_addr(struct sockaddr*);
	private:
		void init();
	};

	
}