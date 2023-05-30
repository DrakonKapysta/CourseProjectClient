#pragma once
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
#include <cassert>
#include <unordered_map>
#include <vector>
#include <errno.h>
#include <tchar.h>
#include <strsafe.h>
#include <Windows.h>
#include <stdio.h> 
#include <istream>
#include <fstream>

#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "pdh.lib")
#pragma comment(lib, "Ws2_32.lib")

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
		//void connectTo(string ServerPort);
		void receiveMessage();
		void sendMessage();
		void sendSystemStatus();
		void closeConnection();
		
	private:
		addrinfo hints, * p, * res;
		string msg, port;
		int sockfd, numbytes, rv;
		char hostAddr[INET6_ADDRSTRLEN];
		WSADATA wsa;
	private:
		void* get_in_addr(struct sockaddr*);
		void init();
		struct systemData { // struct with system info
			int cpuUsage;
			double freeMemory;
		};
	};

	
}