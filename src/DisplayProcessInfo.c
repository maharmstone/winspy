//
//	DisplayProcessInfo.c
//  Copyright (c) 2002 by J Brown
//	Freeware
//
//	void SetProcesInfo(HWND hwnd)
//
//	Fill the process-tab-pane with proces info for the
//  specified window.
//

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <psapi.h>

#include "WinSpy.h"
#include "resource.h"

#include <tlhelp32.h>


typedef BOOL  (WINAPI * EnumProcessModulesProc )(HANDLE, HMODULE *, DWORD, LPDWORD);
typedef DWORD (WINAPI * GetModuleBaseNameProc  )(HANDLE, HMODULE, LPWSTR, DWORD);
typedef DWORD (WINAPI * GetModuleFileNameExProc)(HANDLE, HMODULE, LPWSTR, DWORD);

//
// This uses PSAPI.DLL, which is only available under NT/2000/XP I think,
// so we dynamically load this library, so that we can still run under 9x.
//
//	dwProcessId  [in]
//  szName       [out]
//  nNameSize    [in]
//  szPath       [out]
//  nPathSize    [in]
//
BOOL GetProcessNameByPid(DWORD dwProcessId, WCHAR szName[], DWORD nNameSize, WCHAR szPath[], DWORD nPathSize)
{
	HMODULE hPSAPI;
	HANDLE hProcess;

	HMODULE hModule;
	DWORD   dwNumModules;

	EnumProcessModulesProc  fnEnumProcessModules;
	GetModuleBaseNameProc   fnGetModuleBaseName;
	GetModuleFileNameExProc fnGetModuleFileNameEx;

	// Attempt to load Process Helper library
	hPSAPI = LoadLibraryW(L"psapi.dll");

	if(!hPSAPI)
	{
		szName[0] = '\0';
		return FALSE;
	}

	// OK, we have access to the PSAPI functions, so open the process
	hProcess = OpenProcess(
		//PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
		PROCESS_QUERY_LIMITED_INFORMATION|PROCESS_VM_READ,
		FALSE, dwProcessId);

	if(!hProcess)
	{
		FreeLibrary(hPSAPI);
		return FALSE;
	}


	fnEnumProcessModules  = (EnumProcessModulesProc)GetProcAddress(hPSAPI, "EnumProcessModules");

	fnGetModuleBaseName   = (GetModuleBaseNameProc)  GetProcAddress(hPSAPI, "GetModuleBaseNameW");
	fnGetModuleFileNameEx = (GetModuleFileNameExProc)GetProcAddress(hPSAPI, "GetModuleFileNameExW");

	if(!fnEnumProcessModules || !fnGetModuleBaseName)
	{
		CloseHandle(hProcess);
		FreeLibrary(hPSAPI);
		return FALSE;
	}

	// Find the first module
	if(fnEnumProcessModules(hProcess, &hModule, sizeof(hModule), &dwNumModules))
	{
		// Now get the module name
		if(szName)
			fnGetModuleBaseName(hProcess, hModule, szName, nNameSize);

		// get module filename
		if(szPath)
			fnGetModuleFileNameEx(hProcess, hModule, szPath, nPathSize);
	}
	else
	{
		szName[0] = 0;
		szPath[0] = 0;
	}

	CloseHandle(hProcess);
	FreeLibrary(hPSAPI);

	return TRUE;
}

//
//	Update the Process tab for the specified window
//
void SetProcessInfo(HWND hwnd)
{
	DWORD dwProcessId;
	DWORD dwThreadId;
	WCHAR ach[32];
	WCHAR szPath[MAX_PATH];

	HWND  hwndDlg = WinSpyTab[PROCESS_TAB].hwnd;

	dwThreadId = GetWindowThreadProcessId(hwnd, &dwProcessId);

	// Process Id
	wsprintfW(ach, L"%08X  (%u)", dwProcessId, dwProcessId);
	SetDlgItemTextW(hwndDlg, IDC_PID, ach);

	// Thread Id
	wsprintfW(ach, L"%08X  (%u)", dwThreadId, dwThreadId);
	SetDlgItemTextW(hwndDlg, IDC_TID, ach);

	// Try to get process name and path
	if(GetProcessNameByPid(dwProcessId, ach,    sizeof(ach)    / sizeof(WCHAR),
										szPath, sizeof(szPath) / sizeof(WCHAR)))
	{
		SetDlgItemTextW(hwndDlg, IDC_PROCESSNAME, ach);
		SetDlgItemTextW(hwndDlg, IDC_PROCESSPATH, szPath);
	}
	else
	{
		SetDlgItemTextW(hwndDlg, IDC_PROCESSNAME, L"N/A");
		SetDlgItemTextW(hwndDlg, IDC_PROCESSNAME, L"N/A");
	}


}
