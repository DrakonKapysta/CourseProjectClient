#include "Client.h"

namespace Net {
	Client::Client()
	{
	}

	double Client::getFreeMemory() {
		MEMORYSTATUSEX memoryStatus;
		memoryStatus.dwLength = sizeof(memoryStatus);

		if (GlobalMemoryStatusEx(&memoryStatus)) {
			DWORDLONG unusedMemory = memoryStatus.ullAvailPhys;
			return unusedMemory/1000000000.0;
		}
		else {
			std::cerr << "Failed to get memory status. Error code: " << GetLastError() << std::endl;
			return 1;
		}
	}

	Client::~Client()
	{
	}
}