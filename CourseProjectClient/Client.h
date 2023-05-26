#pragma once
#include <windows.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <pdh.h>
#include <pdhmsg.h>
#include <comdef.h>
#include <Wbemidl.h>

#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "pdh.lib")

namespace Net {
	class Client
	{
	public:
		Client();
		~Client();
		double getFreeMemory();
		int getCpuUsage();
	private:
	};

	
}