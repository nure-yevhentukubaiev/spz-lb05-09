#include "SCMan.h"
#include "framework.h"
#include "testdrv.h"

SCMan::SCMan(VOID)
{
	this->lpszServiceName =
		(LPTSTR)LocalAlloc(LMEM_ZEROINIT, sizeof(LPTSTR) * CCHBUF_MEDIUM);
	this->lpszSymlinkName =
		(LPTSTR)LocalAlloc(LMEM_ZEROINIT, sizeof(LPTSTR) * CCHBUF_BIG);
	this->lpszDriverPath =
		(LPTSTR)LocalAlloc(LMEM_ZEROINIT, sizeof(LPTSTR) * CCHBUF_BIG);
	this->hScMan = OpenSCManager(NULL, SERVICES_ACTIVE_DATABASE, 0);
}

SCMan::~SCMan(VOID)
{
	CloseHandle(this->hDevice);
	CloseServiceHandle(this->hSvc);
	CloseServiceHandle(this->hScMan);
	LocalFree(this->lpszServiceName);
	LocalFree(this->lpszSymlinkName);
	LocalFree(this->lpszDriverPath);
}

DWORD SCMan::AddDriver(VOID)
{
	this->hSvc = CreateService(
		this->hScMan,
		this->lpszServiceName, this->lpszServiceName,
		0, 0, SERVICE_DEMAND_START, 0,
		this->lpszDriverPath,
		NULL, 0,
		NULL,
		NULL, NULL
	);
	return GetLastError();
}

BOOL SCMan::RemoveDriver(VOID) const
{
	return DeleteService(this->hSvc);
}

BOOL SCMan::StartDriver(VOID) const
{
	return StartService(this->hSvc, 0, NULL);
}

BOOL SCMan::StopDriver(VOID) const
{
	return ControlService(
		this->hSvc,
		SERVICE_CONTROL_STOP,
		NULL
	);
}

DWORD SCMan::OpenDevice(VOID)
{
	this->hDevice = CreateFile(
		this->lpszSymlinkName,
		0, 0, 0, CREATE_NEW,
		0,
		NULL
	);
	return GetLastError();
}

BOOL SCMan::CloseDevice(VOID) const
{
	return CloseHandle(this->hDevice);
}

BOOL SCMan::Read(LPVOID lpBuf, ULONG ulSize, PULONG pulActualSize) const
{
	return ReadFile(
		this->hDevice, lpBuf,
		ulSize, pulActualSize,
		NULL
	);
}

BOOL SCMan::Write(LPVOID lpBuf, ULONG ulSize, PULONG pulActualSize) const
{
	return WriteFile(
		this->hDevice, lpBuf,
		ulSize, pulActualSize,
		NULL
	);
}