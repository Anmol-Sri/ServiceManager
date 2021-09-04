#pragma once

#include <afxtempl.h>
#include <WinSvc.h>

enum Status
{
	Unknown = 0,
	Active = 1,
	Inactive = 2
};

struct Service
{
public: //interface
	inline Service() : pid(0), status(Unknown) {}
	inline ~Service() {}
	inline Service(Service& service) { *this = service; }
	Service& operator=(Service& service);
	inline CString GetName() { return name; }
	inline CString GetDisplayName() { return display_name; }
	inline int GetProcessID() { return pid; }
	inline Status GetStatus() { return status; }

protected: //data
	CString name;
	CString display_name;
	int pid;
	Status status;
	friend class ServiceManager;
};

#define CServiceArray CArray<Service,Service>

class ServiceManager
{
public: //interface
	inline ServiceManager(void) : manager(0) {}
	inline ~ServiceManager(void) { CleanUp(); }
	bool Initialize();
	void CleanUp();
	CString GetExePath(const TCHAR* name);
	bool GetServices(CServiceArray& services, int status_mask = Active|Inactive);
	bool StartService(const TCHAR* name);
	bool StopService(const TCHAR* name);
	//void __stdcall DoStartSvc(const TCHAR* name);
	// Incase of Error with StartService Uncomment the Above line and also the DoStartSvc function from ServiceManager.cpp
	//void __stdcall DoStopSvc(const TCHAR* name);
	//BOOL __stdcall StopDependentServices(SC_HANDLE schService);
	// Incase of Error with StopService Uncomment the Above line and also the DoStopSvc function from ServiceManager.cpp
	//int EnumerateAllServices();
protected: //data
	SC_HANDLE manager;

	friend struct Service;
};
