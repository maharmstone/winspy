//
//	DisplayClassInfo.c
//  Copyright (c) 2002 by J Brown
//	Freeware
//
//	void SetClassInfo(HWND hwnd)
//
//	Fill the class-tab-pane with class info for the
//  specified window
//
//	History:
//
//	1.7.1 - fixed bug where 'resolve' window-proc wasn't getting updated
//

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>

#include "WinSpy.h"
#include "resource.h"
#include "Utils.h"

BOOL GetRemoteWindowInfo(HWND hwnd, WNDCLASSEXW *pClass, WNDPROC *pProc, WCHAR *pszText, int nTextLen);


void VerboseClassName(WCHAR ach[])
{
	if     (lstrcmpiW(ach, L"#32770") == 0)	lstrcatW(ach, L" (Dialog)");
	else if(lstrcmpiW(ach, L"#32768") == 0)	lstrcatW(ach, L" (Menu)");
	else if(lstrcmpiW(ach, L"#32769") == 0)	lstrcatW(ach, L" (Desktop window)");
	else if(lstrcmpiW(ach, L"#32771") == 0)	lstrcatW(ach, L" (Task-switch window)");
	else if(lstrcmpiW(ach, L"#32772") == 0)	lstrcatW(ach, L" (Icon title)");
}

//
//	Class styles lookup table
//
StyleLookupType ClassLookup[] =
{
	STYLE_(CS_BYTEALIGNCLIENT),
	STYLE_(CS_BYTEALIGNWINDOW),
	STYLE_(CS_OWNDC),
	STYLE_(CS_CLASSDC),
	STYLE_(CS_PARENTDC),
	STYLE_(CS_DBLCLKS),
	STYLE_(CS_GLOBALCLASS),
	STYLE_(CS_HREDRAW),
	STYLE_(CS_VREDRAW),
	STYLE_(CS_NOCLOSE),
	STYLE_(CS_SAVEBITS),

	STYLE_(CS_IME)
};

//
//	Stock Icon lookup table. These values must be converted to
//	stock icon handle values by calling LoadIconW(NULL, ID) before
//	the list can be searched.
//
StyleLookupType IconLookup[] =
{
	STYLE_(IDI_WARNING),
	STYLE_(IDI_ERROR),
	STYLE_(IDI_INFORMATION),
	STYLE_(IDI_APPLICATION),
	STYLE_(IDI_HAND),
	STYLE_(IDI_QUESTION),
	STYLE_(IDI_EXCLAMATION),
	STYLE_(IDI_ASTERISK),

#if(WINVER >= 0x0400)
	STYLE_(IDI_WINLOGO),
#endif

};

//
//	Stock Cursor lookup table. These values must also be
//  converted to stock cursor handles.
//
StyleLookupType CursorLookup[] =
{
	STYLE_(IDC_ARROW),
	STYLE_(IDC_IBEAM),
	STYLE_(IDC_WAIT),
	STYLE_(IDC_CROSS),
	STYLE_(IDC_UPARROW),
	STYLE_(IDC_SIZE),
	STYLE_(IDC_ICON),
	STYLE_(IDC_SIZENWSE),
	STYLE_(IDC_SIZENESW),
	STYLE_(IDC_SIZEWE),
	STYLE_(IDC_SIZENS),
	STYLE_(IDC_SIZEALL),
	STYLE_(IDC_NO),

#if(WINVER >= 0x0500)
	STYLE_(IDC_HAND),
#endif

	STYLE_(IDC_APPSTARTING),

#if(WINVER >= 0x0400)
	STYLE_(IDC_HELP),
#endif

};

//
//	COLOR_xx Brush ID lookup. Needs no conversion
//
StyleLookupType BrushLookup[] =
{
	STYLE_(COLOR_SCROLLBAR),
	STYLE_(COLOR_BACKGROUND),
	STYLE_(COLOR_ACTIVECAPTION),
	STYLE_(COLOR_INACTIVECAPTION),
	STYLE_(COLOR_MENU),
	STYLE_(COLOR_WINDOW),
	STYLE_(COLOR_WINDOWFRAME),
	STYLE_(COLOR_MENUTEXT),
	STYLE_(COLOR_WINDOWTEXT),
	STYLE_(COLOR_CAPTIONTEXT),
	STYLE_(COLOR_ACTIVEBORDER),
	STYLE_(COLOR_INACTIVEBORDER),
	STYLE_(COLOR_APPWORKSPACE),
	STYLE_(COLOR_HIGHLIGHT),
	STYLE_(COLOR_HIGHLIGHTTEXT),
	STYLE_(COLOR_BTNFACE),
	STYLE_(COLOR_BTNSHADOW),
	STYLE_(COLOR_GRAYTEXT),
	STYLE_(COLOR_BTNTEXT),
	STYLE_(COLOR_INACTIVECAPTIONTEXT),
	STYLE_(COLOR_BTNHIGHLIGHT),

#if(WINVER >= 0x0400)
	STYLE_(COLOR_3DDKSHADOW),
	STYLE_(COLOR_3DLIGHT),
	STYLE_(COLOR_INFOTEXT),
	STYLE_(COLOR_INFOBK),
#endif

#if(WINVER >= 0x0500)
	STYLE_(COLOR_HOTLIGHT),
	STYLE_(COLOR_GRADIENTACTIVECAPTION),
	STYLE_(COLOR_GRADIENTINACTIVECAPTION),
#endif

};

//
//	GetStockObject brush lookup. These values must be
//  converted to valid stock brushes.
//
StyleLookupType StkBrLookup[] =
{
	STYLE_(WHITE_BRUSH),
	STYLE_(BLACK_BRUSH),
	STYLE_(LTGRAY_BRUSH),
	STYLE_(GRAY_BRUSH),
	STYLE_(DKGRAY_BRUSH),
	STYLE_(NULL_BRUSH),
};

#define NUM_ICON_STYLES		(sizeof(IconLookup)   /	sizeof(IconLookup[0]))
#define NUM_CURSOR_STYLES	(sizeof(CursorLookup) / sizeof(CursorLookup[0]))
#define NUM_CLASS_STYLES	(sizeof(ClassLookup)  /	sizeof(ClassLookup[0]))
#define NUM_BRUSH_STYLES	(sizeof(BrushLookup)  /	sizeof(BrushLookup[0]))
#define NUM_STKBR_STYLES	(sizeof(StkBrLookup)  /	sizeof(StkBrLookup[0]))

//
//	This table is a combination of the BrushLookup and StkBrLookup tables.
//	All values are handles to stock brushes.
//
StyleLookupType BrushLookup2[NUM_BRUSH_STYLES + NUM_STKBR_STYLES];

#define NUM_BRUSH2_STYLES	(sizeof(BrushLookup2) /	sizeof(BrushLookup2[0]))

//
//	Prepare the resource lookup tables by obtaining the
//  internal handle values for all stock objects.
//
void InitStockStyleLists()
{
	int i;
	for(i = 0; i < NUM_ICON_STYLES; i++)
		IconLookup[i].style = (UINT)LoadIconW(NULL, MAKEINTRESOURCEW(IconLookup[i].style));

	for(i = 0; i < NUM_CURSOR_STYLES; i++)
		CursorLookup[i].style = (UINT)LoadCursorW(NULL, MAKEINTRESOURCEW(CursorLookup[i].style));

	for(i = 0; i < NUM_BRUSH_STYLES; i++)
	{
		BrushLookup2[i].style = (UINT)GetSysColorBrush(BrushLookup[i].style);
		BrushLookup2[i].szName = BrushLookup[i].szName;
	}

	for(i = 0; i < NUM_STKBR_STYLES; i++)
	{
		BrushLookup2[i+NUM_BRUSH_STYLES].style = (UINT)GetStockObject(StkBrLookup[i].style);
		BrushLookup2[i+NUM_BRUSH_STYLES].szName = StkBrLookup[i].szName;
	}

}

//
//	Lookup the specified handle in the style list
//
static int FormatStyle(WCHAR *ach, StyleLookupType *stylelist, int items, UINT matchthis)
{
	int i;

	for(i = 0; i < items; i++)
	{
		if(stylelist[i].style == matchthis)
		{
			wsprintfW(ach, L"%s", stylelist[i].szName);
			return i;
		}
	}

	wsprintfW(ach, szHexFmt, matchthis);
	if(matchthis == 0 || matchthis == -1) lstrcpyW(ach, L"(None)");

	return -1;
}

//
//	Set the class information on the Class Tab, for the specified window
//
void SetClassInfo(HWND hwnd)
{
	WCHAR ach[256];

	int i, numstyles, classbytes;
	HWND hwndDlg = WinSpyTab[CLASS_TAB].hwnd;
	UINT style;

	if(hwnd == 0) return;


	GetClassNameW(hwnd, ach, sizeof(ach) / sizeof(WCHAR));

	// be nice and give the proper name for the following class names
	//
	VerboseClassName(ach);

	SetDlgItemTextW(hwndDlg, IDC_CLASSNAME, ach);

	//class style
	wsprintfW(ach, szHexFmt, spy_WndClassEx.style);
	SetDlgItemTextW(hwndDlg, IDC_STYLE, ach);

	//atom
	wsprintfW(ach, L"%04X", GetClassLongW(hwnd, GCW_ATOM));
	SetDlgItemTextW(hwndDlg, IDC_ATOM, ach);

	//extra class bytes
	wsprintfW(ach, L"%d", spy_WndClassEx.cbClsExtra);
	SetDlgItemTextW(hwndDlg, IDC_CLASSBYTES, ach);

	//extra window bytes
	wsprintfW(ach, L"%d", spy_WndClassEx.cbWndExtra);
	SetDlgItemTextW(hwndDlg, IDC_WINDOWBYTES, ach);

	//menu (not implemented)
	wsprintfW(ach, szHexFmt, GetClassLongPtrW(hwnd, GCLP_MENUNAME));
	SetDlgItemTextW(hwndDlg, IDC_MENUHANDLE, L"(None)");

	//cursor handle
	style = (UINT)GetClassLongPtrW(hwnd, GCLP_HCURSOR);
	FormatStyle(ach, CursorLookup, NUM_CURSOR_STYLES, style);
	SetDlgItemTextW(hwndDlg, IDC_CURSORHANDLE, ach);

	//icon handle
	style = (UINT)GetClassLongPtrW(hwnd, GCLP_HICON);
	FormatStyle(ach, IconLookup, NUM_ICON_STYLES, style);
	SetDlgItemTextW(hwndDlg, IDC_ICONHANDLE, ach);

	//background brush handle
	style = (UINT)GetClassLongPtrW(hwnd, GCLP_HBRBACKGROUND);

	//first of all, search by COLOR_xxx value
	if(-1 == FormatStyle(ach, BrushLookup, NUM_BRUSH_STYLES, style-1))
	{
		//now search by handle value
		i = FormatStyle(ach, BrushLookup2, NUM_BRUSH2_STYLES, style);
		if(i != -1)
		{
			wsprintfW(ach, L"%08X  (%s)", BrushLookup2[i].style, BrushLookup2[i].szName);
		}
	}

	//set the brush handle text
	SetDlgItemTextW(hwndDlg, IDC_BKGNDBRUSH, ach);

	//window procedure
	if(spy_WndProc == 0)
	{
		wsprintfW(ach, L"N/A");
	}
	else
	{
		wsprintfW(ach, szHexFmt, spy_WndProc);
		if(spy_WndProc != spy_WndClassEx.lpfnWndProc)
			lstrcatW(ach, L" (Subclassed)");
	}

	SetDlgItemTextW(hwndDlg, IDC_WNDPROC, ach);

	SetDlgItemTextW(WinSpyTab[GENERAL_TAB].hwnd, IDC_WINDOWPROC2, ach);

	//class window procedure
	if(spy_WndClassEx.lpfnWndProc == 0)
		wsprintfW(ach, L"N/A");
	else
		wsprintfW(ach, szHexFmt, spy_WndClassEx.lpfnWndProc);

	SetDlgItemTextW(hwndDlg, IDC_CLASSPROC, ach);



	//instance handle
	wsprintfW(ach, szHexFmt, spy_WndClassEx.hInstance);
	SetDlgItemTextW(hwndDlg, IDC_INSTANCEHANDLE, ach);

	//
	// fill the combo box with the class styles
	//
	numstyles = 0;
	SendDlgItemMessageW(hwndDlg, IDC_STYLELIST, CB_RESETCONTENT, 0, 0);
	for(i = 0; i < NUM_CLASS_STYLES; i++)
	{
		if(spy_WndClassEx.style & ClassLookup[i].style)
		{
			SendDlgItemMessageW(hwndDlg, IDC_STYLELIST, CB_ADDSTRING, 0,
				(LPARAM)ClassLookup[i].szName);

			numstyles++;
		}
	}

	SendDlgItemMessageW(hwndDlg, IDC_STYLELIST, CB_SETCURSEL, 0, 0);
	EnableDlgItem(hwndDlg, IDC_STYLELIST, numstyles != 0);

	//
	// fill combo box with class extra bytes
	//
	i = 0;
	classbytes = spy_WndClassEx.cbClsExtra;
	EnableDlgItem(hwndDlg, IDC_BYTESLIST, classbytes != 0);
	SendDlgItemMessageW(hwndDlg, IDC_BYTESLIST, CB_RESETCONTENT, 0, 0);

	while(classbytes != 0)
	{
		if(classbytes >= 4)
			wsprintfW(ach, L"+%-8d %08X", i, GetClassLongW(hwnd, i));
		else
			wsprintfW(ach, L"+%-8d (Unavailable)", i);

		i += 4;
		classbytes = max(classbytes - 4, 0);

		SendDlgItemMessageW(hwndDlg, IDC_BYTESLIST, CB_ADDSTRING, 0, (LPARAM)ach);
	}

	SendDlgItemMessageW(hwndDlg, IDC_BYTESLIST, CB_SETCURSEL, 0, 0);
}

