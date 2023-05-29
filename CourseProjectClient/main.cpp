#include <iostream>
#include "client.h"

using namespace std;

int main(int argc, char* argv[]) {
	Net::Client client("27015");
	std::cout << "Unused memory: " << client.getFreeMemory() << " gb" << std::endl;
	while (true) {
		std::cout << "CPU Usage: " << client.getCpuUsage() << "%" << std::endl;
	}

}