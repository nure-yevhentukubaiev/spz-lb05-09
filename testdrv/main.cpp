#include "framework.h"
#include "testdrv.h"
#include "SCMan.h"

#define TESTDRV_BYTES_TO_READ 8U
#define TESTDRV_BYTES_TO_WRITE 16U
#define SHOW_BYTES 4U

static LRESULT CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL AppendLogEntry(LPCTSTR lpszLogEntry);
static BOOL PrintLastError(VOID);

SCMan scman;
HWND g_hDlg;
HFONT hFontLogView;

static BOOL AppendLogEntry(LPCTSTR lpszLogEntry)
{
	LPTSTR lpszLog = NULL;
	HWND hLogView = GetDlgItem(g_hDlg, IDC_LOGVIEW);
	INT iLogLength = GetWindowTextLength(hLogView) + 1;

	lpszLog = (LPTSTR)LocalAlloc(
		LMEM_ZEROINIT,
		sizeof(TCHAR) * (iLogLength + lstrlen(lpszLogEntry))
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
	
	if (dwErr) {
		lpszErrorDesc = (LPTSTR)LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR) * CCHBUF_SMALL);
		_stprintf_s(
			lpszErrorDesc, CCHBUF_SMALL,
			_T("ERROR %X: "), dwErr
		);
		AppendLogEntry(lpszErrorDesc);
		LocalFree(lpszErrorDesc);
	}

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
			GetDlgItemText(hDlg, IDC_EDIT_SERVICENAME, scman.lpszServiceName, CCHBUF_MEDIUM);
			GetDlgItemText(hDlg, IDC_EDIT_FILEPATH, scman.lpszDriverPath, CCHBUF_BIG);
			AppendLogEntry(_T("RemoveDriver: "));
			scman.RemoveDriver();
			PrintLastError();
			break;
		case IDC_START_DRIVER:
			GetDlgItemText(hDlg, IDC_EDIT_SERVICENAME, scman.lpszServiceName, CCHBUF_MEDIUM);
			AppendLogEntry(_T("StartDriver: "));
			scman.StartDriver();
			PrintLastError();
			break;
		case IDC_STOP_DRIVER:
			GetDlgItemText(hDlg, IDC_EDIT_SERVICENAME, scman.lpszServiceName, CCHBUF_MEDIUM);
			AppendLogEntry(_T("StopDriver: "));
			scman.StopDriver();
			PrintLastError();
			break;
		case IDC_OPEN_DEVICE:
			GetDlgItemText(hDlg, IDC_EDIT_SYMBOLICLINKNAME, scman.lpszSymlinkName, CCHBUF_MEDIUM);
			AppendLogEntry(_T("OpenDevice: "));
			scman.OpenDevice();
			PrintLastError();
			break;
		case IDC_CLOSE_DEVICE:
			GetDlgItemText(hDlg, IDC_EDIT_SYMBOLICLINKNAME, scman.lpszSymlinkName, CCHBUF_MEDIUM);
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
			ofn.lpstrFilter = _T("All Files\0*.*\0Driver files (.sys)\0*.SYS\0\0");

			if (GetOpenFileName(&ofn))
				SetDlgItemText(hDlg, IDC_EDIT_FILEPATH, ofn.lpstrFile);
		}
		break;
		case IDC_READ:
		{
			DWORD dwActualSize = 0;
			LPSTR lpBuf =
				(LPSTR)LocalAlloc(LMEM_ZEROINIT, sizeof(CHAR) * (TESTDRV_BYTES_TO_READ + 1));
			AppendLogEntry(_T("Read: "));
			if (!scman.Read(lpBuf, TESTDRV_BYTES_TO_READ, &dwActualSize)) {
				PrintLastError();
				break;
			}
			LPTSTR lpszLogEntry =
				(LPTSTR)LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR) * CCHBUF_SMALL);
			
			DWORD dwBytesToShow = min(dwActualSize, SHOW_BYTES);
			lpBuf[dwBytesToShow] = '\0';
			_stprintf_s(
				lpszLogEntry, CCHBUF_SMALL,
				_T("%d bytes read, first %d bytes: %hs\r\n"),
				dwActualSize, dwBytesToShow, lpBuf
			);
			AppendLogEntry(lpszLogEntry);
			LocalFree(lpszLogEntry);
			LocalFree(lpBuf);
		}
		break;
		case IDC_WRITE:
		{
			DWORD dwActualSize = 0;
			LPSTR lpBuf =
				(LPSTR)LocalAlloc(LMEM_ZEROINIT, sizeof(CHAR) * (TESTDRV_BYTES_TO_WRITE + 1));
			lstrcpyA(lpBuf, "0123456789ABCDEF");
			AppendLogEntry(_T("Write: "));
			if (!scman.Write(lpBuf, TESTDRV_BYTES_TO_WRITE, &dwActualSize)) {
				PrintLastError();
				LocalFree(lpBuf);
				break;
			}
			LPTSTR lpszLogEntry =
				(LPTSTR)LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR) * CCHBUF_SMALL);

			DWORD dwBytesToShow = min(dwActualSize, SHOW_BYTES);
			lpBuf[dwBytesToShow] = '\0';
			_stprintf_s(
				lpszLogEntry, CCHBUF_SMALL,
				_T("%d bytes read, first %d bytes: %hs\r\n"),
				dwActualSize, dwBytesToShow, lpBuf
			);
			AppendLogEntry(lpszLogEntry);
			LocalFree(lpszLogEntry);
			LocalFree(lpBuf);
		}
			break;
		case IDC_CTL_CODE:
		{
			DWORD dwActualSize = 0;
			LPSTR lpInBuf =
				(LPSTR)LocalAlloc(LMEM_ZEROINIT, sizeof(CHAR) * (TESTDRV_BYTES_TO_READ));
			LPSTR lpOutBuf =
				(LPSTR)LocalAlloc(LMEM_ZEROINIT, sizeof(CHAR) * (TESTDRV_BYTES_TO_WRITE));
			LPTSTR lpszLogEntry =
				(LPTSTR)LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR) * CCHBUF_SMALL);
			lstrcpyA(lpInBuf, "FEDCBA9");
			AppendLogEntry(_T("SentCtlCode: "));
			for (
				DWORD dwCtlCode = DRV05_BASE_CTL_FUNC;
				dwCtlCode < DRV05_BASE_CTL_FUNC + 8;
				dwCtlCode++
			) {
				if (scman.SendCtlCode(
					dwCtlCode,
					lpInBuf, TESTDRV_BYTES_TO_READ,
					lpOutBuf, TESTDRV_BYTES_TO_WRITE,
					&dwActualSize
				)) {
					PrintLastError();
					break;
				}

				DWORD dwBytesToShow = min(dwActualSize, SHOW_BYTES);
				lpOutBuf[dwBytesToShow] = '\0';
				_stprintf_s(
					lpszLogEntry, CCHBUF_SMALL,
					_T("%d bytes handled, first %d bytes: %hs\r\n"),
					dwActualSize, dwBytesToShow, lpOutBuf
				);
				AppendLogEntry(lpszLogEntry);
			}
			LocalFree(lpszLogEntry);
			LocalFree(lpInBuf);
			LocalFree(lpOutBuf);
		}
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
		AppendLogEntry(_T("Beginning of Log View\r\n"));
		return TRUE;
		break;
	}
	return FALSE;
}