#ifndef UTILS_INCLUDED
#define UTILS_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

BOOL EnableDlgItem(HWND hwnd, UINT nCtrlId, BOOL fEnabled);
BOOL ShowDlgItem(HWND hwnd, UINT nCtrlId, DWORD dwShowCmd);

int WINAPI GetRectHeight(RECT *rect);
int WINAPI GetRectWidth(RECT *rect);

DWORD GetDlgItemBaseInt(HWND hwnd, UINT ctrlid, int base);
UINT _tstrtoib16(WCHAR *szHexStr);
BOOL EnableDialogTheme(HWND hwnd);

BOOL EnableDebugPrivilege();

WCHAR *GetVersionString(WCHAR *szFileName, WCHAR *szValue, WCHAR *szBuffer, ULONG nLength);

#ifdef __cplusplus
}
#endif

#endif
