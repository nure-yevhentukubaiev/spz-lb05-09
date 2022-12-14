#include "framework.h"
#include "testdrv.h"
#include "SCMan.h"


static LRESULT CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL AppendLogEntry(LPCTSTR lpszLogEntry);
static BOOL PrintLastError(VOID);

SCMan scman;
LPVOID lpBuf;
HWND g_hDlg;
HFONT hFontLogView;

static BOOL AppendLogEntry(LPCTSTR lpszLogEntry)
{
	LPTSTR lpszLog = NULL;
	HWND hLogView = GetDlgItem(g_hDlg, IDC_LOGVIEW);
	INT iLogLength = GetWindowTextLength(hLogView) + 1;

	lpszLog = (LPTSTR)LocalAlloc(
		LMEM_ZEROINIT,
		sizeof(LPTSTR) * (iLogLength + lstrlen(lpszLogEntry))
	);
	GetWindowText(hLogView, lpszLog, iLogLength);
	lstrcat(lpszLog, lpszLogEntry);
	SetWindowText(hLogView, lpszLog);

	LocalFree(lpszLog);
	return TRUE;
}


static BOOL PrintLastError(VOID)
{
	DWORD dwErr = GetLastError();
	
	LPTSTR lpszErrorDesc = NULL;
	
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER
		| FORMAT_MESSAGE_FROM_SYSTEM
		| FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, dwErr,
		0, (LPTSTR)&lpszErrorDesc, 0,
		NULL
	);
	
	AppendLogEntry(lpszErrorDesc);
	
	LocalFree(lpszErrorDesc);
	return TRUE;
}

INT APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpszCmd, INT nCmdShow)
{
	INT iRet;

	UNREFERENCED_PARAMETER(hPrevInst);
	UNREFERENCED_PARAMETER(lpszCmd);
	UNREFERENCED_PARAMETER(nCmdShow);

	iRet = (INT)DialogBox(hInst, MAKEINTRESOURCE(IDD_FORMVIEW), HWND_DESKTOP, DlgProc);
	return iRet;
}

LRESULT CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_ADD_DRIVER:
		{
			GetDlgItemText(hDlg, IDC_EDIT_SERVICENAME, scman.lpszServiceName, CCHBUF_MEDIUM);
			GetDlgItemText(hDlg, IDC_EDIT_FILEPATH, scman.lpszDriverPath, CCHBUF_BIG);
			AppendLogEntry(_T("AddDriver: "));
			scman.AddDriver();
			PrintLastError();
		}
			break;
		case IDC_REMOVE_DRIVER:
			AppendLogEntry(_T("RemoveDriver: "));
			scman.RemoveDriver();
			PrintLastError();
			break;
		case IDC_START_DRIVER:
			AppendLogEntry(_T("StartDriver: "));
			scman.StartDriver();
			PrintLastError();
			break;
		case IDC_STOP_DRIVER:
			AppendLogEntry(_T("StopDriver: "));
			scman.StopDriver();
			PrintLastError();
			break;
		case IDC_OPEN_DEVICE:
			GetDlgItemText(hDlg, IDC_EDIT_SERVICENAME, scman.lpszSymlinkName, CCHBUF_MEDIUM);
			AppendLogEntry(_T("OpenDevice: "));
			scman.OpenDevice();
			PrintLastError();
			break;
		case IDC_CLOSE_DEVICE:
			AppendLogEntry(_T("CloseDevice: "));
			scman.CloseDevice();
			PrintLastError();
			break;
		case IDC_FILEPICKER:
		{
			OPENFILENAME ofn;
			SecureZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hDlg;
			ofn.lpstrTitle = _T("Create file...");
			ofn.lpstrFile = scman.lpszDriverPath;
			ofn.nMaxFile = CCHBUF_BIG;
			ofn.Flags = 
				OFN_FILEMUSTEXIST
				| OFN_READONLY
				| OFN_PATHMUSTEXIST;
			ofn.lpstrFilter = _T("All Files\0*.*\0Driver files (.sys)\0.SYS\0\0");

			if (GetOpenFileName(&ofn))
				SetDlgItemText(hDlg, IDC_EDIT_FILEPATH, ofn.lpstrFile);
		}
		case IDC_READ:
		{
			DWORD dwActualSize = 0;
			AppendLogEntry(_T("Read: "));
			if (scman.Read(lpBuf, 8, &dwActualSize)) {
				LPTSTR lpszLogEntry =
					(LPTSTR)LocalAlloc(LMEM_ZEROINIT, sizeof(LPTSTR) * CCHBUF_SMALL);
				LPSTR lpBufCpy = NULL;
				(LPSTR)CopyMemory(lpBufCpy, lpBuf, dwActualSize+1);

				_stprintf_s(
					lpszLogEntry, CCHBUF_SMALL,
					_T("%lX bytes read: %hs\r\n"),
					lstrlenA(lpBufCpy), lpBufCpy
				);
				AppendLogEntry(lpszLogEntry);
				LocalFree(lpszLogEntry);
			} else {
				PrintLastError();
			}
		}
			break;
		case IDC_WRITE:
			AppendLogEntry(_T("Write: "));
			//scman.Write(lpBuf, 16);
			break;
		}
		break;
	case WM_CLOSE:
		DeleteObject(hFontLogView);
		EndDialog(hDlg, 0);
		return TRUE;
		break;
	case WM_INITDIALOG:

		g_hDlg = hDlg;
		hFontLogView = CreateFont(
			14, NULL, NULL, NULL, NULL,
			FALSE, FALSE, FALSE,
			DEFAULT_CHARSET, NULL, NULL, DEFAULT_QUALITY, FF_DONTCARE,
			_T("Courier New")
		);
		SendDlgItemMessage(hDlg, IDC_LOGVIEW, WM_SETFONT, (WPARAM)hFontLogView, TRUE);
		AppendLogEntry(_T("Begin of Log View"));
		return TRUE;
		break;
	}
	return FALSE;
}