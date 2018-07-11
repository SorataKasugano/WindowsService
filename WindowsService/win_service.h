#pragma once

/* Windows service operations encapsulation with a singleton
*  @file win_service.h
*  @brief WinService class£¬provide install/uninstall/register/run interface for windows service
*  @author S.K.
*  @note Be careful for compatibility about UNICODE and ANSI
*  @date 3.14.2018 */

#undef UNICODE
#include <string>
#include <Windows.h>

class WinService
{
public:
	WinService(const WinService&) = delete;
	WinService& operator=(const WinService&) = delete;
	~WinService() = default;

	static WinService& Instance();

	static void WINAPI ServiceControlHandler(DWORD dwControlCode);
	static void WINAPI ServiceMain(DWORD argc, LPTSTR *argv);
	void WINAPI MyServiceMain(DWORD argc, LPTSTR *argv);
	/* access application logic
	*  @note you can invoke it immediately without a windows service process */
	void MainProcess(DWORD argc, LPTSTR *argv);

	bool SetServiceStatus();
	void OnServiceStopped();
	
	bool IsInstalled();
	void InstallService();
	void UninstallService();
	/* launch as a windows service process*/
	bool RunService();

private:
	WinService() = default;
	// change service name here
	std::string m_strServiceName{ "TestService" };

private:
	SERVICE_STATUS_HANDLE	m_hServiceHandle;
	SERVICE_STATUS			m_serviceStatus;
	HANDLE					m_hEventStopService;
};


