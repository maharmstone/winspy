//
//	RegHelper.c
//
//  Copyright (c) 2002 by J Brown
//  Freeware
//
//	Implements registry helper functions
//

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include "RegHelper.h"

LONG GetSettingInt(HKEY hkey, WCHAR szKeyName[], LONG nDefault)
{
	DWORD type;
	LONG value;
	ULONG len = sizeof(value);

	if(ERROR_SUCCESS == RegQueryValueExW(hkey, szKeyName, 0, &type, (BYTE *)&value, &len))
	{
		if(type != REG_DWORD) return nDefault;
		return value;
	}
	else
	{
		return nDefault;
	}
}

BOOL GetSettingBool(HKEY hkey, WCHAR szKeyName[], BOOL nDefault)
{
	DWORD type;
	BOOL  value;
	ULONG len = sizeof(value);

	if(ERROR_SUCCESS == RegQueryValueExW(hkey, szKeyName, 0, &type, (BYTE *)&value, &len))
	{
		if(type != REG_DWORD) return nDefault;
		return value != 0;
	}
	else
	{
		return nDefault;
	}
}

LONG WriteSettingInt(HKEY hkey, WCHAR szKeyName[], LONG nValue)
{
	return RegSetValueExW(hkey, szKeyName, 0, REG_DWORD, (BYTE *)&nValue, sizeof(nValue));
}

LONG WriteSettingBool(HKEY hkey, WCHAR szKeyName[], BOOL nValue)
{
	return RegSetValueExW(hkey, szKeyName, 0, REG_DWORD, (BYTE *)&nValue, sizeof(nValue));
}
