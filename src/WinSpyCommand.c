//
//	WinSpyCommand.c
//
//  Copyright (c) 2002 by J Brown
//  Freeware
//
//	Menu / Control Command handler
//

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>

#include "resource.h"
#include "WinSpy.h"
#include "Utils.h"
#include "FindTool.h"
#include "CaptureWindow.h"

HTREEITEM FindTreeItemByHwnd(HWND hwndTree, HWND hwndTarget, HTREEITEM hItem);

static void ShowHelp(HWND hwndMain, UINT uCommand, DWORD dwData)
{
	WCHAR szPath[MAX_PATH];
	WCHAR *ptr;

	// Assume help file is in same directory as the
	// main executable. So, just replace WinSpy.exe with WinSpy.hlp
	// to find the name!

	GetModuleFileNameW(0, szPath, MAX_PATH);

	ptr = szPath + lstrlenW(szPath) - 1;

	while(ptr > szPath && *ptr != L'\\')
		ptr--;

	lstrcpyW(ptr, L"\\WinSpy.hlp");

	WinHelpW(hwndMain, szPath, uCommand, dwData);
}

void SetPinState(BOOL fPinned)
{
	fPinWindow = fPinned;

	SendMessageW(hwndPin, TB_CHANGEBITMAP, IDM_WINSPY_PIN,
		MAKELPARAM(fPinWindow, 0));

	SendMessageW(hwndPin, TB_CHECKBUTTON, IDM_WINSPY_PIN,
			MAKELPARAM(fPinWindow, 0));

}

UINT WinSpyDlg_CommandHandler(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	HTREEITEM   hItem;
	TVITEMW     item;
	NMHDR		hdr;
	UINT        uLayout;

	HWND hwndGeneral;
	HWND hwndFocus;
	HWND hwndCtrl;
	HWND hwndTree;

	switch(LOWORD(wParam))
	{
	case IDM_WINSPY_ONTOP:

		if(GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_TOPMOST)
		{
			// Not top-most any more
			SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_ZONLY);
		}
		else
		{
			// Make top-most (float above all other windows)
			SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_ZONLY);
		}

		return TRUE;

	case IDM_WINSPY_TOGGLE:
		ToggleWindowLayout(hwnd);
		return TRUE;

	case IDM_WINSPY_TOGGLEEXP:

		uLayout = GetWindowLayout(hwnd);

		if(uLayout == WINSPY_EXPANDED)
			SetWindowLayout(hwnd, WINSPY_MINIMIZED);

		else if(uLayout == WINSPY_NORMAL)
			SetWindowLayout(hwnd, WINSPY_EXPANDED);

		else if(uLayout == WINSPY_MINIMIZED)
			SetWindowLayout(hwnd, WINSPY_NORMAL);

		return TRUE;

	case IDM_WINSPY_ZOOMTL:
		WinSpy_ZoomTo(hwnd, PINNED_TOPLEFT);
		return TRUE;

	case IDM_WINSPY_ZOOMTR:
		WinSpy_ZoomTo(hwnd, PINNED_TOPRIGHT);
		return TRUE;

	case IDM_WINSPY_ZOOMBR:
		WinSpy_ZoomTo(hwnd, PINNED_BOTTOMRIGHT);
		return TRUE;

	case IDM_WINSPY_ZOOMBL:
		WinSpy_ZoomTo(hwnd, PINNED_BOTTOMLEFT);
		return TRUE;

	case IDM_WINSPY_REFRESH:
		DisplayWindowInfo(spy_hCurWnd);
		return TRUE;

	case IDM_WINSPY_HELP:
		ShowHelp(hwnd, HELP_CONTENTS, 0);
		return TRUE;

	case IDM_WINSPY_OPTIONS:
		ShowOptionsDlg(hwnd);
		return TRUE;

	case IDM_WINSPY_PIN:

		fPinWindow = !fPinWindow;

		SendMessageW(hwndPin, TB_CHANGEBITMAP, IDM_WINSPY_PIN,
			MAKELPARAM(fPinWindow, 0));

		// if from an accelerator, then we have to manually check the
		if(HIWORD(wParam) == 1)
		{
			SendMessageW(hwndPin, TB_CHECKBUTTON, IDM_WINSPY_PIN,
				MAKELPARAM(fPinWindow, 0));
		}

		GetPinnedPosition(hwnd, &ptPinPos);
		return TRUE;

	case IDC_HIDDEN:
		fShowHidden = IsDlgButtonChecked(hwnd, IDC_HIDDEN);
		return TRUE;

	case IDC_MINIMIZE:
		fMinimizeWinSpy = IsDlgButtonChecked(hwnd, IDC_MINIMIZE);
		return TRUE;

	case IDM_WINSPY_GENERAL:
	case IDM_WINSPY_STYLES:
	case IDM_WINSPY_PROPERTIES:
	case IDM_WINSPY_CLASS:
	case IDM_WINSPY_WINDOWS:
	case IDM_WINSPY_SCROLLBARS:

		// Simulate the tab-control being clicked
		hdr.hwndFrom = GetDlgItem(hwnd, IDC_TAB1);
		hdr.idFrom   = IDC_TAB1;
		hdr.code     = TCN_SELCHANGE;

		SendMessageW(hdr.hwndFrom, TCM_SETCURSEL, LOWORD(wParam) - IDM_WINSPY_GENERAL, 0);

		SendMessageW(hwnd, WM_NOTIFY, 0, (LPARAM)&hdr);

		return TRUE;

	case IDC_FLASH:

		hwndTree = GetDlgItem(hwnd, IDC_TREE1);

		hItem = (HTREEITEM)SendMessageW(hwnd, TVM_GETNEXTITEM, TVGN_CARET, 0);

		item.mask  = TVIF_PARAM | TVIF_HANDLE;
		item.hItem = hItem;

		SendMessageW(hwndTree, TVM_GETITEMW, 0, (LPARAM)&item);

		FlashWindowBorder((HWND)item.lParam, TRUE);

		return TRUE;

	case IDC_EXPAND:

		if(GetWindowLayout(hwnd) == WINSPY_NORMAL)
		{
			RefreshTreeView(GetDlgItem(hwnd, IDC_TREE1));
			SetWindowLayout(hwnd, WINSPY_EXPANDED);

		}
		else
		{
			SetWindowLayout(hwnd, WINSPY_NORMAL);
		}

		return TRUE;

	case IDC_CAPTURE:
		CaptureWindow(hwnd, spy_hCurWnd);
		MessageBoxW(hwnd, L"Window contents captured to clipboard", szAppName, MB_ICONINFORMATION);
		return TRUE;

	case IDOK:

		hwndGeneral = WinSpyTab[GENERAL_TAB].hwnd;
		hwndFocus =  GetFocus();

		if(hwndFocus == GetDlgItem(hwndGeneral, IDC_HANDLE))
		{
			hwndCtrl = (HWND)GetDlgItemBaseInt(hwndGeneral, IDC_HANDLE, 16);

			if(IsWindow(hwndCtrl))
			{
				spy_hCurWnd = hwndCtrl;
				DisplayWindowInfo(spy_hCurWnd);
			}

			return 0;
		}
		else if(hwndFocus == GetDlgItem(hwndGeneral, IDC_CAPTION1) ||
			hwndFocus == GetWindow(GetDlgItem(hwndGeneral, IDC_CAPTION2), GW_CHILD))
		{
			PostMessageW(hwndGeneral, WM_COMMAND, MAKEWPARAM(IDC_SETCAPTION, BN_CLICKED), 0);
			return FALSE;
		}


		if(GetFocus() != (HWND)lParam)
			return FALSE;

		ExitWinSpy(hwnd, 0);

		return TRUE;

	case IDC_LOCATE:

		hwndTree = GetDlgItem(hwnd, IDC_TREE1);

		// Find treeview item that contains our window handle
		hItem = FindTreeItemByHwnd(hwndTree, spy_hCurWnd, NULL);

		// Move it into view!
		SendMessageW(hwndTree, TVM_ENSUREVISIBLE, 0, (LPARAM)hItem);
		SendMessageW(hwndTree, TVM_SELECTITEM, TVGN_CARET, (LPARAM)hItem);
		SetFocus(hwndTree);

		return TRUE;

	case IDC_REFRESH:
		RefreshTreeView(GetDlgItem(hwnd, IDC_TREE1));
		return TRUE;
	}

	return FALSE;
}

UINT WinSpyDlg_SysMenuHandler(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	WCHAR szText[200];
	WCHAR szTitle[60];
	WCHAR szVersion[40];
	WCHAR szCurExe[MAX_PATH];

	switch(wParam & 0xFFF0)
	{
	case SC_RESTORE:

		if(IsMinimized(hwnd))
		{
			break;
		}
		else
		{
			ToggleWindowLayout(hwnd);
			return 1;
		}

	case SC_MAXIMIZE:
		ToggleWindowLayout(hwnd);
		return 1;
	}

	switch(wParam)
	{
	case IDM_WINSPY_HELP:
		ShowHelp(hwnd, HELP_CONTENTS, 0);
		return TRUE;

	case IDM_WINSPY_ABOUT:

		GetModuleFileNameW(0, szCurExe, MAX_PATH);
		GetVersionString(szCurExe, L"FileVersion", szVersion, 40);

        wsprintfW(szText, L"%s v%s\n\r\n\rCopyright(c) 2002-2012 by Catch22 Productions.\n\rWritten by J Brown.\n\r\n\rHomepage at www.catch22.net", szAppName, szVersion);
		wsprintfW(szTitle, L"About %s", szAppName);

		MessageBoxW(hwnd, szText, szTitle, MB_OK|MB_ICONINFORMATION);
		return TRUE;

	case IDM_WINSPY_OPTIONS:
		ShowOptionsDlg(hwnd);
		return TRUE;

	case IDM_WINSPY_ONTOP:

		PostMessageW(hwnd, WM_COMMAND, wParam, lParam);
		return TRUE;

	}
	return FALSE;

}
