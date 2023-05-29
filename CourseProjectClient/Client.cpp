#include "Client.h"

namespace Net {
	Client::Client(string ServerPort)
	{
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
            fprintf(stderr, "WSAStartup failed.\n");
            exit(1);
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
        this->msg = "Unused memory: " + to_string(getFreeMemory()) + " gb; "
            + "CPU Usage: " + to_string(getCpuUsage()) + "%;";
        if (send(sockfd, msg.c_str(), size(msg) + 1, 0) == -1) {
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
        sendSystemStatus();
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
    void Client::receiveMessage() {

    }
    void Client::sendMessage() {

    }
	Client::~Client()
	{
        WSACleanup();
	}
}