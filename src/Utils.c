//
//	Utils.c
//
//  Copyright (c) 2002 by J Brown
//  Freeware
//
//	Lots of utility and general helper functions.
//

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <malloc.h>
#include "Utils.h"

int atoi( const char *string );

//
//	Enable/Disable privilege with specified name (for current process)
//
static BOOL EnablePrivilege(WCHAR *szPrivName, BOOL fEnable)
{
	TOKEN_PRIVILEGES tp;
	LUID	luid;
	HANDLE	hToken;

	if(!LookupPrivilegeValueW(NULL, szPrivName, &luid))
		return FALSE;

	if(!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
		return FALSE;

	tp.PrivilegeCount			= 1;
	tp.Privileges[0].Luid		= luid;
	tp.Privileges[0].Attributes = fEnable ? SE_PRIVILEGE_ENABLED : 0;

	AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL);

	CloseHandle(hToken);

	return (GetLastError() == ERROR_SUCCESS);
}


BOOL EnableDebugPrivilege()
{
    return EnablePrivilege(L"SeDebugPrivilege", TRUE);
}

BOOL EnableDlgItem(HWND hwnd, UINT nCtrlId, BOOL fEnabled)
{
	return EnableWindow(GetDlgItem(hwnd, nCtrlId), fEnabled);
}

BOOL ShowDlgItem(HWND hwnd, UINT nCtrlId, DWORD dwShowCmd)
{
	return ShowWindow(GetDlgItem(hwnd, nCtrlId), dwShowCmd);
}

int WINAPI GetRectHeight(RECT *rect)
{
	return rect->bottom - rect->top;
}

int WINAPI GetRectWidth(RECT *rect)
{
	return rect->right - rect->left;
}


//
//	Convert the specified string (with a hex-number in it)
//  into the equivalent hex-value
//
UINT _tstrtoib16(WCHAR *szHexStr)
{
	UINT  num = 0;

	WCHAR *hexptr = szHexStr;
	UINT  ch = *hexptr++;

	while(isxdigit(ch))
	{
		UINT x = ch - L'0';
		if(x > 9 && x <= 42) x -= 7;		//A-Z
		else if(x > 42)   x -= 39;			//a-z

		num = (num << 4) | (x & 0x0f);
		ch = *hexptr++;
	}

	return num;
}

static DWORD GetNumericValue(HWND hwnd, int base)
{
	WCHAR szAddressText[128];

	GetWindowTextW(hwnd, szAddressText, sizeof(szAddressText) / sizeof(WCHAR));

	switch(base)
	{
	case 1:
	case 16:			//base is currently hex
		return _tstrtoib16(szAddressText);

	case 0:
	case 10:			//base is currently decimal
		return _wtoi(szAddressText);

	default:
		return 0;
	}
}

DWORD GetDlgItemBaseInt(HWND hwnd, UINT ctrlid, int base)
{
	return GetNumericValue(GetDlgItem(hwnd, ctrlid), base);
}

//
//	Copied from uxtheme.h
//  If you have this new header, then delete these and
//  #include <uxtheme.h> instead!
//
#define ETDT_DISABLE        0x00000001
#define ETDT_ENABLE         0x00000002
#define ETDT_USETABTEXTURE  0x00000004
#define ETDT_ENABLETAB      (ETDT_ENABLE  | ETDT_USETABTEXTURE)

//
typedef HRESULT (WINAPI * ETDTProc) (HWND, DWORD);

//
//	Try to call EnableThemeDialogTexture, if uxtheme.dll is present
//
BOOL EnableDialogTheme(HWND hwnd)
{
	HMODULE hUXTheme;
	ETDTProc fnEnableThemeDialogTexture;

	hUXTheme = LoadLibraryW(L"uxtheme.dll");

	if(hUXTheme)
	{
		fnEnableThemeDialogTexture =
			(ETDTProc)GetProcAddress(hUXTheme, "EnableThemeDialogTexture");

		if(fnEnableThemeDialogTexture)
		{
			fnEnableThemeDialogTexture(hwnd, ETDT_ENABLETAB);

			FreeLibrary(hUXTheme);
			return TRUE;
		}
		else
		{
			// Failed to locate API!
			FreeLibrary(hUXTheme);
			return FALSE;
		}
	}
	else
	{
		// Not running under XP? Just fail gracefully
		return FALSE;
	}
}

#pragma comment(lib, "version.lib")

//
//	Get the specified file-version information string from a file
//
//	szItem	- version item string, e.g:
//		"FileDescription", "FileVersion", "InternalName",
//		"ProductName", "ProductVersion", etc  (see MSDN for others)
//
WCHAR *GetVersionString(WCHAR *szFileName, WCHAR *szValue, WCHAR *szBuffer, ULONG nLength)
{
	UINT  len;
	PVOID  ver;
	DWORD  *codepage;
	WCHAR  fmt[0x40];
	PVOID  ptr = 0;
	BOOL   result = FALSE;

	szBuffer[0] = '\0';

	len = GetFileVersionInfoSizeW(szFileName, 0);

	if(len == 0 || (ver = malloc(len)) == 0)
		return NULL;

	if(GetFileVersionInfoW(szFileName, 0, len, ver))
	{
		if(VerQueryValueW(ver, L"\\VarFileInfo\\Translation", (void**)&codepage, &len))
		{
			wsprintfW(fmt, L"\\StringFileInfo\\%04x%04x\\%s", (*codepage) & 0xFFFF,
					(*codepage) >> 16, szValue);

			if(VerQueryValueW(ver, fmt, &ptr, &len))
			{
				lstrcpynW(szBuffer, (WCHAR*)ptr, min(nLength, len));
				result = TRUE;
			}
		}
	}

	free(ver);
	return result ? szBuffer : NULL;
}
