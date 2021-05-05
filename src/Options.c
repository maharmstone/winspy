//
//	Options.c
//
//  Copyright (c) 2002 by J Brown
//  Freeware
//
//	Implements the Options dialog for WinSpy
//

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>

#include "WinSpy.h"
#include "RegHelper.h"
#include "resource.h"

BOOL fSaveWinPos;
BOOL fAlwaysOnTop;
BOOL fMinimizeWinSpy;
BOOL fFullDragging;
BOOL fShowHidden;
BOOL fShowDimmed;
UINT uTreeInclude;
BOOL fClassThenText;
BOOL fPinWindow;
BOOL fShowInCaption;
BOOL fEnableToolTips;

extern POINT ptPinPos;
extern UINT  uPinnedCorner;

extern HWND hwndToolTip;

#define REG_BASESTR  L"Software\\Catch22\\WinSpy++ 1.5"

static WCHAR szRegLoc[] = REG_BASESTR;

void LoadSettings(void)
{
	HKEY hkey;

	RegCreateKeyExW(HKEY_CURRENT_USER, szRegLoc, 0, L"", 0, KEY_READ, NULL, &hkey, NULL);

	fSaveWinPos     = GetSettingBool(hkey, L"SavePosition",		TRUE);
	fAlwaysOnTop    = GetSettingBool(hkey, L"AlwaysOnTop",		FALSE);
	fMinimizeWinSpy = GetSettingBool(hkey, L"MinimizeWinSpy",	TRUE);
	fFullDragging   = GetSettingBool(hkey, L"FullDragging",		TRUE);
	fShowHidden     = GetSettingBool(hkey, L"ShowHidden",		FALSE);
	fShowDimmed     = GetSettingBool(hkey, L"ShowDimmed",		TRUE);
	fClassThenText  = GetSettingBool(hkey, L"ClassThenText",     TRUE);
	fPinWindow      = GetSettingBool(hkey, L"PinWindow",         FALSE);
	fShowInCaption  = GetSettingBool(hkey, L"ShowInCaption",     TRUE);
	fEnableToolTips = GetSettingBool(hkey, L"EnableToolTips",	FALSE);
	uTreeInclude    = GetSettingInt (hkey, L"TreeItems", WINLIST_INCLUDE_ALL);

	uPinnedCorner   = GetSettingInt (hkey, L"PinCorner",         0);

	ptPinPos.x      = GetSettingInt(hkey,  L"xpos", CW_USEDEFAULT);
	ptPinPos.y      = GetSettingInt(hkey,  L"ypos", CW_USEDEFAULT);

	RegCloseKey(hkey);
}

void SaveSettings(void)
{
	HKEY hkey;

	RegCreateKeyExW(HKEY_CURRENT_USER, szRegLoc, 0, L"", 0, KEY_WRITE, NULL, &hkey, NULL);

	WriteSettingBool(hkey, L"SavePosition",		fSaveWinPos);
	WriteSettingBool(hkey, L"AlwaysOnTop",		fAlwaysOnTop);
	WriteSettingBool(hkey, L"MinimizeWinSpy",	fMinimizeWinSpy);
	WriteSettingBool(hkey, L"FullDragging",		fFullDragging);
	WriteSettingBool(hkey, L"ShowHidden",		fShowHidden);
	WriteSettingBool(hkey, L"ShowDimmed",		fShowDimmed);
	WriteSettingBool(hkey, L"ClassThenText",     fClassThenText);
	WriteSettingBool(hkey, L"PinWindow",         fPinWindow);
	WriteSettingBool(hkey, L"ShowInCaption",     fShowInCaption);
	WriteSettingBool(hkey, L"EnableToolTips",    fEnableToolTips);
	WriteSettingInt (hkey, L"TreeItems",         uTreeInclude);
	WriteSettingInt (hkey, L"PinCorner",         uPinnedCorner);

	WriteSettingInt (hkey, L"xpos",              ptPinPos.x);
	WriteSettingInt (hkey, L"ypos",              ptPinPos.y);

	RegCloseKey(hkey);
}

INT_PTR CALLBACK OptionsDlgProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	static HWND hwndTarget;

	switch(iMsg)
	{
	case WM_INITDIALOG:
		CheckDlgButton(hwnd, IDC_OPTIONS_SAVEPOS,	    fSaveWinPos);
		CheckDlgButton(hwnd, IDC_OPTIONS_FULLDRAG,	    fFullDragging);
		CheckDlgButton(hwnd, IDC_OPTIONS_DIR,		    fClassThenText);
		CheckDlgButton(hwnd, IDC_OPTIONS_SHOWHIDDEN,    fShowDimmed);
		CheckDlgButton(hwnd, IDC_OPTIONS_SHOWINCAPTION, fShowInCaption);
		CheckDlgButton(hwnd, IDC_OPTIONS_TOOLTIPS,		fEnableToolTips);

		CheckDlgButton(hwnd, IDC_OPTIONS_INCHANDLE,
			(uTreeInclude & WINLIST_INCLUDE_HANDLE) ? TRUE : FALSE);

		CheckDlgButton(hwnd, IDC_OPTIONS_INCCLASS,
			(uTreeInclude & WINLIST_INCLUDE_CLASS)  ? TRUE : FALSE);

		return TRUE;

	case WM_CLOSE:
		EndDialog(hwnd, 0);
		return TRUE;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:

			fSaveWinPos     = IsDlgButtonChecked(hwnd, IDC_OPTIONS_SAVEPOS);
			fFullDragging   = IsDlgButtonChecked(hwnd, IDC_OPTIONS_FULLDRAG);
			fClassThenText  = IsDlgButtonChecked(hwnd, IDC_OPTIONS_DIR);
			fShowDimmed     = IsDlgButtonChecked(hwnd, IDC_OPTIONS_SHOWHIDDEN);
			fShowInCaption  = IsDlgButtonChecked(hwnd, IDC_OPTIONS_SHOWINCAPTION);
			fEnableToolTips = IsDlgButtonChecked(hwnd, IDC_OPTIONS_TOOLTIPS);

			uTreeInclude   = 0;

			if(IsDlgButtonChecked(hwnd, IDC_OPTIONS_INCHANDLE))
				uTreeInclude |= WINLIST_INCLUDE_HANDLE;

			if(IsDlgButtonChecked(hwnd, IDC_OPTIONS_INCCLASS))
				uTreeInclude |= WINLIST_INCLUDE_CLASS;

			EndDialog(hwnd, 0);
			return TRUE;

		case IDCANCEL:
			EndDialog(hwnd, 0);
			return TRUE;
		}

		return FALSE;
	}

	return FALSE;
}


void ShowOptionsDlg(HWND hwndParent)
{
	DialogBoxW(GetModuleHandleW(0), MAKEINTRESOURCEW(IDD_OPTIONS), hwndParent, OptionsDlgProc);

	if(!fShowInCaption)
	{
		SetWindowTextW(hwndParent, szAppName);
	}

	SendMessageW(hwndToolTip, TTM_ACTIVATE, fEnableToolTips, 0);
}
