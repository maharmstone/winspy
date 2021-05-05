//
//	WinSpy.c
//
//  Copyright (c) 2002 by J Brown
//  Freeware
//
//	Main implementation.
//
//	v 1.7.1	- moved system-menu items to appear before the Close item
//

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <commctrl.h>

#include "resource.h"
#include "WinSpy.h"
#include "FindTool.h"
#include "CaptureWindow.h"
#include "BitmapButton.h"
#include "Utils.h"

HWND		hwndPin;		// Toolbar with pin bitmap
HWND		hwndSizer;		// Sizing grip for bottom-right corner
HWND		hwndToolTip;	// tooltip for main window controls only
HINSTANCE	hInst;			// Current application instance

WCHAR szHexFmt[]	= L"%08X";
WCHAR szAppName[]	= L"WinSpy++";

//
//	Current window being spied on
//
HWND       spy_hCurWnd = 0;
WNDCLASSEXW spy_WndClassEx;
WNDPROC    spy_WndProc;
BOOL       spy_fPassword = FALSE;   // is it a password (edit) control?
WCHAR      spy_szPassword[200];
WCHAR      spy_szClassName[70];


static TBBUTTON tbbPin[] =
{
	{	0,	IDM_WINSPY_PIN,		TBSTATE_ENABLED, TBSTYLE_CHECK,  0, 0	},
};

#define IDC_PIN_TOOLBAR 2000 // must be unique, so check resource.h
#define TOOLBAR_PIN_STYLES  (TBSTYLE_FLAT |	WS_CHILD | WS_VISIBLE | \
						CCS_NOPARENTALIGN | CCS_NORESIZE | CCS_NODIVIDER)

DialogTab WinSpyTab[NUMTABCONTROLITEMS] =
{
	0, L"General",		IDD_TAB_GENERAL,	GeneralDlgProc,
	0, L"Styles",		IDD_TAB_STYLES,		StyleDlgProc,
	0, L"Properties",	IDD_TAB_PROPERTIES, PropertyDlgProc,
	0, L"Class",			IDD_TAB_CLASS,		ClassDlgProc,
	0, L"Windows",		IDD_TAB_WINDOWS,	WindowDlgProc,
	0, L"Process",		IDD_TAB_PROCESS,    ProcessDlgProc,
};

static int nCurrentTab = 0;

//
//	Try to get class information normally - if
//  it's a private application class, then we need to
//  do this remotely
//
void GetRemoteInfo(HWND hwnd)
{
	BOOL b;

	ZeroMemory(&spy_WndClassEx, sizeof(WNDCLASSEXW));
	spy_WndClassEx.cbSize = sizeof(WNDCLASSEXW);

	b = GetClassInfoExW(0, spy_szClassName, &spy_WndClassEx);

	//try to use the standard methods to get the window procedure
	//and class information. If that fails, then we have to inject
	//a remote thread into the window's process and call the functions
	//from there.
	if(spy_WndProc == 0 || b == FALSE || spy_fPassword)
	{
		//Remote Threads only available under Windows NT
		if(GetVersion() < 0x80000000)
		{
			// doesn't work with debug info!!!!!!!!
			// make sure we never call this function unless we have
			// a release build!!!
			GetRemoteWindowInfo(hwnd, &spy_WndClassEx, &spy_WndProc, spy_szPassword, 200);
		}
		else
		{
			SendMessageW(hwnd, WM_GETTEXT, 200, (LPARAM)spy_szPassword);
		}
	}
}

//
//	Top-level function for retrieving+displaying a window's
//  information (styles/class/properties etc)
//
void DisplayWindowInfo(HWND hwnd)
{
	if(hwnd == 0) return;
	spy_hCurWnd = hwnd;

	GetClassNameW(hwnd, spy_szClassName, 70);

	if(IsWindowUnicode(hwnd))
		spy_WndProc = (WNDPROC)GetWindowLongPtrW(hwnd, GWLP_WNDPROC);
	else
		spy_WndProc = (WNDPROC)GetWindowLongPtrA(hwnd, GWLP_WNDPROC);

	// If an password-edit control, then we can
	// inject our thread to get the password text!
	if(lstrcmpiW(spy_szClassName, L"Edit") == 0)
	{
		// If a password control
		DWORD dwStyle = GetWindowLong(hwnd, GWL_STYLE);

		if(dwStyle & ES_PASSWORD)
			spy_fPassword = TRUE;
		else
			spy_fPassword = FALSE;
	}
	else
		spy_fPassword = FALSE;


	// do classinfo first, so we can get the window procedure
	if(spy_fPassword || nCurrentTab == CLASS_TAB)
	{
		GetRemoteInfo(hwnd);
		SetClassInfo(hwnd);
	}

	SetGeneralInfo(hwnd);
	SetStyleInfo(hwnd);
	SetPropertyInfo(hwnd);
	SetWindowInfo(hwnd);
	SetScrollbarInfo(hwnd);
	SetProcessInfo(hwnd);
}

//
//	User-defined callback function for the Find Tool
//
UINT CALLBACK WndFindProc(HWND hwndTool, UINT uCode, HWND hwnd)
{
	HWND hwndMain = GetParent(hwndTool);

	WCHAR ach[90] = {0};
	WCHAR szClass[70] = {0};

	static BOOL fFirstDrag = TRUE;
	static HWND hwndLastTarget;
	static BOOL fOldShowHidden;

	switch(uCode)
	{
	case WFN_SELCHANGED:

		spy_hCurWnd = hwnd;
		spy_WndProc = 0;

		if(fShowInCaption)
		{
			GetClassNameW(hwnd, szClass, sizeof(szClass) / sizeof(WCHAR));
			wsprintfW(ach, L"%s [%08X, %s]", szAppName, hwnd, szClass);
			SetWindowTextW(hwndMain, ach);
		}

		SetGeneralInfo(hwnd);
		return 0;

	case WFN_BEGIN:

		hwndLastTarget = spy_hCurWnd;

		spy_hCurWnd = hwnd;

		if(fMinimizeWinSpy)
		{
			SetWindowLayout(hwndMain, WINSPY_MINIMIZED);
			InvalidateRect(hwndMain, 0, 0);
			UpdateWindow(hwndMain);
		}

		fOldShowHidden = fShowHidden;
		break;

	case WFN_CANCELLED:

		// Restore the current window + Fall through!
		spy_hCurWnd = hwndLastTarget;

		if(fShowInCaption)
		{
			GetClassNameW(spy_hCurWnd, szClass, sizeof(szClass) / sizeof(WCHAR));

			wsprintfW(ach, L"%s [%08X, %s]", szAppName, spy_hCurWnd, szClass);
			SetWindowTextW(hwndMain, ach);
		}

	case WFN_END:

		ShowWindow(hwndMain, SW_SHOW);

		if(fMinimizeWinSpy || fFirstDrag)
		{
			fFirstDrag = FALSE;
			SetWindowLayout(hwndMain, WINSPY_LASTMAX);
		}

		DisplayWindowInfo(spy_hCurWnd);

		if(fMinimizeWinSpy)
		{
			InvalidateRect(hwndMain, 0, TRUE);
			InvalidateRect(WinSpyTab[nCurrentTab].hwnd, 0, TRUE);
		}

		fShowHidden = fOldShowHidden;
		break;

	case WFN_CTRL_UP:
		ToggleWindowLayout(hwndMain);
		break;

	case WFN_CTRL_DOWN:
		ToggleWindowLayout(hwndMain);
		break;

	case WFN_SHIFT_UP:
		SetWindowPos(hwndMain, 0,0,0,0, 0, SWP_SHOWONLY);
		fShowHidden = fOldShowHidden;
		break;

	case WFN_SHIFT_DOWN:
		SetWindowPos(hwndMain, 0,0,0,0, 0, SWP_HIDEONLY);
		fShowHidden = TRUE;
		break;

	}
	return 0;
}

//
// Check/Uncheck the specified item in the system menu
//
void CheckSysMenu(HWND hwnd, UINT uItemId, BOOL fChecked)
{
	HMENU hSysMenu;

	hSysMenu = GetSystemMenu(hwnd, FALSE);

	if(fChecked)
		CheckMenuItem(hSysMenu, uItemId, MF_CHECKED|MF_BYCOMMAND);
	else
		CheckMenuItem(hSysMenu, uItemId, MF_UNCHECKED|MF_BYCOMMAND);
}

//
// Is the specified item in system menu checked?
//
BOOL IsSysMenuChecked(HWND hwnd, UINT uItemId)
{
	HMENU hSysMenu;
	DWORD dwState;

	hSysMenu = GetSystemMenu(hwnd, FALSE);

	dwState = GetMenuState(hSysMenu, uItemId, MF_BYCOMMAND);

	return (dwState & MF_CHECKED) ? TRUE : FALSE;
}

//
//	Toggle the checked status for specified item
//
BOOL ToggleSysMenuCheck(HWND hwnd, UINT uItemId)
{
	if(IsSysMenuChecked(hwnd, uItemId))
	{
		CheckSysMenu(hwnd, uItemId, FALSE);
		return FALSE;
	}
	else
	{
		CheckSysMenu(hwnd, uItemId, TRUE);
		return TRUE;
	}
}

//
//	Determine the window layout and check/uncheck  the
//  maximized menu item accordingly
//
void SetSysMenuIconFromLayout(HWND hwnd, UINT layout)
{
	if(layout == WINSPY_MINIMIZED)
		CheckSysMenu(hwnd, SC_MAXIMIZE, FALSE);
	else
		CheckSysMenu(hwnd, SC_MAXIMIZE, TRUE);
}

//
//	Create a sizing grip for the lower-right corner
//
HWND CreateSizeGrip(HWND hwndDlg)
{
	HWND hwndSizeGrip;

	// Create a sizing grip for the lower-right corner
	hwndSizeGrip = CreateWindowW(
		L"Scrollbar",
		L"",
		WS_VISIBLE|WS_CHILD|SBS_SIZEGRIP|
		SBS_SIZEBOXBOTTOMRIGHTALIGN|WS_CLIPSIBLINGS,
		0,0,20,20,
		hwndDlg, 0, hInst, 0);

	return hwndSizeGrip;
}

//
//	Create a tooltip control,
//
HWND CreateTooltip(HWND hwndDlg)
{
	HWND hwndTT;
	TOOLINFOW ti;
	int i;
	BOOL fRet;

	struct CtrlTipsTag
	{
		UINT  uDlgId;	// -1 for main window, 0-n for tab dialogs
		UINT  uCtrlId;
		WCHAR szText[50];

	} CtrlTips[] =
	{
		-1, IDC_DRAGGER,    L"Window Finder Tool",
		-1, IDC_PIN_TOOLBAR,L"Keep On-Screen (F4)",
		-1, IDC_MINIMIZE,   L"Minimize On Use",
		-1, IDC_HIDDEN,		L"Display Hidden Windows",
		-1, IDC_CAPTURE,    L"Capture Current Window (Alt+C)",
		-1, IDOK,			L"Exit WinSpy++",
		-1, IDC_EXPAND,     L"Expand / Collapse (F3)",
		-1, IDC_REFRESH,    L"Refresh Window List (F6)",
		-1, IDC_LOCATE,     L"Locate Current Window",
		-1, IDC_FLASH,      L"Highlight Selected Window",

		GENERAL_TAB, IDC_HANDLE_MENU,  L"Window Commands",
		GENERAL_TAB, IDC_SETCAPTION,   L"Set Window Caption",
		GENERAL_TAB, IDC_EDITSIZE,     L"Adjust Window Position",
		STYLE_TAB,	 IDC_EDITSTYLE,    L"Edit Styles",
		STYLE_TAB,   IDC_EDITSTYLEEX,  L"Edit Extended Styles",
		PROCESS_TAB, IDC_PROCESS_MENU, L"Process Commands",
		WINDOW_TAB,  IDC_PARENT,       L"Parent Window",
		WINDOW_TAB,  IDC_OWNER,        L"Owner Window",
	};

	// Create tooltip for main window
    hwndTT = CreateWindowExW(WS_EX_TOPMOST,
        TOOLTIPS_CLASSW,
        NULL,
        WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        hwndDlg,
        NULL,
        hInst,
        NULL
        );

	//
	//	Add tooltips to every control (above)
	//
	for(i = 0; i < sizeof(CtrlTips) / sizeof(CtrlTips[0]); i++)
	{
		HWND hwnd;

		if(CtrlTips[i].uDlgId == -1)
			hwnd = hwndDlg;
		else
			hwnd = WinSpyTab[CtrlTips[i].uDlgId].hwnd;

		ti.cbSize   = sizeof(ti);
		ti.uFlags   = TTF_SUBCLASS | TTF_IDISHWND;
		ti.hwnd     = hwnd;
		ti.uId      = (UINT)GetDlgItem(hwnd, CtrlTips[i].uCtrlId);
		ti.hinst    = hInst;
		ti.lpszText = CtrlTips[i].szText;
		ti.lParam   = 0;

		fRet = (BOOL)SendMessageW(hwndTT, TTM_ADDTOOL, 0, (LPARAM)&ti);
	}

	SendMessageW(hwndTT, TTM_ACTIVATE, fEnableToolTips, 0);

	return hwndTT;
}

//
//	Create a toolbar with one button in it, for
//  the pin-button
//
HWND CreatePinToolbar(HWND hwndDlg)
{
	RECT    rect;
	HWND	hwndTB;

	// Create the toolbar to hold pin bitmap
	hwndTB = CreateToolbarEx(
			hwndDlg,
			TOOLBAR_PIN_STYLES,				//,
			IDC_PIN_TOOLBAR,				//toolbar ID (don't need)
			2,								//number of button images
			hInst,							//where the bitmap is
			IDB_PIN_BITMAP,					//bitmap resource name
			tbbPin,							//TBBUTTON structure
			sizeof(tbbPin) / sizeof(tbbPin[0]),
			15,14,15,14,					//0,0,//16,18, 16, 18,
			sizeof(TBBUTTON) );


	// Find out how big the button is, so we can resize the
	// toolbar to fit perfectly
	SendMessageW(hwndTB, TB_GETITEMRECT, 0, (LPARAM)&rect);

	SetWindowPos(hwndTB, HWND_TOP, 0,0,
		rect.right-rect.left,
		rect.bottom-rect.top, SWP_NOMOVE);

	// Setup the bitmap image
	SendMessageW(hwndTB, TB_CHANGEBITMAP, IDM_WINSPY_PIN,
		(LPARAM)MAKELPARAM(fPinWindow, 0));

	// Checked / Unchecked
	SendMessageW(hwndTB, TB_CHECKBUTTON, IDM_WINSPY_PIN, MAKELONG(fPinWindow, 0));

	return hwndTB;
}

//
//	WM_INITDIALOG handler
//
BOOL WinSpy_InitDlg(HWND hwnd)
{
	HBITMAP hBmp1, hBmp2;
	HMENU   hSysMenu;
	int     i;
	HICON   hIcon;
	TCITEMW tcitem;

	// Initialize the finder tool
	MakeFinderTool(GetDlgItem(hwnd, IDC_DRAGGER), WndFindProc);

	// Make the More>> button into a bitmap
	MakeDlgBitmapButton(hwnd, IDC_EXPAND, IDI_ICON16);

	hwndSizer   = CreateSizeGrip(hwnd);
	hwndPin     = CreatePinToolbar(hwnd);

	// Load image lists etc
	InitGlobalWindowTree(GetDlgItem(hwnd, IDC_TREE1));

	// Create each dialog-tab pane,
	for(i = 0; i < NUMTABCONTROLITEMS; i++)
	{
		ZeroMemory(&tcitem, sizeof(tcitem));

		tcitem.mask = TCIF_TEXT;
		tcitem.pszText = (LPWSTR)WinSpyTab[i].szText;

		// Create the dialog pane
		WinSpyTab[i].hwnd = CreateDialogW(hInst,
			MAKEINTRESOURCEW(WinSpyTab[i].id), hwnd, WinSpyTab[i].dlgproc);

		// Create the corresponding tab
		SendDlgItemMessageW(hwnd, IDC_TAB1, TCM_INSERTITEMW, i, (LPARAM)&tcitem);

		SetWindowTextW(WinSpyTab[i].hwnd, WinSpyTab[i].szText);

		// Make this dialog XP-theme aware!
		EnableDialogTheme(WinSpyTab[i].hwnd);
	}

	InitStockStyleLists();

	//
	CheckDlgButton(hwnd, IDC_MINIMIZE, fMinimizeWinSpy);
	CheckDlgButton(hwnd, IDC_HIDDEN,   fShowHidden);

	// Position our contents, work out how big the various
	// layouts are (depending on current system settings for border
	// width, titlebar height etc).
	WinSpyDlg_SizeContents(hwnd);

	// Make sure we test for this FIRST, before we do ANY SetWindowPos'.
	// Otherwise, we will get a WM_WINDOWPOSCHANGED, and set fAlwaysOnTop to 0!
	if(fAlwaysOnTop)
	{
		SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_ZONLY);
	}

	// See what the registry settings are, and setup accordingly
	if(fSaveWinPos && ptPinPos.x != CW_USEDEFAULT && ptPinPos.y != CW_USEDEFAULT)
	{
		SetWindowLayout(hwnd, WINSPY_MINIMIZED);
	}
	else
	{
		RECT rect;
		GetWindowRect(hwnd, &rect);
		ptPinPos.x = rect.left;
		ptPinPos.y = rect.top;
	}

	// display the general tab
	SetWindowPos(WinSpyTab[0].hwnd, 0, 0, 0, 0, 0, SWP_SHOWONLY);

	//
	//	Customize this window's system menu by adding our own
	//  commands.
	//
	hSysMenu = GetSystemMenu(hwnd, FALSE);


	// add items *before* the close item
	InsertMenuW(hSysMenu, SC_CLOSE, MF_BYCOMMAND | MF_ENABLED | MF_STRING, IDM_WINSPY_ABOUT,   L"&About");
	InsertMenuW(hSysMenu, SC_CLOSE, MF_BYCOMMAND | MF_ENABLED | MF_STRING, IDM_WINSPY_OPTIONS, L"&Options...	Alt+Enter");
	InsertMenuW(hSysMenu, SC_CLOSE, MF_BYCOMMAND | MF_ENABLED | MF_STRING, IDM_WINSPY_HELP,    L"&Help	F1");
	InsertMenuW(hSysMenu, SC_CLOSE, MF_BYCOMMAND | MF_SEPARATOR,           -1,                 L"");
	InsertMenuW(hSysMenu, SC_CLOSE, MF_BYCOMMAND | MF_ENABLED | MF_STRING	|
		(fAlwaysOnTop ? MF_CHECKED : 0), IDM_WINSPY_ONTOP,
		L"Always On &Top	Shift+Y");
	InsertMenuW(hSysMenu, SC_CLOSE, MF_BYCOMMAND | MF_SEPARATOR,           -1,                 L"");

/*	AppendMenuW(hSysMenu, MF_SEPARATOR,           -1,                 L"");
	AppendMenuW(hSysMenu, MF_ENABLED | MF_STRING, IDM_WINSPY_ABOUT,   L"&About");
	AppendMenuW(hSysMenu, MF_ENABLED | MF_STRING, IDM_WINSPY_OPTIONS, L"&Options...	Alt+Enter");
	AppendMenuW(hSysMenu, MF_ENABLED | MF_STRING, IDM_WINSPY_HELP,    L"&Help	F1");
	AppendMenuW(hSysMenu, MF_SEPARATOR,           -1,                 L"");
	AppendMenuW(hSysMenu, MF_ENABLED | MF_STRING	|
		(fAlwaysOnTop ? MF_CHECKED : 0), IDM_WINSPY_ONTOP,
		L"Always On &Top	Shift+Y");*/

	// Change the Maximize item to a Toggle Layout item
	ModifyMenuW(hSysMenu, SC_MAXIMIZE, MF_ENABLED|MF_STRING,SC_MAXIMIZE,
		L"&Toggle Layout	F3");

	// Change the bitmaps for the Maximize item
	hBmp1 = LoadBitmapW(hInst, MAKEINTRESOURCEW(IDB_CHECK1));
	hBmp2 = LoadBitmapW(hInst, MAKEINTRESOURCEW(IDB_CHECK2));
	SetMenuItemBitmaps(hSysMenu, SC_MAXIMIZE, MF_BYCOMMAND, hBmp1, hBmp2);

	// Set the dialog's Small Icon
	hIcon = LoadImageW(hInst, MAKEINTRESOURCEW(IDI_ICON1), IMAGE_ICON, 16, 16, 0);
	SendMessageW(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);

	// Set the dialog's Large Icon
	hIcon = LoadImageW(hInst, MAKEINTRESOURCEW(IDI_ICON1), IMAGE_ICON, 32, 32, 0);
	SendMessageW(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);

	// Create tooltips after all other windows
	hwndToolTip = CreateTooltip(hwnd);

	ForceVisibleDisplay(hwnd);

	// Set focus to first item
	return TRUE;
}

//
//	WM_NOTIFY handler
//
UINT WinSpyDlg_NotifyHandler(HWND hwnd, WPARAM wParam, NMHDR *hdr)
{
	NMTREEVIEWW   *nmtv = (NMTREEVIEWW *)hdr;
	TVHITTESTINFO hti;
	TVITEMW       tvi;

	UINT   uCmd;
	HMENU  hMenu, hPopup;
	POINT  pt;

	switch(hdr->code)
	{
	// TabView selection has changed, so show appropriate tab-pane
	case TCN_SELCHANGE:

		ShowWindow(WinSpyTab[nCurrentTab].hwnd, SW_HIDE);

		nCurrentTab = SendMessageW(hdr->hwndFrom, TCM_GETCURSEL, 0, 0);

		SetWindowPos(WinSpyTab[nCurrentTab].hwnd, HWND_TOP, 0,0,0,0, SWP_SHOWONLY);

		if(nCurrentTab == CLASS_TAB)
		{
			GetRemoteInfo(spy_hCurWnd);
			SetClassInfo(spy_hCurWnd);
		}

		return TRUE;

	// TreeView has been right-clicked, so show the popup menu
	case NM_DBLCLK:
	case NM_RCLICK:

		// Find out where in the TreeView the mouse has been clicked
		GetCursorPos(&pt);

		hti.pt = pt;
		ScreenToClient(hdr->hwndFrom, &hti.pt);

		// Find item which has been right-clicked on
		if(SendMessageW(hdr->hwndFrom, TVM_HITTEST, 0, (LPARAM)&hti) &&
			(hti.flags & (TVHT_ONITEM|TVHT_ONITEMRIGHT) ))
		{
			// Now get the window handle, which is stored in the lParam
			// portion of the TVITEM structure..
			ZeroMemory(&tvi, sizeof(tvi));
			tvi.mask = TVIF_HANDLE | TVIF_PARAM;
			tvi.hItem = hti.hItem;

			SendMessageW(hdr->hwndFrom, TVM_GETITEMW, 0, (LPARAM)&tvi);

			if(hdr->code == NM_RCLICK)
			{
				hMenu = LoadMenuW(hInst, MAKEINTRESOURCEW(IDR_MENU3));
				hPopup = GetSubMenu(hMenu, 0);

				WinSpy_SetupPopupMenu(hPopup, (HWND)tvi.lParam);

				// Show the menu
				uCmd = TrackPopupMenu(hPopup, TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, 0, hwnd, 0);

				// Act accordingly
				WinSpy_PopupCommandHandler(hwnd, uCmd, (HWND)tvi.lParam);

				DestroyMenu(hMenu);

				return TRUE;
			}
			/*else if(!(hti.flags & TVHT_ONITEMICON))
			{
				FlashWindowBorder((HWND)tvi.lParam, TRUE);

				// Return non-zero to prevent item from expanding when double-clicked
				SetWindowLongW(hwnd, DWL_MSGRESULT, TRUE);
				return TRUE;
			}*/
		}

		return TRUE;

	// TreeView selection has changed, so update the main window properties
	case TVN_SELCHANGED:

		if(IsWindowVisible(GetDlgItem(hwnd, IDC_TREE1)))
		{
			//Find the window handle stored in the TreeView item's lParam
			ZeroMemory(&tvi, sizeof(tvi));

			tvi.mask = TVIF_HANDLE | TVIF_PARAM;
			tvi.hItem = nmtv->itemNew.hItem;

			// Get TVITEM structure
			SendMessageW(hdr->hwndFrom, TVM_GETITEMW, 0, (LPARAM)&tvi);

			DisplayWindowInfo((HWND)tvi.lParam);
		}

		return TRUE;
	}

	return 0;
}

//
//	WM_SYSCOLORCHANGE handler
//
BOOL WinSpyDlg_SysColorChange(HWND hwnd)
{
	int i;

	// forward this to all the dialogs - they can look after
	// their own controls
	for(i = 0; i < NUMTABCONTROLITEMS; i++)
		PostMessageW(WinSpyTab[i].hwnd, WM_SYSCOLORCHANGE, 0, 0);

	// Set the treeview colours
	SendMessageW(GetDlgItem(hwnd, IDC_TREE1), TVM_SETBKCOLOR, 0, GetSysColor(COLOR_WINDOW));


	// Recreate toolbar, so it uses new colour scheme
	DestroyWindow(hwndPin);

	hwndPin = CreatePinToolbar(hwnd);

	// Send a WM_SIZE so that the pin toolbar gets repositioned
	SetWindowPos(hwnd, 0, 0, 0, 0, 0,
		SWP_NOMOVE|SWP_NOSIZE|
		SWP_NOZORDER|SWP_NOACTIVATE|
		SWP_FRAMECHANGED);

	return TRUE;
}

#ifdef _DEBUG
void DumpRect(HWND hwnd)
{
	RECT  rect;
	WCHAR ach[80];
	GetWindowRect(hwnd, &rect);
	wsprintfW(ach, L"%d %d %d %d", rect.left, rect.top, rect.right, rect.bottom);
	OutputDebugString(ach);

}
#endif

//
//	Dialog procedure for main window
//
INT_PTR WINAPI DialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_INITDIALOG:
		return WinSpy_InitDlg(hwnd);

	case WM_CLOSE:
		ExitWinSpy(hwnd, 0);
		return TRUE;

	case WM_DESTROY:
		DeInitGlobalWindowTree(GetDlgItem(hwnd, IDC_TREE1));
		return TRUE;

	case WM_SYSCOLORCHANGE:
		return WinSpyDlg_SysColorChange(hwnd);

	case WM_SYSCOMMAND:
		return WinSpyDlg_SysMenuHandler(hwnd, wParam, lParam);

	case WM_COMMAND:
		return WinSpyDlg_CommandHandler(hwnd, wParam, lParam);

	case WM_SIZE:
		return WinSpyDlg_Size(hwnd, wParam, lParam);

	case WM_SIZING:
		return WinSpyDlg_Sizing(hwnd, (UINT)wParam, (RECT *)lParam);

	case WM_NCHITTEST:
		return WinSpyDlg_FullWindowDrag(hwnd, wParam, lParam);

	case WM_ENTERSIZEMOVE:
		return WinSpyDlg_EnterSizeMove(hwnd);

	case WM_EXITSIZEMOVE:
		return WinSpyDlg_ExitSizeMove(hwnd);

	case WM_WINDOWPOSCHANGED:
		return WinSpyDlg_WindowPosChanged(hwnd, (WINDOWPOS *)lParam);

	case WM_NOTIFY:
		return WinSpyDlg_NotifyHandler(hwnd, wParam, (NMHDR *)lParam);

	case WM_DRAWITEM:
		return DrawBitmapButton((DRAWITEMSTRUCT *)lParam);

	// Update our layout based on new settings
	case WM_SETTINGCHANGE:
		WinSpyDlg_SizeContents(hwnd);
		return TRUE;
	}

	return FALSE;
}

//
//	The only reason I do this is to "obfuscate" the main
//  window. All the windows are just dialogs (#32770), but
//  I use this function to make a new dialog class with
//  a different name..no other reason.
//
//	Check the dialog resources to see how the new name
//  is specified. (MFC extensions must be turned off for the resource
//  to enable this feature).
//
void RegisterDialogClass(WCHAR szNewName[])
{
	WNDCLASSEXW wc;

	// Get the class structure for the system dialog class
	wc.cbSize = sizeof(wc);
	GetClassInfoExW(0, L"#32770", &wc);

	// Make sure our new class does not conflict
	wc.style &= ~CS_GLOBALCLASS;

	// Register an identical dialog class, but with a new name!
	wc.lpszClassName = szNewName;

	RegisterClassExW(&wc);
}
//
//	This is where the fun begins
//
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	HWND	hwndMain;
	HACCEL	hAccelTable;
	MSG		msg;

	INITCOMMONCONTROLSEX ice;
	hInst = hInstance;

	ice.dwSize = sizeof ice;
	ice.dwICC  = ICC_BAR_CLASSES      | ICC_TREEVIEW_CLASSES |
		         ICC_LISTVIEW_CLASSES | ICC_TAB_CLASSES      ;

	EnableDebugPrivilege();

	InitCommonControls();//Ex(&ice);

	RegisterDialogClass(L"WinSpy");
	RegisterDialogClass(L"WinSpyPane");

	LoadSettings();

	//DialogBoxW(hInstance, MAKEINTRESOURCEW(IDD_MAIN), 0, DialogProc);

	hwndMain = CreateDialogW(hInstance, MAKEINTRESOURCEW(IDD_MAIN), 0, DialogProc);

	//Initialise the keyboard accelerators
	hAccelTable = LoadAcceleratorsW(hInstance, MAKEINTRESOURCEW(IDR_ACCELERATOR1));

	//
	// UPDATED (fix for Matrox CenterPOPUP feature :)
	//
	//	If we use ShowWindow, then my Matrox card automatically centers WinSpy
	//  on the current monitor (even if we restored WinSpy to it's position last
	//  time we ran). Therefore we use SetWindowPos to display the dialog, as
	//  Matrox don't seem to hook this in their display driver..
	//
	SetWindowPos(hwndMain, 0, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);

	while(GetMessageW(&msg, NULL,0,0))
	{
		// Get the accelerator keys before IsDlgMsg gobbles them up!
		if(!TranslateAcceleratorW(hwndMain, hAccelTable, &msg))
		{
			// Let IsDialogMessage process TAB etc
			if(!IsDialogMessageW(hwndMain, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessageW(&msg);
			}
		}
	}

	SaveSettings();

	return 0;
}

void ExitWinSpy(HWND hwnd, UINT uCode)
{
	DestroyWindow(hwnd);
	PostQuitMessage(uCode);
}
