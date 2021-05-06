//
//	DisplayPropInfo.c
//  Copyright (c) 2002 by J Brown
//	Freeware
//
//	void SetPropertyInfo(HWND hwnd)
//
//	Fill the properties-tab-pane with class info for the
//  specified window
//

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>

#include "resource.h"
#include "WinSpy.h"

//
//	Called once for each window property
//
static BOOL CALLBACK PropEnumProcEx(HWND hwnd, LPWSTR lpszString, HANDLE hData, ULONG_PTR dwUser)
{
	HWND   hwndList = (HWND)dwUser;
	WCHAR  ach[256];
	LVITEMW lvitem;

	wsprintfW(ach, szHexFmt, hData);

	lvitem.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE;
	lvitem.iSubItem = 0;
	lvitem.pszText = ach;
	lvitem.iItem = 0;
	lvitem.state = 0;
	lvitem.stateMask = 0;
	lvitem.iImage = 0;

	SendMessageW(hwndList, LVM_INSERTITEMW, 0, (LPARAM)&lvitem);

	lvitem.iSubItem = 1;

	// check that lpszString is a valid string, and not an ATOM in disguise
	if(((DWORD)lpszString & 0xffff0000) == 0)
	{
		wsprintfW(ach, L"%08X (Atom)", lpszString);

        lvitem.pszText = ach;
	}
	else
	{
        lvitem.pszText = lpszString;
	}

	SendMessageW(hwndList, LVM_SETITEMTEXTW, 0, (LPARAM)&lvitem);

	return TRUE;
}

//
//	Display the window properties (SetProp API)
//
static void EnumWindowProps(HWND hwnd, HWND hwndList)
{
	SendMessageW(hwndList, LVM_DELETEALLITEMS, 0, 0);
	EnumPropsExW(hwnd, PropEnumProcEx, (ULONG_PTR)hwndList);
}

void SetPropertyInfo(HWND hwnd)
{
	if(hwnd == 0) return;
	EnumWindowProps(hwnd, GetDlgItem(WinSpyTab[PROPERTY_TAB].hwnd, IDC_LIST1));
}

