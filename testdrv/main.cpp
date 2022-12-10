#include "framework.h"
#include "testdrv.h"

static LRESULT CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

INT APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpszCmd, INT nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInst);
	UNREFERENCED_PARAMETER(lpszCmd);
	UNREFERENCED_PARAMETER(nCmdShow);
	return DialogBox(hInst, MAKEINTRESOURCE(IDD_FORMVIEW), HWND_DESKTOP, DlgProc);
}

LRESULT CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_ADD_DRIVER:
			break;
		case IDC_REMOVE_DRIVER:
			break;
		case IDC_START_DRIVER:
			break;
		case IDC_STOP_DRIVER:
			break;
		case IDC_OPEN_DEVICE:
			break;
		case IDC_CLOSE_DEVICE:
			break;
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