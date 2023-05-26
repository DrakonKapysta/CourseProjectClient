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
	int Client::getCpuUsage() {
        int res = 0;
        HRESULT hres;

        // Инициализация COM библиотеки
        hres = CoInitializeEx(0, COINIT_MULTITHREADED);
        if (FAILED(hres)) {
            std::cerr << "Failed to initialize COM library. Error code: " << std::hex << hres << std::endl;
            return 1;
        }

        // Инициализация WMI
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
	Client::~Client()
	{
	}
}