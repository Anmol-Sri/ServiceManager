#include "StdAfx.h"
#include "ServiceManager.h"
#include <type_traits>
#include <memory>
#include <stddef.h>

Service& Service::operator=(Service& service)
{
	this->display_name = service.display_name;
	this->name = service.name;
	this->pid = service.pid;
	this->status = service.status;

	return *this;
}

bool ServiceManager::Initialize()
{
	manager = OpenSCManager(NULL,SERVICES_ACTIVE_DATABASE,SC_MANAGER_ALL_ACCESS);
	//manager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	return (manager!=0);
}

void ServiceManager::CleanUp()
{
	if(manager)
	{
		CloseServiceHandle(manager);
		manager = 0;
	}
}
/*
int ServiceManager::EnumerateAllServices() {
	void* buf = NULL;
	DWORD bufSize = 0;
	DWORD moreBytesNeeded, serviceCount;
	for (;;) {
		printf("Calling EnumServiceStatusEx with bufferSize %d\n", bufSize);
		if (EnumServicesStatusEx(
			manager,
			SC_ENUM_PROCESS_INFO,
			SERVICE_WIN32,
			SERVICE_STATE_ALL,
			(LPBYTE)buf,
			bufSize,
			&moreBytesNeeded,
			&serviceCount,
			NULL,
			NULL)) {
			ENUM_SERVICE_STATUS_PROCESS* services = (ENUM_SERVICE_STATUS_PROCESS*)buf;
			int cnt = 0;
			for (DWORD i = 0; i < serviceCount; ++i, cnt++) {
				//printf("%s\n", services[i].lpServiceName);
				cout << "Service Number : " << cnt << endl;
				cout << _T(services[i].lpServiceName) << endl;
			}
			free(buf);
			return 0;
		}
		int err = GetLastError();
		if (ERROR_MORE_DATA != err) {
			free(buf);
			return err;
		}
		bufSize += moreBytesNeeded;
		free(buf);
		buf = malloc(bufSize);
	}
}
*/

CString ServiceManager::GetExePath(const TCHAR* name) {
	SC_HANDLE hService = OpenService(manager, name, SERVICE_QUERY_CONFIG);
	DWORD dwBytesNeeded = 0, dwBufSize;
	LPQUERY_SERVICE_CONFIG lpsc;
	if (!QueryServiceConfig(hService, NULL, 0, &dwBytesNeeded))
	{
		DWORD dwError = GetLastError();
		if (ERROR_INSUFFICIENT_BUFFER == dwError)
		{
			dwBufSize = dwBytesNeeded;
			lpsc = (LPQUERY_SERVICE_CONFIG)HeapAlloc(GetProcessHeap(), 0, dwBufSize);
		}
		else
		{
			CloseServiceHandle(hService);
		}
	}
	string ans = "";
	if (QueryServiceConfig(hService, lpsc, dwBufSize, &dwBytesNeeded))
	{
		//lpsc->lpBinaryPathName contains exe 
		ans = (lpsc->lpBinaryPathName);
	}
	HeapFree(GetProcessHeap(), 0, lpsc);
	CloseServiceHandle(hService);
	return ans.c_str();
}

bool ServiceManager::GetServices(CServiceArray& services, int status_mask)
{
	services.RemoveAll();
	/*
	//form the mask
	int mask = 0;
	if((status_mask&Status::Active) && (status_mask&Status::Inactive))
		mask = SERVICE_STATE_ALL;
	else if(status_mask&Status::Active)
		mask = SERVICE_ACTIVE;
	else if(status_mask&Status::Inactive)
		mask = SERVICE_INACTIVE;
	else
		return false;

	int maxnum = 64000/sizeof(ENUM_SERVICE_STATUS_PROCESS);
	ENUM_SERVICE_STATUS_PROCESS *pData = new ENUM_SERVICE_STATUS_PROCESS[maxnum];
	int size = maxnum*sizeof(ENUM_SERVICE_STATUS_PROCESS); //the maximum
	DWORD needed = 0;
	DWORD num = 0;
	DWORD pos = 0;


	//cout << "Checking the GetServices" << endl;
	if (!EnumServicesStatusEx(manager, SC_ENUM_PROCESS_INFO, SERVICE_DRIVER | SERVICE_WIN32,
		mask, (BYTE*)pData, size, &needed, &num, &pos, NULL))
	{
		cout << "Error While EnumServiceStatusEx" << endl;
		delete[] pData;
		return false; 
	}
	*/
	void* buf = NULL;
	DWORD bufSize = 0;
	DWORD moreBytesNeeded, serviceCount;
	for (;;) {
		printf("Calling EnumServiceStatusEx with bufferSize %d\n", bufSize);
		if (EnumServicesStatusEx(
			manager,
			SC_ENUM_PROCESS_INFO,
			SERVICE_WIN32,
			SERVICE_STATE_ALL,
			(LPBYTE)buf,
			bufSize,
			&moreBytesNeeded,
			&serviceCount,
			NULL,
			NULL)) {
			ENUM_SERVICE_STATUS_PROCESS* pData = (ENUM_SERVICE_STATUS_PROCESS*)buf;
			int cnt = 0;
			for (DWORD i = 0; i < serviceCount; ++i, cnt++) {
				//printf("%s\n", services[i].lpServiceName);
				//cout << "Service Number : " << cnt << endl;
				//cout << _T(services[i].lpServiceName) << endl;
				Service s;
				s.name = pData[i].lpServiceName;
				//cout << "Inside the loop" << endl;
				s.display_name = pData[i].lpDisplayName;
				s.pid = pData[i].ServiceStatusProcess.dwProcessId;
				s.status = (pData[i].ServiceStatusProcess.dwCurrentState == SERVICE_STOPPED) ? Inactive : Active;
				services.Add(s);
			}
			//delete[] pData;
			free(buf);
			return true;
		}
		int err = GetLastError();
		if (ERROR_MORE_DATA != err) {
			free(buf);
			return err;
		}
		bufSize += moreBytesNeeded;
		free(buf);
		buf = malloc(bufSize);
	}
	/*
	//fill the array
	for(int i=0; i<num; ++i)
	{
		Service s;
		s.name = pData[i].lpServiceName;
		//cout << "Inside the loop" << endl;
		s.display_name = pData[i].lpDisplayName;
		s.pid = pData[i].ServiceStatusProcess.dwProcessId;
		s.status = (pData[i].ServiceStatusProcess.dwCurrentState==SERVICE_STOPPED)?Inactive:Active;
		services.Add(s);
	}
	
	delete [] pData;
	*/
	return false;
}

bool ServiceManager::StartService(const TCHAR* name)
{
	SC_HANDLE serviceHandle = OpenService(manager, name, SC_MANAGER_ALL_ACCESS);
	SERVICE_STATUS_PROCESS status;
	DWORD bytesNeeded;
	bool ok = true;
	QueryServiceStatusEx(serviceHandle, SC_STATUS_PROCESS_INFO, (LPBYTE)&status, sizeof(SERVICE_STATUS_PROCESS), &bytesNeeded);
	if (status.dwCurrentState == SERVICE_RUNNING) {
		cout << "Service Already Running\n";
	}
	else {
		//Start it.
		BOOL b = ::StartService(serviceHandle, 0, NULL);
		ok = b;
		if (b) {
			cout << "Service started." << endl;
		}
		else {
			cout << "Service failed to start." << endl;
		}
	}
	CloseServiceHandle(serviceHandle);
	return ok;
	/*
	SC_HANDLE service = OpenService(manager,name,SERVICE_START);
	if(!service)
		return false;

	bool ret = ::StartService(service,0,NULL);
	if (ret) cout << "True\n"; else cout << "False\n";
	CloseServiceHandle(service);
	cout << "Service Started Successfully\n";
	return ret;
	*/
}

bool ServiceManager::StopService(const TCHAR* name)
{
	SC_HANDLE serviceHandle = OpenService(manager, name, SC_MANAGER_ALL_ACCESS);
	SERVICE_STATUS_PROCESS status;
	DWORD bytesNeeded;
	QueryServiceStatusEx(serviceHandle, SC_STATUS_PROCESS_INFO, (LPBYTE)&status, sizeof(SERVICE_STATUS_PROCESS), &bytesNeeded);
	bool ok = true;
	if (status.dwCurrentState == SERVICE_RUNNING) {
		// Stop it.
		BOOL b = ControlService(serviceHandle, SERVICE_CONTROL_STOP, (LPSERVICE_STATUS)&status);
		ok = b;
		if (b) {
			cout << "Service stopped." << endl;
		}
		else {
			cout << "Service failed to stop." << endl;
		}
	}
	else {
		cout << "Service Already in a Stop State\n";
	}
	CloseServiceHandle(serviceHandle);
	return ok;
	/*
	SC_HANDLE service = OpenService(manager,name,SERVICE_STOP);
	if(!service)
		return false;

	SERVICE_STATUS status;
	bool ret = ControlService(service,SERVICE_CONTROL_STOP,&status);
	if (ret) cout << "True\n"; else cout << "False\n";
	CloseServiceHandle(service);
	cout << "Service Stopped Successfully\n";
	return ret;
	*/
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//								!!DO NOT REMOVE THE BELOW SECTION CODE!!							//
//////////////////////////////////////////////////////////////////////////////////////////////////////
// In Case the Above ServiceStart Fails:
//
// Purpose: 
//   Starts the service if possible.
//
// Parameters:
//   None
// 
// Return value:
//   None
//
/*
VOID __stdcall ServiceManager::DoStartSvc(const TCHAR* name)
{
	SERVICE_STATUS_PROCESS ssStatus;
	DWORD dwOldCheckPoint;
	DWORD dwStartTickCount;
	DWORD dwWaitTime;
	DWORD dwBytesNeeded;


	if (NULL == manager)
	{
		printf("OpenSCManager failed (%d)\n", GetLastError());
		return;
	}

	// Get a handle to the service.

	SC_HANDLE schService = OpenService(
		manager,         // SCM database 
		name,            // name of service 
		SERVICE_ALL_ACCESS);  // full access 

	if (schService == NULL)
	{
		printf("OpenService failed (%d)\n", GetLastError());
		CloseServiceHandle(manager);
		return;
	}

	// Check the status in case the service is not stopped. 

	if (!QueryServiceStatusEx(
		schService,                     // handle to service 
		SC_STATUS_PROCESS_INFO,         // information level
		(LPBYTE)&ssStatus,             // address of structure
		sizeof(SERVICE_STATUS_PROCESS), // size of structure
		&dwBytesNeeded))              // size needed if buffer is too small
	{
		printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
		CloseServiceHandle(schService);
		CloseServiceHandle(manager);
		return;
	}

	// Check if the service is already running. It would be possible 
	// to stop the service here, but for simplicity this example just returns. 

	if (ssStatus.dwCurrentState != SERVICE_STOPPED && ssStatus.dwCurrentState != SERVICE_STOP_PENDING)
	{
		printf("Cannot start the service because it is already running\n");
		CloseServiceHandle(schService);
		CloseServiceHandle(manager);
		return;
	}

	// Save the tick count and initial checkpoint.

	dwStartTickCount = GetTickCount();
	dwOldCheckPoint = ssStatus.dwCheckPoint;

	// Wait for the service to stop before attempting to start it.

	while (ssStatus.dwCurrentState == SERVICE_STOP_PENDING)
	{
		// Do not wait longer than the wait hint. A good interval is 
		// one-tenth of the wait hint but not less than 1 second  
		// and not more than 10 seconds. 

		dwWaitTime = ssStatus.dwWaitHint / 10;

		if (dwWaitTime < 1000)
			dwWaitTime = 1000;
		else if (dwWaitTime > 10000)
			dwWaitTime = 10000;

		Sleep(dwWaitTime);

		// Check the status until the service is no longer stop pending. 

		if (!QueryServiceStatusEx(
			schService,                     // handle to service 
			SC_STATUS_PROCESS_INFO,         // information level
			(LPBYTE)&ssStatus,             // address of structure
			sizeof(SERVICE_STATUS_PROCESS), // size of structure
			&dwBytesNeeded))              // size needed if buffer is too small
		{
			printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
			CloseServiceHandle(schService);
			CloseServiceHandle(manager);
			return;
		}

		if (ssStatus.dwCheckPoint > dwOldCheckPoint)
		{
			// Continue to wait and check.

			dwStartTickCount = GetTickCount();
			dwOldCheckPoint = ssStatus.dwCheckPoint;
		}
		else
		{
			if (GetTickCount() - dwStartTickCount > ssStatus.dwWaitHint)
			{
				printf("Timeout waiting for service to stop\n");
				CloseServiceHandle(schService);
				CloseServiceHandle(manager);
				return;
			}
		}
	}

	// Attempt to start the service.

	if (!::StartService(
		schService,  // handle to service 
		0,           // number of arguments 
		NULL))      // no arguments 
	{
		printf("StartService failed (%d)\n", GetLastError());
		CloseServiceHandle(schService);
		CloseServiceHandle(manager);
		return;
	}
	else printf("Service start pending...\n");

	// Check the status until the service is no longer start pending. 

	if (!QueryServiceStatusEx(
		schService,                     // handle to service 
		SC_STATUS_PROCESS_INFO,         // info level
		(LPBYTE)&ssStatus,             // address of structure
		sizeof(SERVICE_STATUS_PROCESS), // size of structure
		&dwBytesNeeded))              // if buffer too small
	{
		printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
		CloseServiceHandle(schService);
		CloseServiceHandle(manager);
		return;
	}

	// Save the tick count and initial checkpoint.

	dwStartTickCount = GetTickCount();
	dwOldCheckPoint = ssStatus.dwCheckPoint;

	while (ssStatus.dwCurrentState == SERVICE_START_PENDING)
	{
		// Do not wait longer than the wait hint. A good interval is 
		// one-tenth the wait hint, but no less than 1 second and no 
		// more than 10 seconds. 

		dwWaitTime = ssStatus.dwWaitHint / 10;

		if (dwWaitTime < 1000)
			dwWaitTime = 1000;
		else if (dwWaitTime > 10000)
			dwWaitTime = 10000;

		Sleep(dwWaitTime);

		// Check the status again. 

		if (!QueryServiceStatusEx(
			schService,             // handle to service 
			SC_STATUS_PROCESS_INFO, // info level
			(LPBYTE)&ssStatus,             // address of structure
			sizeof(SERVICE_STATUS_PROCESS), // size of structure
			&dwBytesNeeded))              // if buffer too small
		{
			printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
			break;
		}

		if (ssStatus.dwCheckPoint > dwOldCheckPoint)
		{
			// Continue to wait and check.

			dwStartTickCount = GetTickCount();
			dwOldCheckPoint = ssStatus.dwCheckPoint;
		}
		else
		{
			if (GetTickCount() - dwStartTickCount > ssStatus.dwWaitHint)
			{
				// No progress made within the wait hint.
				break;
			}
		}
	}

	// Determine whether the service is running.

	if (ssStatus.dwCurrentState == SERVICE_RUNNING)
	{
		printf("Service started successfully.\n");
	}
	else
	{
		printf("Service not started. \n");
		printf("  Current State: %d\n", ssStatus.dwCurrentState);
		printf("  Exit Code: %d\n", ssStatus.dwWin32ExitCode);
		printf("  Check Point: %d\n", ssStatus.dwCheckPoint);
		printf("  Wait Hint: %d\n", ssStatus.dwWaitHint);
	}

	CloseServiceHandle(schService);
	//CloseServiceHandle(manager);
}
*/


//////////////////////////////////////////////////////////////////////////////////////////////////////
//								!!DO NOT REMOVE THE BELOW SECTION CODE!!							//
//////////////////////////////////////////////////////////////////////////////////////////////////////
// In Case the Above ServiceStop Fails:


/*
//
// Purpose: 
//   Stops the service.
//
// Parameters:
//   None
// 
// Return value:
//   None
//
VOID __stdcall ServiceManager::DoStopSvc(const TCHAR* name)
{
	SERVICE_STATUS_PROCESS ssp;
	DWORD dwStartTime = GetTickCount();
	DWORD dwBytesNeeded;
	DWORD dwTimeout = 30000; // 30-second time-out
	DWORD dwWaitTime;

	if (NULL == manager)
	{
		printf("OpenSCManager failed (%d)\n", GetLastError());
		return;
	}

	// Get a handle to the service.

	SC_HANDLE schService = OpenService(
		manager,         // SCM database 
		name,            // name of service 
		SERVICE_STOP |
		SERVICE_QUERY_STATUS |
		SERVICE_ENUMERATE_DEPENDENTS);

	if (schService == NULL)
	{
		printf("OpenService failed (%d)\n", GetLastError());
		CloseServiceHandle(manager);
		return;
	}

	// Make sure the service is not already stopped.

	if (!QueryServiceStatusEx(
		schService,
		SC_STATUS_PROCESS_INFO,
		(LPBYTE)&ssp,
		sizeof(SERVICE_STATUS_PROCESS),
		&dwBytesNeeded))
	{
		printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
		goto stop_cleanup;
	}

	if (ssp.dwCurrentState == SERVICE_STOPPED)
	{
		printf("Service is already stopped.\n");
		goto stop_cleanup;
	}

	// If a stop is pending, wait for it.

	while (ssp.dwCurrentState == SERVICE_STOP_PENDING)
	{
		printf("Service stop pending...\n");

		// Do not wait longer than the wait hint. A good interval is 
		// one-tenth of the wait hint but not less than 1 second  
		// and not more than 10 seconds. 

		dwWaitTime = ssp.dwWaitHint / 10;

		if (dwWaitTime < 1000)
			dwWaitTime = 1000;
		else if (dwWaitTime > 10000)
			dwWaitTime = 10000;

		Sleep(dwWaitTime);

		if (!QueryServiceStatusEx(
			schService,
			SC_STATUS_PROCESS_INFO,
			(LPBYTE)&ssp,
			sizeof(SERVICE_STATUS_PROCESS),
			&dwBytesNeeded))
		{
			printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
			goto stop_cleanup;
		}

		if (ssp.dwCurrentState == SERVICE_STOPPED)
		{
			printf("Service stopped successfully.\n");
			goto stop_cleanup;
		}

		if (GetTickCount() - dwStartTime > dwTimeout)
		{
			printf("Service stop timed out.\n");
			goto stop_cleanup;
		}
	}

	// If the service is running, dependencies must be stopped first.

	StopDependentServices(schService);

	// Send a stop code to the service.

	if (!ControlService(
		schService,
		SERVICE_CONTROL_STOP,
		(LPSERVICE_STATUS)&ssp))
	{
		printf("ControlService failed (%d)\n", GetLastError());
		goto stop_cleanup;
	}

	// Wait for the service to stop.

	while (ssp.dwCurrentState != SERVICE_STOPPED)
	{
		Sleep(ssp.dwWaitHint);
		if (!QueryServiceStatusEx(
			schService,
			SC_STATUS_PROCESS_INFO,
			(LPBYTE)&ssp,
			sizeof(SERVICE_STATUS_PROCESS),
			&dwBytesNeeded))
		{
			printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
			goto stop_cleanup;
		}

		if (ssp.dwCurrentState == SERVICE_STOPPED)
			break;

		if (GetTickCount() - dwStartTime > dwTimeout)
		{
			printf("Wait timed out\n");
			goto stop_cleanup;
		}
	}
	printf("Service stopped successfully\n");

stop_cleanup:
	CloseServiceHandle(schService);
	CloseServiceHandle(manager);
}

BOOL __stdcall ServiceManager::StopDependentServices(SC_HANDLE schService)
{
	DWORD i;
	DWORD dwBytesNeeded;
	DWORD dwCount;

	LPENUM_SERVICE_STATUS   lpDependencies = NULL;
	ENUM_SERVICE_STATUS     ess;
	SC_HANDLE               hDepService;
	SERVICE_STATUS_PROCESS  ssp;

	DWORD dwStartTime = GetTickCount();
	DWORD dwTimeout = 30000; // 30-second time-out

	// Pass a zero-length buffer to get the required buffer size.
	if (EnumDependentServices(schService, SERVICE_ACTIVE,
		lpDependencies, 0, &dwBytesNeeded, &dwCount))
	{
		// If the Enum call succeeds, then there are no dependent
		// services, so do nothing.
		return TRUE;
	}
	else
	{
		if (GetLastError() != ERROR_MORE_DATA)
			return FALSE; // Unexpected error

		// Allocate a buffer for the dependencies.
		lpDependencies = (LPENUM_SERVICE_STATUS)HeapAlloc(
			GetProcessHeap(), HEAP_ZERO_MEMORY, dwBytesNeeded);

		if (!lpDependencies)
			return FALSE;

		__try {
			// Enumerate the dependencies.
			if (!EnumDependentServices(schService, SERVICE_ACTIVE,
				lpDependencies, dwBytesNeeded, &dwBytesNeeded,
				&dwCount))
				return FALSE;

			for (i = 0; i < dwCount; i++)
			{
				ess = *(lpDependencies + i);
				// Open the service.
				hDepService = OpenService(manager,
					ess.lpServiceName,
					SERVICE_STOP | SERVICE_QUERY_STATUS);

				if (!hDepService)
					return FALSE;

				__try {
					// Send a stop code.
					if (!ControlService(hDepService,
						SERVICE_CONTROL_STOP,
						(LPSERVICE_STATUS)&ssp))
						return FALSE;

					// Wait for the service to stop.
					while (ssp.dwCurrentState != SERVICE_STOPPED)
					{
						Sleep(ssp.dwWaitHint);
						if (!QueryServiceStatusEx(
							hDepService,
							SC_STATUS_PROCESS_INFO,
							(LPBYTE)&ssp,
							sizeof(SERVICE_STATUS_PROCESS),
							&dwBytesNeeded))
							return FALSE;

						if (ssp.dwCurrentState == SERVICE_STOPPED)
							break;

						if (GetTickCount() - dwStartTime > dwTimeout)
							return FALSE;
					}
				}
				__finally
				{
					// Always release the service handle.
					CloseServiceHandle(hDepService);
				}
			}
		}
		__finally
		{
			// Always free the enumeration buffer.
			HeapFree(GetProcessHeap(), 0, lpDependencies);
		}
	}
	return TRUE;
}
*/