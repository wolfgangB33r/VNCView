//  Copyright (C) 2014 Wolfgang Beer. All Rights Reserved.
//  email wolfgang@pocketvnc.com
//  This file is part of the MobileVNC viewer software.
//
#include "dialog.h"


void readUserInput(HWND hWndDlg)
{
	TCHAR szBufW[1000];
	// read server address and display number
	HWND h = GetDlgItem(hWndDlg, IDC_COMBO_SERVER_ADR );
	if(h)
	{
		int len = Edit_GetText(h, szBufW, 1000);
		char* adr = (char*)malloc(len + 1);
		wcstombs(adr, szBufW, len+1);
		getServerAdr(adr); 
	}
	// read password
	h = GetDlgItem(hWndDlg, IDC_EDIT_PWD);
	if(h)
	{
		int len = Edit_GetText(h, szBufW, 1000);
		char* pwd = (char*)malloc(len + 1);
		wcstombs(pwd, szBufW, len+1);
		setPassword(pwd);
	}
	h = GetDlgItem(hWndDlg, IDC_RADIO_8BIT);
	if(h)
	{
		if(Button_GetCheck(h)) 
		{
			setColorDepth(8);
		}
	}
	h = GetDlgItem(hWndDlg, IDC_RADIO_16BIT);
	if(h)
	{
		if(Button_GetCheck(h)) 
		{
			setColorDepth(16);
		}
	}
	h = GetDlgItem(hWndDlg, IDC_RADIO_32BIT);
	if(h)
	{
		if(Button_GetCheck(h)) 
		{
			setColorDepth(32);
		}
	}
	h = GetDlgItem(hWndDlg, IDC_CHECK_FULLSCREEN);
	if(h)
	{
		if(Button_GetCheck(h)) 
		{
			setFullscreen(true);
		}
		else 
		{
			setFullscreen(false);
		}
	}
	h = GetDlgItem(hWndDlg, IDC_CHECK_SCROLLBARS);
	if(h)
	{
		if(Button_GetCheck(h)) 
		{
			setScrollbars(true);
		}
		else 
		{
			setScrollbars(false);
		}
	}
	h = GetDlgItem(hWndDlg, IDC_CHECK_RECONNECT);
	if(h)
	{
		if(Button_GetCheck(h)) 
		{
			setAutoReconnect(true);
		}
		else 
		{
			setAutoReconnect(false);
		}
	}
	 
}

void loadAdresses(HWND hwnd)
{
	FILE* fp = fopen("servers.txt", "r+");
	if(fp != NULL)
	{
		char adr[500];
		WCHAR* s[500];
		while ( fgets (adr , 500 , fp) != NULL )
		{
			printf("remembered server: %s", adr);
			wsprintf((LPWSTR)s, TEXT("%S"), adr);
			SendDlgItemMessage(hwnd, IDC_COMBO_SERVER_ADR, CB_ADDSTRING, 0, (LPARAM)s);
		}
		fclose(fp);
	}
}


LRESULT CALLBACK DlgProc(HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	RECT r;
	int width, height;
	switch(Msg)
	{
	case WM_INITDIALOG:
		// reposition the dialog in the middle of the screen
		GetWindowRect(hWndDlg, &r);
		width = r.right - r.left;
		height = r.bottom - r.top;
		MoveWindow(hWndDlg, (GetSystemMetrics(SM_CXSCREEN) - width) / 2, (GetSystemMetrics(SM_CYSCREEN) - height) / 2, width, height, true);
		// load remembered server adresses
		loadAdresses(hWndDlg);
		return TRUE;
	case WM_COMMAND:
		switch(wParam)
		{
		case IDOK:
			readUserInput(hWndDlg);
			EndDialog(hWndDlg, 1);
			return TRUE;
		case IDCANCEL:
			EndDialog(hWndDlg, 0);
			return TRUE;
		}
		break;
	}

	return FALSE;
}

bool showOptionsDialog() 
{
	return DialogBox(GetModuleHandle(0), MAKEINTRESOURCE(IDD_OPTIONS_DLG), NULL, reinterpret_cast<DLGPROC>(DlgProc));
}