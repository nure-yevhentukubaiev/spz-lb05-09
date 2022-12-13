#include "framework.h"
#include "testdrv.h"
#include "SCMan.h"


static LRESULT CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
SCMan scman;

INT APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpszCmd, INT nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInst);
	UNREFERENCED_PARAMETER(lpszCmd);
	UNREFERENCED_PARAMETER(nCmdShow);
	INT iRet = (INT)DialogBox(hInst, MAKEINTRESOURCE(IDD_FORMVIEW), HWND_DESKTOP, DlgProc);
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
			scman.AddDriver();
		}
			break;
		case IDC_REMOVE_DRIVER:
			scman.RemoveDriver();
			break;
		case IDC_START_DRIVER:
			scman.StartDriver();
			break;
		case IDC_STOP_DRIVER:
			scman.StopDriver();
			break;
		case IDC_OPEN_DEVICE:
			GetDlgItemText(hDlg, IDC_EDIT_SERVICENAME, scman.lpszSymlinkName, CCHBUF_MEDIUM);
			scman.OpenDevice();
			break;
		case IDC_CLOSE_DEVICE:
			scman.CloseDevice();
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
		}
		break;
	case WM_CLOSE:
		EndDialog(hDlg, 0);
		return TRUE;
		break;
	case WM_INITDIALOG:
		return TRUE;
		break;
	}
	return FALSE;
}