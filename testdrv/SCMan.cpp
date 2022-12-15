#include "SCMan.h"
#include "framework.h"
#include "testdrv.h"

SCMan::SCMan(VOID)
{
	this->lpszServiceName =
		(LPTSTR)LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR) * CCHBUF_MEDIUM);
	this->lpszSymlinkName =
		(LPTSTR)LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR) * CCHBUF_BIG);
	this->lpszDriverPath =
		(LPTSTR)LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR) * CCHBUF_BIG);
	this->hScMan = OpenSCManager(
		NULL,
		SERVICES_ACTIVE_DATABASE,
		SC_MANAGER_ALL_ACCESS
	);
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
		SERVICE_ALL_ACCESS,
		SERVICE_KERNEL_DRIVER, SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL,
		this->lpszDriverPath,
		NULL, 0,
		NULL,
		NULL, NULL
	);
	return GetLastError();
}

BOOL SCMan::TryOpenService(VOID)
{
	SetLastError(ERROR_SUCCESS);
	this->hSvc = (this->hSvc)
		? this->hSvc
		: OpenService(
			this->hScMan,
			this->lpszServiceName,
			SERVICE_ALL_ACCESS
		);
	if (GetLastError()) {
		return FALSE;
	}
	return TRUE;
}

BOOL SCMan::RemoveDriver(VOID)
{
	if (!this->TryOpenService())
		return FALSE;
	DeleteService(this->hSvc);
	CloseServiceHandle(this->hSvc);
	this->hSvc = NULL;
	return TRUE;
}

BOOL SCMan::StartDriver(VOID)
{
	if (!this->TryOpenService())
		return FALSE;
	return StartService(this->hSvc, 0, NULL);
}

BOOL SCMan::StopDriver(VOID)
{
	if (!this->TryOpenService())
		return FALSE;
	SERVICE_STATUS svcstat;
	BOOL bRet = ControlService(
		this->hSvc,
		SERVICE_CONTROL_STOP,
		&svcstat
	);

	return bRet;
}

DWORD SCMan::OpenDevice(VOID)
{
	this->hDevice = CreateFile(
		this->lpszSymlinkName,
		GENERIC_ALL,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		NULL, OPEN_EXISTING,
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