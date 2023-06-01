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

enum class Tasks {
	DoIncrement,
	DoDecrement,
};

namespace Net {
	class Client
	{
	public:
		Client(string ServerPort);
		~Client();
		float getFreeMemory();
		int getCpuUsage();
		void connectDefault();
		void connectToHub(string serverPort);
		void receiveTask();
		void sendTaskResults(string data);
		void sendSystemStatus();
		void closeConnection();
		void selectTask();
		void selectTaskEnum();
		
	private:
		addrinfo hints, * p, * res;
		Tasks taskEnum;
		string msg, port;
		char task[128];
		int sockfd, numbytes, rv, hubfd;
		char hostAddr[INET6_ADDRSTRLEN];
		WSADATA wsa;
	private:
		void* get_in_addr(struct sockaddr*);
		void init();
		struct ClientData
		{
			int cpuUsage{};
			double freeMemSpace{};
			double data{};
		};
		
	};
}