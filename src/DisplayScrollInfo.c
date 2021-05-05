//
//	DisplayScrollInfo.c
//  Copyright (c) 2002 by J Brown
//	Freeware
//
//	void SetScrollInfo(HWND hwnd)
//
//	Fill the scrollbar-tab-pane with scrollbar info for the
//  specified window
//

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>

#include "resource.h"
#include "WinSpy.h"

void SetScrollbarInfo(HWND hwnd)
{
	SCROLLINFO si;
	DWORD  dwStyle;
	int    bartype;
	WCHAR  ach[256];
	HWND   hwndDlg = WinSpyTab[PROPERTY_TAB].hwnd;

	if(hwnd == 0) return;

	si.cbSize = sizeof(SCROLLINFO);
	si.fMask  = SIF_ALL;

	dwStyle = GetWindowLong(hwnd, GWL_STYLE);

	bartype = SB_HORZ;

	GetClassNameW(hwnd, ach, sizeof(ach) / sizeof(WCHAR));

	if(lstrcmpiW(ach, L"ScrollBar") == 0)
	{
		if((dwStyle & SBS_VERT) == 0)
			bartype = SB_CTL;

		SetDlgItemTextW(hwndDlg, IDC_HSTATE, L"Visible");
	}
	else
	{
		if(dwStyle & WS_HSCROLL)
			SetDlgItemTextW(hwndDlg, IDC_HSTATE, L"Visible");
		else
			SetDlgItemTextW(hwndDlg, IDC_HSTATE, L"Disabled");
	}

	if(GetScrollInfo(hwnd, bartype, &si))
	{
		SetDlgItemInt(hwndDlg, IDC_HMIN, si.nMin, TRUE);
		SetDlgItemInt(hwndDlg, IDC_HMAX, si.nMax, TRUE);
		SetDlgItemInt(hwndDlg, IDC_HPOS, si.nPos, TRUE);
		SetDlgItemInt(hwndDlg, IDC_HPAGE, si.nPage, TRUE);

		if(bartype == SB_HORZ)
		{
			if(dwStyle & WS_HSCROLL)
				SetDlgItemTextW(hwndDlg, IDC_HSTATE, L"Visible");
			else
				SetDlgItemTextW(hwndDlg, IDC_HSTATE, L"Hidden");
		}
	}
	else
	{
		SetDlgItemTextW(hwndDlg, IDC_HMIN, L"");
		SetDlgItemTextW(hwndDlg, IDC_HMAX, L"");
		SetDlgItemTextW(hwndDlg, IDC_HPOS, L"");
		SetDlgItemTextW(hwndDlg, IDC_HPAGE, L"");
		SetDlgItemTextW(hwndDlg, IDC_HSTATE, L"Disabled");
	}

	bartype = SB_VERT;

	if(lstrcmpiW(ach, L"ScrollBar") == 0)
	{
		if((dwStyle & SBS_VERT) == SB_VERT)
			bartype = SB_CTL;

		SetDlgItemTextW(hwndDlg, IDC_VSTATE, L"Visible");
	}
	else
	{
		if(dwStyle & WS_VSCROLL)
			SetDlgItemTextW(hwndDlg, IDC_VSTATE, L"Visible");
		else
			SetDlgItemTextW(hwndDlg, IDC_VSTATE, L"Disabled");
	}

	if(GetScrollInfo(hwnd, bartype, &si))
	{
		SetDlgItemInt(hwndDlg, IDC_VMIN, si.nMin, TRUE);
		SetDlgItemInt(hwndDlg, IDC_VMAX, si.nMax, TRUE);
		SetDlgItemInt(hwndDlg, IDC_VPOS, si.nPos, TRUE);
		SetDlgItemInt(hwndDlg, IDC_VPAGE, si.nPage, TRUE);

		if(bartype == SB_VERT)
		{
			if(dwStyle & WS_VSCROLL)
				SetDlgItemTextW(hwndDlg, IDC_VSTATE, L"Visible");
			else
				SetDlgItemTextW(hwndDlg, IDC_VSTATE, L"Hidden");
		}
	}
	else
	{
		SetDlgItemTextW(hwndDlg, IDC_VMIN, L"");
		SetDlgItemTextW(hwndDlg, IDC_VMAX, L"");
		SetDlgItemTextW(hwndDlg, IDC_VPOS, L"");
		SetDlgItemTextW(hwndDlg, IDC_VPAGE, L"");
		SetDlgItemTextW(hwndDlg, IDC_VSTATE, L"Disabled");
	}
}
