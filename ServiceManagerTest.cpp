// ServiceManagerTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "servicemanager.h"
#include <conio.h>
#include <unordered_map>
#include <iostream>
#include <string>
using namespace std;
int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	ServiceManager sm;
	if(!sm.Initialize())
	{
		cout << _T("Cannot initialize.") << endl;
		getch();
		return 1;
	}
	//sm.EnumerateAllServices();
	CServiceArray arr;
	sm.GetServices(arr);

	//enumerate
	cout << _T("Services Running:") << _T("-------------------") << endl;
	//cout << arr.GetSize() << endl;
	unordered_map < int, CString > mp;
	for(int i=0; i<arr.GetSize(); ++i)
	{
		CString n1 = arr[i].GetName();
		//CString n2 = arr[i].GetDisplayName();
		mp[i] = n1;
		//n1.AnsiToOem();
		//n2.AnsiToOem();

		//cout << i << ". " << n1 << _T(" (") << n2 << _T(")") << endl;
		//cout << _T("Status: ") << ((arr[i].GetStatus()==Active)?(_T("Active")):(_T("Inactive"))) << endl;
		//cout << _T("Process ID: ") << arr[i].GetProcessID() << endl;
		//cout << endl;

		//if(!(i%5) && i!=0)
		//	getch();
	}

	//start/stop
	//sm.StartService(_T("MySQL"));
	//sm.StopService(_T("MySQL"));
	while (true) {
		cout << "Enter : " << endl;
		cout << "1. To display the names of Services\n";
		cout << "2. To display the Service Names along with DisplayName, ProcessID and Status\n";
		cout << "3. Get the Path of a Service\n";
		cout << "4. To Start a Service\n";
		cout << "5. To Stop a Service\n";
		cout << "6. !!EXIT!!\n";
		int x; cin >> x;
		if (x == 1) {
			for (auto x : mp) {
				cout << x.first << " " << x.second << endl;
			}
		}
		else if (x == 2) {
			for (int i = 0; i < arr.GetSize(); ++i)
			{
				CString n1 = arr[i].GetName();
				CString n2 = arr[i].GetDisplayName();
				n1.AnsiToOem();
				n2.AnsiToOem();
				cout << i << ". " << n1 << _T(" (") << n2 << _T(")") << endl;
				cout << _T("Status: ") << ((arr[i].GetStatus() == Active) ? (_T("Active")) : (_T("Inactive"))) << endl;
				cout << _T("Process ID: ") << arr[i].GetProcessID() << endl;
				cout << _T("Path : ") << _T(sm.GetExePath(_T(mp[i]))) << endl;
				cout << endl;
			}
		}
		else if (x == 3) {
			cout << "Enter the Index of the Service to Get the Path for :\n";
			int x; cin >> x;
			cout << _T(sm.GetExePath(_T(mp[x]))) << endl;
		}
		else if (x == 4) {
			cout << "Enter the Index of the Service to Start\n";
			int x; cin >> x;
			sm.StartService(_T(mp[x]));
			//sm.DoStartSvc(_T(mp[x]));
		}
		else if (x == 5) {
			cout << "Enter the Index of the Service to Stop\n";
			int x; cin >> x;
			sm.StopService(_T(mp[x]));
			//sm.DoStopSvc(_T(mp[x]));
		}
		else {
			return 0;
		}
	}
	getch();
	return 0;
}
