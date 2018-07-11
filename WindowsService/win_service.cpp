#include "win_service.h"

WinService & WinService::Instance()
{
	// instantiated in first use, guaranteed to be destroyed
	static WinService instance;
	return instance;
}

void WinService::ServiceControlHandler(DWORD dwControlCode)
{
	switch (dwControlCode)
	{
	case SERVICE_CONTROL_SHUTDOWN:
	case SERVICE_CONTROL_STOP:
		WinService::Instance().OnServiceStopped();
		return;
		/* ...other control codes handler */
	default:
		break;
	}
	WinService::Instance().SetServiceStatus();
}

void WinService::ServiceMain(DWORD argc, LPTSTR * argv)
{
	WinService::Instance().MyServiceMain(argc, argv);
}

bool WinService::SetServiceStatus()
{
	return ::SetServiceStatus(m_hServiceHandle, &m_serviceStatus);
}

bool WinService::IsInstalled()
{
	bool bResult = false;
	SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if (hSCM != NULL)
	{
		SC_HANDLE hService = OpenService(hSCM, m_strServiceName.data(), SERVICE_QUERY_CONFIG);

		if (hService != NULL)
		{
			bResult = true;

			CloseServiceHandle(hService);
		}

		CloseServiceHandle(hSCM);
	}

	return bResult;
}

void WinService::InstallService()
{
	if (IsInstalled())
	{
		printf("service have already install,please uninstall first\n");
		return;
	}

	SC_HANDLE hControlManager = OpenSCManager(0, 0, SC_MANAGER_CREATE_SERVICE);

	if (hControlManager)
	{
		TCHAR path[_MAX_PATH + 1];

		if (GetModuleFileName(0, path, sizeof(path) / sizeof(path[0])) > 0)
		{
			SC_HANDLE hService = CreateService(hControlManager,
				m_strServiceName.data(), m_strServiceName.data(),
				SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
				SERVICE_AUTO_START, SERVICE_ERROR_IGNORE, path,
				0, 0, 0, 0, 0);

			if (hService)
			{
				CloseServiceHandle(hService);
			}
		}

		CloseServiceHandle(hControlManager);
	}
	else
	{
		printf("open service control manager error\n");
	}
}

void WinService::UninstallService()
{
	if (!IsInstalled())
	{
		printf("service not checked\n");
		return;
	}

	DWORD dwOption = SC_MANAGER_CONNECT;

	SC_HANDLE hControlManager = OpenSCManager(0, 0, dwOption);

	if (hControlManager)
	{
		dwOption = SERVICE_QUERY_STATUS | DELETE;

		SC_HANDLE hService = OpenService(hControlManager,
			m_strServiceName.data(), dwOption);

		if (hService)
		{
			SERVICE_STATUS m_serviceStatus;

			if (QueryServiceStatus(hService, &m_serviceStatus))
			{
				if (m_serviceStatus.dwCurrentState == SERVICE_STOPPED)
				{
					DeleteService(hService);
				}
				else
				{
					printf("please stop service first\n");
				}
			}

			CloseServiceHandle(hService);
		}

		CloseServiceHandle(hControlManager);
	}
}


void WinService::MyServiceMain(DWORD argc, LPTSTR *argv)
{
	// initialize service status
	m_serviceStatus.dwServiceType = SERVICE_WIN32;
	m_serviceStatus.dwCurrentState = SERVICE_STOPPED;
	m_serviceStatus.dwWin32ExitCode = NO_ERROR;

	m_serviceStatus.dwServiceSpecificExitCode = NO_ERROR;
	m_serviceStatus.dwControlsAccepted = 0;
	m_serviceStatus.dwCheckPoint = 0;
	m_serviceStatus.dwWaitHint = 0;

	m_hServiceHandle = RegisterServiceCtrlHandler(m_strServiceName.data(), ServiceControlHandler);

	if (m_hServiceHandle)
	{
		m_serviceStatus.dwCurrentState = SERVICE_START_PENDING;
		SetServiceStatus();

		m_hEventStopService = CreateEvent(0, FALSE, FALSE, 0);

		// configure valid service action
		m_serviceStatus.dwControlsAccepted |= (SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN);
		m_serviceStatus.dwCurrentState = SERVICE_RUNNING;
		SetServiceStatus();

		// access application logic, function returns followed by service stopping
		MainProcess(argc, argv);

		m_serviceStatus.dwCurrentState = SERVICE_STOP_PENDING;
		SetServiceStatus();

		CloseHandle(m_hEventStopService);
		m_hEventStopService = nullptr;

		// stopping service,deny disturbances
		m_serviceStatus.dwControlsAccepted &= ~(SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN);
		m_serviceStatus.dwCurrentState = SERVICE_STOPPED;
		SetServiceStatus();
	}
}

void WinService::MainProcess(DWORD argc, LPTSTR *argv)
{
	if (argc > 1 &&
		(strcmp(argv[1], "-console") == 0 || strcmp(argv[1], "-cli") == 0))
	{
		// TODO:launch your application here


		char strCmd[1024] = "", strLastCmd[1024] = "";

		while (true)
		{
			printf("Console Demo->");
			fgets(strCmd, 128, stdin);
			if (strlen(strCmd) > 0)
			{
				strCmd[strlen(strCmd) - 1] = '\0';
			}

			if (strcmp(strCmd, ".") == 0)
			{
				strcpy_s(strCmd, strLastCmd); // repeat the latest command
			}
			
			if (strcmp(strCmd, "quit") == 0 || strcmp(strCmd, "exit") == 0)
			{
				printf("Good bye!\n");
				break;
			}

			strcpy_s(strLastCmd, strCmd);
		}
	}
	else	
	{
		// TODO:launch your application here


		// if stop event has been trigger
		while (WaitForSingleObject(m_hEventStopService, 100) == WAIT_TIMEOUT)
		{
		}
	}

	// TODO:clean your application resources and quit here


}

bool WinService::RunService()
{
	SERVICE_TABLE_ENTRY serviceTable[] =
	{
		{ (LPSTR)m_strServiceName.data(), ServiceMain },
		{ 0, 0 }
	};
	int err = 0;
	if (!StartServiceCtrlDispatcher(serviceTable))
		err = GetLastError();
	return err;
}

void WinService::OnServiceStopped()
{
	m_serviceStatus.dwCurrentState = SERVICE_STOP_PENDING;
	SetServiceStatus();
	SetEvent(m_hEventStopService);
}