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
	BOOL TryOpenService(VOID);
	BOOL RemoveDriver(VOID);
	BOOL StartDriver(VOID);
	BOOL StopDriver(VOID);
	DWORD OpenDevice(VOID);
	BOOL CloseDevice(VOID) const;
	BOOL Read(LPVOID lpBuf, ULONG ulSize, PULONG pulActualSize) const;
	BOOL Write(LPVOID lpBuf, ULONG ulSize, PULONG pulActualSize) const;
};