//
//	StyleEdit.c
//
//  Copyright (c) 2002 by J Brown
//  Freeware
//
//	Implements the Style Editor dialog box
//
//  void ShowWindowStyleEditor(HWND hwndParent, HWND hwndTarget, BOOL fExtended)
//
//  hwndParent - parent window of dialog
//  hwndTarget - target window
//  fExtended  - display standard (FALSE) or extended (TRUE) window styles
//

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>

#include "FindTool.h"
#include "WinSpy.h"
#include "resource.h"
#include "Utils.h"

typedef struct
{
	HWND   hwndTarget;	// what window are we looking at??
	BOOL   fExtended;	// Extended (TRUE) or Normal (FALSE)

	DWORD  dwStyle;		// original style

} StyleEditState;

static StyleEditState state;

void FillStyleLists(HWND hwndTarget, HWND hwndStyleList, HWND hwndExStyleList,
					BOOL fAllStyles, BOOL fExtControl);

//
//	Define our callback function for the Window Finder Tool
//
static UINT CALLBACK StyleEditWndFindProc(HWND hwndTool, UINT uCode, HWND hwnd)
{
	HWND hwndDlg;
	WCHAR szText[120];

	DWORD dwStyle;

	switch(uCode)
	{
	case WFN_END:
		hwndDlg  = GetParent(hwndTool);

		if(GetClassLongW(state.hwndTarget, GCW_ATOM) == GetClassLongW(hwnd, GCW_ATOM))
		{
			dwStyle = GetWindowLong(hwnd, GWL_STYLE);

			wsprintfW(szText, L"%08X", dwStyle);

			SetDlgItemTextW(hwndDlg, IDC_EDIT1, szText);

			FillStyleLists(hwnd, GetDlgItem(hwndDlg, IDC_LIST1), 0, TRUE, FALSE);
			SendDlgItemMessageW(hwnd, IDC_LIST1, LB_SETTOPINDEX, 0, 0);
		}
		else
		{
			wsprintfW(szText, L"Window %08X\n\r\n\rUnable to copy this window's styles, \n\rbecause it belongs to a different class.  ", hwnd);
			MessageBoxW(hwndDlg, szText, szAppName, MB_OK|MB_ICONINFORMATION);
		}

		break;

	}
	return 0;
}

static INT_PTR CALLBACK StyleEditProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	static StyleEditState *state;

	HWND hwndList;

	DWORD dwStyle = 0;
	WCHAR szText[32];

	switch(iMsg)
	{
	case WM_INITDIALOG:

		// Passed through in call to DialogBoxParam
		state = (StyleEditState *)lParam;

		hwndList = GetDlgItem(hwnd, IDC_LIST1);

		if(state->fExtended)
			FillStyleLists(state->hwndTarget, NULL, hwndList, TRUE, FALSE);
		else
			FillStyleLists(state->hwndTarget, hwndList, NULL, TRUE, FALSE);

		SendMessageW(hwndList, LB_SETTOPINDEX, 0, 0);

		MakeFinderTool(GetDlgItem(hwnd, IDC_DRAGGER), StyleEditWndFindProc);

		if(state->fExtended)
			dwStyle = GetWindowLong(state->hwndTarget, GWL_EXSTYLE);
		else
			dwStyle = GetWindowLong(state->hwndTarget, GWL_STYLE);

		wsprintfW(szText, L"%08X", dwStyle);
		SetDlgItemTextW(hwnd, IDC_EDIT1, szText);

		return TRUE;

	case WM_CLOSE:
		EndDialog(hwnd, 0);
		return TRUE;

	case WM_MEASUREITEM:
		return FunkyList_MeasureItem(hwnd, (UINT)wParam, (MEASUREITEMSTRUCT *)lParam);

	case WM_DRAWITEM:
		if(wParam == IDC_LIST1 || wParam == IDC_LIST2)
			return FunkyList_DrawItem(hwnd, (UINT)wParam, (DRAWITEMSTRUCT *)lParam);
		else
			return FALSE;

		//if clicked on one of the underlined static controls, then
	//display window info..
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_APPLY:

			dwStyle = GetDlgItemBaseInt(hwnd, IDC_EDIT1, 16);

			if(state->fExtended)
				SetWindowLongW(state->hwndTarget, GWL_EXSTYLE, dwStyle);
			else
				SetWindowLongW(state->hwndTarget, GWL_STYLE, dwStyle);

			SetWindowPos(state->hwndTarget, 0,
				0,0,0,0,
				SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|
				SWP_NOACTIVATE|SWP_DRAWFRAME);

			InvalidateRect(state->hwndTarget, 0, TRUE);

			return TRUE;

		case IDCANCEL:
			EndDialog(hwnd, 0);
			return TRUE;

		case IDC_CLEAR:
			//remove selection from all items
			SendDlgItemMessageW(hwnd, IDC_LIST1, LB_SETSEL, FALSE, (LONG)-1);
			return TRUE;

		}

		switch(HIWORD(wParam))
		{
		case LBN_SELCHANGE:
			if(LOWORD(wParam) == IDC_LIST1)
			{
				int cursel, caretidx;

				hwndList = GetDlgItem(hwnd, IDC_LIST1);

				dwStyle  = GetDlgItemBaseInt(hwnd, IDC_EDIT1, 16);

				caretidx = (int)SendMessageW(hwndList, LB_GETCARETINDEX, 0, 0);
				cursel   = (int)SendMessageW(hwndList, LB_GETSEL, caretidx, 0);

				if(cursel)
					dwStyle |=  SendMessageW(hwndList, LB_GETITEMDATA, caretidx, 0);
				else
					dwStyle &= ~SendMessageW(hwndList, LB_GETITEMDATA, caretidx, 0);

				wsprintfW(szText, L"%08X", dwStyle);
				SetDlgItemTextW(hwnd, IDC_EDIT1, szText);

				return TRUE;
			}

			return FALSE;
		}

		return FALSE;
	}
	return FALSE;
}


void ShowWindowStyleEditor(HWND hwndParent, HWND hwndTarget, BOOL fExtended)
{
	state.hwndTarget = hwndTarget;
	state.dwStyle    = 0;
	state.fExtended  = fExtended;

	DialogBoxParamW(GetModuleHandleW(0), MAKEINTRESOURCEW(IDD_STYLE_EDIT), hwndParent, StyleEditProc, (LPARAM)&state);

	// Update the main display
	SetGeneralInfo(hwndTarget);
	SetStyleInfo(hwndTarget);
}
