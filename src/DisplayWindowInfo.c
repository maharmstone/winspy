//
//	DisplayWindowInfo.c
//  Copyright (c) 2002 by J Brown
//	Freeware
//
//  void SetWindowInfo(HWND hwnd)
//
//	Fill the window-tab-pane with list of child+siblings
//

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>

#include "resource.h"
#include "WinSpy.h"

static BOOL CALLBACK ChildWindowProc(HWND hwnd, LPARAM lParam)
{
	WCHAR  ach[256];
	WCHAR  cname[256];
	WCHAR  wname[256];
	LVITEMW lvitem;

	//only display 1st generation (1-deep) children -
	//(don't display child windows of child windows)
	if(GetParent(hwnd) == spy_hCurWnd)
	{
		GetClassNameW(hwnd, cname, sizeof(cname) / sizeof(WCHAR));
		GetWindowTextW(hwnd, wname, sizeof(wname) / sizeof(WCHAR));
		wsprintfW(ach, szHexFmt, hwnd);

		lvitem.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE;
		lvitem.iSubItem = 0;
		lvitem.pszText = ach;
		lvitem.iItem = 0;
		lvitem.state = 0;
		lvitem.stateMask = 0;
		lvitem.iImage = 0;

		SendMessageW((HWND)lParam, LVM_INSERTITEMW, 0, (LPARAM)&lvitem);

		lvitem.iSubItem = 1;
		lvitem.pszText = cname;

		SendMessageW((HWND)lParam, LVM_SETITEMTEXTW, 0, (LPARAM)&lvitem);

		lvitem.iSubItem = 2;
		lvitem.pszText = wname;

		SendMessageW((HWND)lParam, LVM_SETITEMTEXTW, 0, (LPARAM)&lvitem);
	}
	return TRUE;
}

static BOOL CALLBACK SiblingWindowProc(HWND hwnd, LPARAM lParam)
{
	WCHAR  ach[256];
	WCHAR  cname[256];
	WCHAR  wname[256];
	LVITEMW lvitem;

	//sibling windows must share the same parent
	if(spy_hCurWnd != hwnd && GetParent(hwnd) == GetParent(spy_hCurWnd))
	{
		GetClassNameW(hwnd, cname, sizeof(cname) / sizeof(WCHAR));
		GetWindowTextW(hwnd, wname, sizeof(wname) / sizeof(WCHAR));
		wsprintfW(ach, szHexFmt, hwnd);

		lvitem.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE;
		lvitem.iSubItem = 0;
		lvitem.pszText = ach;
		lvitem.iItem = 0;
		lvitem.state = 0;
		lvitem.stateMask = 0;
		lvitem.iImage = 0;

		SendMessageW((HWND)lParam, LVM_INSERTITEMW, 0, (LPARAM)&lvitem);

		lvitem.iSubItem = 1;
		lvitem.pszText = cname;

		SendMessageW((HWND)lParam, LVM_SETITEMTEXTW, 0, (LPARAM)&lvitem);

		lvitem.iSubItem = 2;
		lvitem.pszText = wname;

		SendMessageW((HWND)lParam, LVM_SETITEMTEXTW, 0, (LPARAM)&lvitem);
	}

	return TRUE;
}

//
//	Get a list of all Child + Siblings for the specified window -
//  Update the Windows tab accordingly
//
void SetWindowInfo(HWND hwnd)
{
	WCHAR ach[10];

	HWND hwndList1 = GetDlgItem(WinSpyTab[WINDOW_TAB].hwnd, IDC_LIST1);
	HWND hwndList2 = GetDlgItem(WinSpyTab[WINDOW_TAB].hwnd, IDC_LIST2);

	if(hwnd == 0) return;

	SendMessageW(hwndList1, LVM_DELETEALLITEMS, 0, 0);
	SendMessageW(hwndList2, LVM_DELETEALLITEMS, 0, 0);

	// Get all children of the window
	EnumChildWindows(hwnd, ChildWindowProc, (LONG)hwndList1);

	// Get children of it's PARENT (i.e, it's siblings!)
	EnumChildWindows(GetParent(hwnd), SiblingWindowProc, (LONG)hwndList2);

	// Set the Parent hyperlink
	wsprintfW(ach, szHexFmt, GetParent(hwnd));
	SetDlgItemTextW(WinSpyTab[WINDOW_TAB].hwnd, IDC_PARENT, ach);

	// Set the Owner hyperlink
	wsprintfW(ach, szHexFmt, GetWindow(hwnd, GW_OWNER));
	SetDlgItemTextW(WinSpyTab[WINDOW_TAB].hwnd, IDC_OWNER, ach);
}
