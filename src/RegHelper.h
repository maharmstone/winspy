#ifndef _REGHELPER_INCLUDED
#define _REGHELPER_INCLUDED

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

LONG GetSettingInt(HKEY hkey, WCHAR szKeyName[], LONG nDefault);
BOOL GetSettingBool(HKEY hkey, WCHAR szKeyName[], BOOL nDefault);
LONG GetSettingStr(HKEY hkey, WCHAR szKeyName[], WCHAR szDefault[], WCHAR szReturnStr[], DWORD nSize);
LONG GetSettingBinary(HKEY hkey, WCHAR szKeyName[], void *buf, ULONG nNumBytes);

LONG WriteSettingInt(HKEY hkey, WCHAR szKeyName[], LONG nValue);
LONG WriteSettingBool(HKEY hkey, WCHAR szKeyName[], BOOL nValue);
LONG WriteSettingStr(HKEY hkey, WCHAR szKeyName[], WCHAR szString[]);
LONG WriteSettingBinary(HKEY hkey, WCHAR szKeyName[], void *buf, UINT nNumBytes);


#ifdef __cplusplus
}
#endif

#endif
