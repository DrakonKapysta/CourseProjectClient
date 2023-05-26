#include <iostream>
#include "client.h"

using namespace std;

int main(int argc, char* argv[]) {
	Net::Client client;
	std::cout << "Unused memory: " << client.getFreeMemory() << " gb" << std::endl;

}