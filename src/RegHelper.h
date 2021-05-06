#ifndef _REGHELPER_INCLUDED
#define _REGHELPER_INCLUDED

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

LONG GetSettingInt(HKEY hkey, WCHAR szKeyName[], LONG nDefault);
BOOL GetSettingBool(HKEY hkey, WCHAR szKeyName[], BOOL nDefault);

LONG WriteSettingInt(HKEY hkey, WCHAR szKeyName[], LONG nValue);
LONG WriteSettingBool(HKEY hkey, WCHAR szKeyName[], BOOL nValue);


#ifdef __cplusplus
}
#endif

#endif
