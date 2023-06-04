#include <WinSock2.h>
#include "Client.h"

#pragma comment(lib, "Ws2_32.lib")

namespace Net {
	Client::Client(string ServerPort)
	{
        int iResult;
        iResult = WSAStartup(MAKEWORD(2, 2), &wsa);
        if (iResult != 0) {
            printf("WSAStartup failed: %d\n", iResult);
            exit(EXIT_FAILURE);
        }
        port = ServerPort;
	}
    void* Client::get_in_addr(struct sockaddr* sa)
    {
        if (sa->sa_family == AF_INET) {
            return &(((struct sockaddr_in*)&sa)->sin_addr);
        }
        return &(((struct sockaddr_in6*)&sa)->sin6_addr);
    }
    float Client::getFreeMemory() {
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
    void Client::closeConnection() {
        closesocket(sockfd);
        cout << "Connection closed" << endl;
    }
	int Client::getCpuUsage() {
        int res = 0;
        HRESULT hres;

        hres = CoInitializeEx(0, COINIT_MULTITHREADED);
        if (FAILED(hres)) {
            std::cerr << "Failed to initialize COM library. Error code: " << std::hex << hres << std::endl;
            return 1;
        }

        hres = CoInitializeSecurity(
            NULL,
            -1,
            NULL,
            NULL,
            RPC_C_AUTHN_LEVEL_DEFAULT,
            RPC_C_IMP_LEVEL_IMPERSONATE,
            NULL,
            EOAC_NONE,
            NULL
        );
        if (FAILED(hres)) {
            CoUninitialize();
            std::cerr << "Failed to initialize WMI security. Error code: " << std::hex << hres << std::endl;
            return 1;
        }

        IWbemLocator* pLoc = NULL;
        hres = CoCreateInstance(
            CLSID_WbemLocator,
            0,
            CLSCTX_INPROC_SERVER,
            IID_IWbemLocator,
            reinterpret_cast<LPVOID*>(&pLoc)
        );
        if (FAILED(hres)) {
            CoUninitialize();
            std::cerr << "Failed to create IWbemLocator object. Error code: " << std::hex << hres << std::endl;
            return 1;
        }

        IWbemServices* pSvc = NULL;
        hres = pLoc->ConnectServer(
            _bstr_t(L"ROOT\\CIMV2"),
            NULL,
            NULL,
            0,
            NULL,
            0,
            0,
            &pSvc
        );
        if (FAILED(hres)) {
            pLoc->Release();
            CoUninitialize();
            std::cerr << "Failed to connect to WMI namespace. Error code: " << std::hex << hres << std::endl;
            return 1;
        }

        hres = CoSetProxyBlanket(
            pSvc,
            RPC_C_AUTHN_WINNT,
            RPC_C_AUTHZ_NONE,
            NULL,
            RPC_C_AUTHN_LEVEL_CALL,
            RPC_C_IMP_LEVEL_IMPERSONATE,
            NULL,
            EOAC_NONE
        );
        if (FAILED(hres)) {
            pSvc->Release();
            pLoc->Release();
            CoUninitialize();
            std::cerr << "Failed to set proxy blanket. Error code: " << std::hex << hres << std::endl;
            return 1;
        }

        IEnumWbemClassObject* pEnumerator = NULL;
        hres = pSvc->ExecQuery(
            bstr_t("WQL"),
            bstr_t("SELECT LoadPercentage FROM Win32_Processor"),
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
            NULL,
            &pEnumerator
        );
        if (FAILED(hres)) {
            pSvc->Release();
            pLoc->Release();
            CoUninitialize();
            std::cerr << "Failed to execute WMI query. Error code: " << std::hex << hres << std::endl;
            return 1;
        }

        IWbemClassObject* pclsObj = NULL;
        ULONG uReturn = 0;
        while (pEnumerator) {
            hres = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
            if (0 == uReturn) {
                break;
            }

            VARIANT vtProp;
            hres = pclsObj->Get(L"LoadPercentage", 0, &vtProp, 0, 0);
            if (SUCCEEDED(hres)) {
                //std::cout << "CPU Usage: " << vtProp.uintVal << "%" << std::endl;
                res = vtProp.uintVal;
                VariantClear(&vtProp);
            }

            pclsObj->Release();
        }

        pEnumerator->Release();
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return res;
	}
    void Client::sendSystemStatus() {
        char buf[128];
        struct ClientData data;
        data.freeMemSpace = getFreeMemory();
        data.cpuUsage = getCpuUsage();
        memcpy(buf, &data, sizeof(data)); // from struct to char array;
        if (send(sockfd, buf, sizeof(buf) + 1, 0) == -1) {
            perror("send");
        }
    }
    void Client::connectDefault() 
    {
        init();
        for (p = res; p != NULL; p = p->ai_next) {
            if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
                perror("client: socket");
                continue;
            }
            if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
                closesocket(sockfd);
                perror("client: connect");
                continue;
            }
            break;
        }
        if (p == NULL) {
            fprintf(stderr, "client: failed to connect\n");
            EXIT_FAILURE;
        }
        inet_ntop(p->ai_family, get_in_addr((struct sockaddr*)p->ai_addr), hostAddr, sizeof(hostAddr));
        printf("client: connecting to %s\n", hostAddr);
        freeaddrinfo(res);
    }
    void Client::connectToHub(string serverPort) {
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        if ((rv = getaddrinfo(NULL, serverPort.c_str(), &hints, &res)) != 0) {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
            EXIT_FAILURE;
        }
        for (p = res; p != NULL; p = p->ai_next) {
            if ((hubfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
                perror("client: socket");
                continue;
            }
            if (connect(hubfd, p->ai_addr, p->ai_addrlen) == -1) {
                closesocket(hubfd);
                perror("client: connect");
                continue;
            }
            break;
        }
        if (p == NULL) {
            fprintf(stderr, "client: failed to connect\n");
            EXIT_FAILURE;
        }
        inet_ntop(p->ai_family, get_in_addr((struct sockaddr*)p->ai_addr), hostAddr, sizeof(hostAddr));
        printf("client: connecting to %s\n", hostAddr);
        freeaddrinfo(res);
    }
    void Client::init() {
        memset(&hints,0,sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        if ((rv = getaddrinfo(NULL, port.c_str(), &hints, &res)) != 0) {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
            EXIT_FAILURE;
        }
    }
    void Client::selectTaskEnum() {
        auto start = std::chrono::steady_clock::now();
        int iterations = 0;
        int number = 0;
        switch (taskData.task)
        {
        case Task::Myltiply:
            while (true) {
                auto current = std::chrono::steady_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::seconds>(current - start).count();
                if (duration >= 10) {
                    break;
                }
            }
            sendTaskResults(taskData.a * taskData.b);
            break;
        case Task::Divide:
            sendTaskResults(taskData.a / taskData.b);
            break;
        default:
            sendTaskResults(NULL);
            break;
        }
    }
    void Client::selectTask() {
        string tempTask = task;
        if (tempTask == "do increment") {
            int number = 0;
            for (size_t i = 0; i < 100; i++)
            {
                number++;
            }
            sendTaskResults(number);
        }
        else if (tempTask == "do decrement") {
            int number = 0;
            for (size_t i = 0; i < 100; i++)
            {
                number--;
            }
            sendTaskResults(number);
        }
    }
    void Client::receiveTask() {
        if ((numbytes = recv(sockfd, task, sizeof(task), 0)) == -1) {
            perror("send");
        }
        memcpy(&taskData, task, sizeof(taskData));
        //task[numbytes] = '\0';
    }
    void Client::sendTaskResults(double data) {
        char buf[128];
        struct ClientData clientData;
        clientData.freeMemSpace = getFreeMemory();
        clientData.cpuUsage = getCpuUsage();
        clientData.data = data;
        memcpy(buf, &clientData, sizeof(clientData)); // from struct to char array;

        if (send(sockfd, buf, size(buf), 0) == -1) {
            perror("send");
        }
    }
	Client::~Client()
	{
        WSACleanup();
	}
}