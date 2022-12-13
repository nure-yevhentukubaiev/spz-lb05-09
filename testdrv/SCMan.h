#pragma once
#include "framework.h"

class SCMan {
private:
	SC_HANDLE hScMan;
	SC_HANDLE hSvc;
	HANDLE hDevice;
public:
	LPTSTR lpszDriverPath;
	LPTSTR lpszServiceName;
	LPTSTR lpszSymlinkName;

	SCMan(VOID);
	~SCMan(VOID);
	DWORD AddDriver(VOID);
	BOOL RemoveDriver(VOID) const;
	BOOL StartDriver(VOID) const;
	BOOL StopDriver(VOID) const;
	DWORD OpenDevice(VOID);
	BOOL CloseDevice(VOID) const;
};