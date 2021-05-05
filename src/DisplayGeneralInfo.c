//
//	DisplayGeneralInfo.c
//  Copyright (c) 2002 by J Brown
//	Freeware
//
//	void SetGeneralInfo(HWND hwnd)
//
//	Fill the general-tab-pane with general info for the
//  specified window
//

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>

#include "resource.h"
#include "WinSpy.h"
#include "Utils.h"

void RemoveHyperlink(HWND hwnd, UINT staticid);
void MakeHyperlink(HWND hwnd, UINT staticid, COLORREF crLink);


void SetGeneralInfo(HWND hwnd)
{
	WCHAR	ach[256];
	HWND	hwndDlg = WinSpyTab[GENERAL_TAB].hwnd;
	RECT	rect;
	int		x1, y1;
	int		i, numbytes;

	if(hwnd == 0) return;

	//handle
	wsprintfW(ach, szHexFmt, hwnd);
	SetDlgItemTextW(hwndDlg, IDC_HANDLE, ach);

	//caption
	ShowDlgItem(hwndDlg, IDC_CAPTION1, SW_SHOW);
	ShowDlgItem(hwndDlg, IDC_CAPTION2, SW_HIDE);

	SendDlgItemMessageW(hwndDlg, IDC_CAPTION2, CB_RESETCONTENT, 0, 0);

	// SendMessage is better than GetWindowText,
	// because it gets text of children in other processes
	if(spy_fPassword == FALSE)
	{
		DWORD_PTR dwResult;

		ach[0] = 0;

		SendMessageTimeoutW(hwnd, WM_GETTEXT, sizeof(ach) / sizeof(WCHAR), (LPARAM)ach,
			SMTO_ABORTIFHUNG, 100, &dwResult);

		SetDlgItemTextW(hwndDlg, IDC_CAPTION1, ach);	// edit box
		SetDlgItemTextW(hwndDlg, IDC_CAPTION2, ach);	// combo box
	}
	else
	{
		SetDlgItemTextW(hwndDlg, IDC_CAPTION1, spy_szPassword);	// edit box
		SetDlgItemTextW(hwndDlg, IDC_CAPTION2, spy_szPassword);	// combo box
	}

	//class name
	GetClassNameW(hwnd, ach, sizeof ach);

	if(IsWindowUnicode(hwnd))	lstrcatW(ach, L"  (Unicode)");

	SetDlgItemTextW(hwndDlg, IDC_CLASS, ach);

	//style
	wsprintfW(ach, szHexFmt, GetWindowLong(hwnd, GWL_STYLE));

	if(IsWindowVisible(hwnd))	lstrcatW(ach, L"  (visible, ");
	else						lstrcatW(ach, L"  (hidden, ");

	if(IsWindowEnabled(hwnd))	lstrcatW(ach, L"enabled)");
	else						lstrcatW(ach, L"disabled)");

	SetDlgItemTextW(hwndDlg, IDC_STYLE, ach);

	//rectangle
	GetWindowRect(hwnd, &rect);
	x1 = rect.left;
	y1 = rect.top;

	wsprintfW(ach, L"(%d,%d) - (%d,%d)  -  %dx%d",
		rect.left,rect.top, rect.right,rect.bottom,
		(rect.right-rect.left), (rect.bottom-rect.top));

	SetDlgItemTextW(hwndDlg, IDC_RECTANGLE, ach);

	//client rect
	GetClientRect(hwnd, &rect);
	MapWindowPoints(hwnd, 0, (POINT *)&rect, 2);
	x1 = rect.left-x1;
	y1 = rect.top-y1;

	OffsetRect(&rect, -rect.left, -rect.top);
	OffsetRect(&rect, x1, y1);

	wsprintfW(ach, L"(%d,%d) - (%d,%d)  -  %dx%d",
		rect.left,rect.top, rect.right,rect.bottom,
		(rect.right-rect.left), (rect.bottom-rect.top));

	SetDlgItemTextW(hwndDlg, IDC_CLIENTRECT, ach);

	//restored rect
	/*GetWindowPlacement(hwnd, &wp);
	wsprintfW(ach, L"(%d,%d) - (%d,%d)  -  %dx%d",
		wp.rcNormalPosition.left, wp.rcNormalPosition.top,
		wp.rcNormalPosition.right, wp.rcNormalPosition.bottom,
		(wp.rcNormalPosition.right-wp.rcNormalPosition.left),
		(wp.rcNormalPosition.bottom-wp.rcNormalPosition.top));

	SetDlgItemTextW(hwndDlg, IDC_RESTOREDRECT, ach);*/

	//window procedure
	if(spy_WndProc == 0)
	{
		wsprintfW(ach, L"N/A");
		SetDlgItemTextW(hwndDlg, IDC_WINDOWPROC, ach);

		ShowDlgItem(hwndDlg, IDC_WINDOWPROC,  SW_SHOW);
		ShowDlgItem(hwndDlg, IDC_WINDOWPROC2, SW_HIDE);
	}
	else
	{
		wsprintfW(ach, szHexFmt, spy_WndProc);
		SetDlgItemTextW(hwndDlg, IDC_WINDOWPROC2, ach);

		ShowDlgItem(hwndDlg, IDC_WINDOWPROC,  SW_HIDE);
		ShowDlgItem(hwndDlg, IDC_WINDOWPROC2, SW_SHOW);
	}

	//instance handle
	wsprintfW(ach, szHexFmt, GetWindowLongPtrW(hwnd, GWLP_HINSTANCE));
	SetDlgItemTextW(hwndDlg, IDC_INSTANCE, ach);

	//user data
	wsprintfW(ach, szHexFmt, GetWindowLongPtrW(hwnd, GWLP_USERDATA));
	SetDlgItemTextW(hwndDlg, IDC_USERDATA, ach);

	//control ID
	wsprintfW(ach, szHexFmt, GetWindowLong(hwnd, GWL_ID));
	SetDlgItemTextW(hwndDlg, IDC_CONTROLID, ach);

	//extra window bytes
	numbytes = GetClassLongW(hwnd, GCL_CBWNDEXTRA);
	i = 0;

	SendDlgItemMessageW(hwndDlg, IDC_WINDOWBYTES, CB_RESETCONTENT, 0, 0);
	EnableDlgItem(hwndDlg, IDC_WINDOWBYTES, numbytes != 0);

	// Retrieve all the window bytes + add to combo box
	while(numbytes != 0)
	{
		if(numbytes >= 4)
			wsprintfW(ach, L"+%-8d  %08X", i, GetWindowLong(hwnd, i));
		else
			wsprintfW(ach, L"+%-8d  %s", i, L"(Unavailable)");

		i += 4;
		numbytes = max(numbytes - 4, 0);

		SendDlgItemMessageW(hwndDlg, IDC_WINDOWBYTES, CB_ADDSTRING, 0, (LPARAM)ach);
	}

	SendDlgItemMessageW(hwndDlg, IDC_WINDOWBYTES, CB_SETCURSEL, 0, 0);
}
