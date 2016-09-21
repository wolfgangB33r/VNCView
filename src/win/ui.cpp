//  Copyright (C) 2014 Wolfgang Beer. All Rights Reserved.
//  email wolfgang@pocketvnc.com
//  This file is part of the MobileVNC viewer software.
//
#include "ui.h"

HINSTANCE hThisInstance;
BITMAPINFO bmp   = { 0 };
DWORD*     pBits = NULL;
HBITMAP    hBmp  = NULL;
HDC        hdcMem = NULL;
bool		running = true;
HWND		hwnd;
bool		needBackPaint = true;

int			attempt		  = 0;
bool		connecting  = false;

BOOL fScroll;         // TRUE if scrolling occurred 

// These variables are required for horizontal scrolling. 
int xMinScroll;       // minimum horizontal scroll value 
int xCurrentScroll;   // current horizontal scroll value 
int xMaxScroll;       // maximum horizontal scroll value 

// These variables are required for vertical scrolling. 
int yMinScroll;       // minimum vertical scroll value 
int yCurrentScroll;   // current vertical scroll value 
int yMaxScroll;       // maximum vertical scroll value 


DWORD WINAPI ReceiveVNCMsgThread(LPVOID param) 
{
	int i = 0;
	int msg;
	while(running)
	{
		if(!receiveNextServerMsgType(&msg)) 
		{
			needBackPaint = true;
			InvalidateRect (hwnd, NULL, TRUE);
			UpdateWindow (hwnd);
			printf(".!error receive server msg\n");
			if(conf.reconnect) 
			{
				printf(".!connection lost, try to reconnect %d...\n", attempt);
				connecting = true;
				if(!openConnection(conf.adr, conf.port, conf.reconnect))
				{
					attempt++;
					Sleep(5000);
				}
				else 
				{
					if(!openSession(conf.pwd))
					{
						Sleep(5000);
						attempt++;
					}
				}
			}
			else 
			{
				printf(".!connection closed, bye!\n");
				running = false;
				SendMessage(hwnd, WM_CLOSE, 0, 0);
				return 0;
			}
		}
		else 
		{
			attempt = 0;
			connecting = false;
			switch(msg) 
			{
			case FRAMEBUFFER_UPDATE:
				int nrRec;
				receiveFrameBufferUpdate(&nrRec);
				printf(".received framebuffer update nrOfRects:%d\n", nrRec);
				for(i = 0; i < nrRec; i++)
				{
					FBRECT rect;
					receiveNextRect(&rect);
					for(int p = 0; p < rect.height * rect.width; p++)
					{
						PIXEL pixel;
						receiveNextPixel(&pixel);

						pBits[pixel.y * rect.width + pixel.x] = RGB(pixel.c.r, pixel.c.g, pixel.c.b);
					}
				}
				InvalidateRect(hwnd, NULL, true);
				break;
			case SET_COLORMAP_ENTRIES:
				printf(".colormap entries\n");
				receiveSetColourMapEntries();
				break;
			case BELL:
				printf(".bell\n");
				break;
			case SERVER_CUT_TEXT:
				printf(".client cut text\n");
				int len;
				char* text = 0;
				receiveServerCutText(&len, text);
				break;
			}
		}
		//Sleep(5000);
	}
	return 0;
}

void startReceiveMsgThread()
{
	running = true;
	unsigned long dwThreadID;
	HANDLE hThreadAcceptSockets = CreateThread(NULL, 0, ReceiveVNCMsgThread, 0, 0, &dwThreadID);
	if(hThreadAcceptSockets == NULL)
	{
		printf(".!failed to create thread for incoming server msgs\n");
	}
	CloseHandle(hThreadAcceptSockets);
}

SCROLLINFO si;
char buttonMask = 0;
int keyCode = 0;

LRESULT CALLBACK WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{    
	PAINTSTRUCT ps = { 0 };
	int enc[] = { 5, 0 };
	int xDelta = 0; 
    int yDelta;     // yDelta = new_pos - current_pos 
    int yNewPos;    // new position 
	int xNewPos;
	LPRECT lprc;
	
    switch (message)                  
    {
	case WM_LBUTTONDOWN:
		buttonMask |= LEFT_MOUSE_DOWN;
		sendPointerEvent(buttonMask, LOWORD(lParam) - xCurrentScroll, HIWORD(lParam) - yCurrentScroll);
		break;
	case WM_LBUTTONUP:
		buttonMask &= LEFT_MOUSE_UP;
		sendPointerEvent(buttonMask, LOWORD(lParam) - xCurrentScroll, HIWORD(lParam) - yCurrentScroll);
		break;
	case WM_RBUTTONDOWN:
		buttonMask |= RIGHT_MOUSE_DOWN;
		sendPointerEvent(buttonMask, LOWORD(lParam) - xCurrentScroll, HIWORD(lParam) - yCurrentScroll);
		break;
	case WM_RBUTTONUP:
		buttonMask &= RIGHT_MOUSE_UP;
		sendPointerEvent(buttonMask, LOWORD(lParam) - xCurrentScroll, HIWORD(lParam) - yCurrentScroll);
		break;
	case WM_MBUTTONDOWN:
		buttonMask |= MIDDLE_MOUSE_DOWN;
		sendPointerEvent(buttonMask, LOWORD(lParam) - xCurrentScroll, HIWORD(lParam) - yCurrentScroll);
		break;
	case WM_MBUTTONUP:
		buttonMask &= MIDDLE_MOUSE_UP;
		sendPointerEvent(buttonMask, LOWORD(lParam) - xCurrentScroll, HIWORD(lParam) - yCurrentScroll);
		break;
	case WM_MOUSEMOVE:
		sendPointerEvent(buttonMask, LOWORD(lParam) - xCurrentScroll, HIWORD(lParam) - yCurrentScroll);
		break;
	case WM_KEYDOWN:
		keyCode = (int)wParam;
		printf(".key down event %d\n", wParam);
		break;
	case WM_CHAR:
		keyCode = (int)wParam; // if char translation delivered different key code change the code
		printf(".char event %d\n", wParam);
		break;
	case WM_KEYUP:
		sendKeyEvent(KEY_DOWN, vncKeyCode(keyCode));
		sendKeyEvent(KEY_UP, vncKeyCode(keyCode));
		break;
	case WM_ERASEBKGND:
		return false; // removes some flickering during repaint
		break;
/*	case WM_SIZING:
		lprc = (LPRECT) lParam;
		if(lprc->right - lprc->left > getScreenWidth()) 
		{
			lprc->right = lprc->left + getScreenWidth();
		}
		if(lprc->bottom - lprc->top > getScreenHeight()) 
		{
			lprc->bottom = lprc->top + getScreenHeight();
		}
		needBackPaint = true;
		return 0;
		break;
		*/
	case WM_SIZE:
		int xNewSize; 
        int yNewSize; 
        xNewSize = LOWORD(lParam); 
        yNewSize = HIWORD(lParam); 
		if(xNewSize > getScreenWidth()) 
		{
			xCurrentScroll = (xNewSize - getScreenWidth()) / 2;
		}
		if(yNewSize > getScreenHeight()) 
		{
			yCurrentScroll = (yNewSize - getScreenHeight()) / 2;
		}
		/*
		xCurrentScroll = min(xCurrentScroll, xMaxScroll); 
        si.cbSize = sizeof(si); 
        si.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS; 
        si.nMin   = xMinScroll; 
        si.nMax   = xMaxScroll; 
        si.nPage  = xNewSize; 
        si.nPos   = xCurrentScroll; 
        SetScrollInfo(hwnd, SB_HORZ, &si, TRUE); 

		yCurrentScroll = min(yCurrentScroll, yMaxScroll); 
		si.cbSize = sizeof(si); 
		si.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS; 
        si.nMin   = yMinScroll; 
        si.nMax   = yMaxScroll; 
        si.nPage  = yNewSize; 
        si.nPos   = yCurrentScroll; 
        SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
		*/
		needBackPaint = true;
		break;
	case WM_CREATE:
		// Initialize the horizontal scrolling variables. 
		xMinScroll = 0; 
        xCurrentScroll = 0; 
        xMaxScroll = getScreenWidth(); 

        // Initialize the vertical scrolling variables. 
        yMinScroll = 0; 
        yCurrentScroll = 0; 
        yMaxScroll = getScreenHeight(); 

		// start receiving messages from server
		startReceiveMsgThread();
	
		sendSetEncodings(2, enc);
		sendFramebufferUpdateRequest(0, 0, 0, getScreenWidth(), getScreenHeight());
		break;
    case WM_PAINT:	
		BeginPaint(hwnd, &ps);
		if(needBackPaint)
		{
			HBRUSH brush = CreateSolidBrush(RGB(255, 255, 255));
			RECT r;
			GetWindowRect(hwnd, &r);
			FillRect(ps.hdc, &r, brush);
			needBackPaint = false;
		}
		if(!connecting) // check if we are reconnecting at the moment
		{
			// paint the screenbuffer
			BitBlt(ps.hdc, xCurrentScroll, yCurrentScroll, getScreenWidth(), getScreenHeight(), hdcMem, 0, 0, SRCCOPY);
		}
		else
		{
			RECT rt; 
			GetClientRect(hwnd, &rt);
			DrawText(ps.hdc, TEXT("connecting..."), 13, &rt, DT_CENTER); 
			// while reconnecting paint some reconnection attempt counter
		}
		EndPaint(hwnd, &ps);
		break;

	case WM_HSCROLL: 
        yDelta = 0; 

        switch (LOWORD(wParam)) 
        { 
            // User clicked the scroll bar shaft left of the scroll box. 
            case SB_PAGEUP: 
                xNewPos = xCurrentScroll - 50; 
                break; 

            // User clicked the scroll bar shaft right of the scroll box. 
            case SB_PAGEDOWN: 
                xNewPos = xCurrentScroll + 50; 
                break; 

            // User clicked the left arrow. 
            case SB_LINEUP: 
                xNewPos = xCurrentScroll - 5; 
                break; 

            // User clicked the right arrow. 
            case SB_LINEDOWN: 
                xNewPos = xCurrentScroll + 5; 
                break; 

            // User dragged the scroll box. 
            case SB_THUMBPOSITION: 
                xNewPos = HIWORD(wParam); 
                break; 

            default: 
                xNewPos = xCurrentScroll; 
        } 

        // New position must be between 0 and the screen width. 
        xNewPos = max(0, xNewPos); 
        xNewPos = min(xMaxScroll, xNewPos); 

        // If the current position does not change, do not scroll.
        if (xNewPos == xCurrentScroll) 
            break; 

        // Set the scroll flag to TRUE. 
        fScroll = TRUE; 

        // Determine the amount scrolled (in pixels). 
        xDelta = xNewPos - xCurrentScroll; 

        // Reset the current scroll position. 
        xCurrentScroll = xNewPos; 

        // Scroll the window. (The system repaints most of the 
        // client area when ScrollWindowEx is called; however, it is 
        // necessary to call UpdateWindow in order to repaint the 
        // rectangle of pixels that were invalidated.) 
        ScrollWindowEx(hwnd, -xDelta, -yDelta, (CONST RECT *) NULL, 
            (CONST RECT *) NULL, (HRGN) NULL, (PRECT) NULL, 
            SW_INVALIDATE); 
        UpdateWindow(hwnd); 

        // Reset the scroll bar. 
        si.cbSize = sizeof(si); 
        si.fMask  = SIF_POS; 
        si.nPos   = xCurrentScroll; 
        SetScrollInfo(hwnd, SB_HORZ, &si, TRUE); 
		InvalidateRect(hwnd, NULL, true);
        break; 
    case WM_VSCROLL: 
		xDelta = 0; 
        switch (LOWORD(wParam)) 
        { 
            // User clicked the scroll bar shaft above the scroll box. 
            case SB_PAGEUP: 
                yNewPos = yCurrentScroll - 50; 
                break; 

            // User clicked the scroll bar shaft below the scroll box. 
            case SB_PAGEDOWN: 
                yNewPos = yCurrentScroll + 50; 
                break; 

            // User clicked the top arrow. 
            case SB_LINEUP: 
                yNewPos = yCurrentScroll - 5; 
                break; 

            // User clicked the bottom arrow. 
            case SB_LINEDOWN: 
                yNewPos = yCurrentScroll + 5; 
                break; 

            // User dragged the scroll box. 
            case SB_THUMBPOSITION: 
                yNewPos = HIWORD(wParam); 
                break; 

            default: 
                yNewPos = yCurrentScroll; 
        } 

        // New position must be between 0 and the screen height. 
        yNewPos = max(0, yNewPos); 
        yNewPos = min(yMaxScroll, yNewPos); 

        // If the current position does not change, do not scroll.
        if (yNewPos == yCurrentScroll) 
            break; 

        // Set the scroll flag to TRUE. 
        fScroll = TRUE; 

        // Determine the amount scrolled (in pixels). 
        yDelta = yNewPos - yCurrentScroll; 

        // Reset the current scroll position. 
        yCurrentScroll = yNewPos; 

        // Scroll the window. (The system repaints most of the 
        // client area when ScrollWindowEx is called; however, it is 
        // necessary to call UpdateWindow in order to repaint the 
        // rectangle of pixels that were invalidated.) 
        ScrollWindowEx(hwnd, -xDelta, -yDelta, (CONST RECT *) NULL, 
            (CONST RECT *) NULL, (HRGN) NULL, (PRECT) NULL, 
            SW_INVALIDATE); 
        UpdateWindow(hwnd); 

        // Reset the scroll bar. 
        si.cbSize = sizeof(si); 
        si.fMask  = SIF_POS; 
        si.nPos   = yCurrentScroll; 
        SetScrollInfo(hwnd, SB_VERT, &si, TRUE); 
		InvalidateRect(hwnd, NULL, true);
        break; 
	case WM_DESTROY:
		running = false;
		PostQuitMessage(0);
		break;
    default:                      
        return DefWindowProc (hwnd, message, wParam, lParam);
    }
    return 0;
}

int windowLeft;
int windowTop;
int windowWidth;
int windowHeight;


void showWindow(bool fullscreen)
{
    MSG messages;            /* Here messages to the application are saved */
    
    WNDCLASS	wc;
    wc.style			= CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc		= (WNDPROC) WndProc;
    wc.cbClsExtra		= 0;
    wc.cbWndExtra		= 0;
    wc.hInstance		= GetModuleHandle(NULL);
    wc.hIcon			= LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1));
    wc.hCursor			= 0;
    wc.hbrBackground	= (HBRUSH) GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName		= 0;
    wc.lpszClassName	= L"viewer";
	RegisterClass(&wc);
	WCHAR	name[256];
	MultiByteToWideChar( 0,0, getName(), -1, name, 256);

	DWORD style = WS_CAPTION | WS_SYSMENU; 
	RECT r;
	r.left = 0;
	r.right = getScreenWidth();
	r.top = 0;
	r.bottom = getScreenHeight();
	
	windowLeft = CW_USEDEFAULT;
	windowTop = CW_USEDEFAULT;
	windowWidth = r.right - r.left;
	windowHeight = r.bottom - r.top;
	// add sized for menu caption and borders
	windowHeight += GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CXFIXEDFRAME) * 2;
	windowWidth += GetSystemMetrics(SM_CXFIXEDFRAME) * 2;
	
    if(fullscreen)
	{
		HMONITOR hmon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
		MONITORINFO mi = { sizeof(mi) };
		if (GetMonitorInfo(hmon, &mi)) 
		{
			windowLeft = mi.rcMonitor.left;
			windowTop = mi.rcMonitor.top;
			windowWidth = mi.rcMonitor.right - mi.rcMonitor.left;
			windowHeight = mi.rcMonitor.bottom - mi.rcMonitor.top;
		}
	}
	/* The class is registered, let's create the program*/
    hwnd = CreateWindowEx (
           0,                   /* Extended possibilites for variation */
           L"viewer",         /* Classname */
           name,           /* Title Text */
		   style, /* default window */
           windowLeft,       /* Windows decides the position */
           windowTop,       /* where the window ends up on the screen */
		   windowWidth,                 /* The programs width */
		   windowHeight,                 /* and height in pixels */
           HWND_DESKTOP,        /* The window is a child-window to desktop */
           NULL,                /* No menu */
           hThisInstance,       /* Program Instance handler */
           NULL                 /* No Window Creation data */
           );
/*
	ShowScrollBar(hwnd, SB_HORZ, false);
	
	if(getScreenWidth() > GetSystemMetrics(SM_CXSCREEN)) 
	{
		ShowScrollBar(hwnd, SB_HORZ, true);
	}
	if(getScreenHeight() > GetSystemMetrics(SM_CYSCREEN))
	{
		ShowScrollBar(hwnd, SB_VERT, true);
	}
*/
    // create the bitmap section
	BITMAPINFO bmp   = { 0 };
	bmp.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
	bmp.bmiHeader.biWidth       = getScreenWidth();
	bmp.bmiHeader.biHeight      = getScreenHeight() * -1;
	bmp.bmiHeader.biPlanes      = 1;
	bmp.bmiHeader.biBitCount    = 32;
	bmp.bmiHeader.biCompression = BI_RGB;
	hBmp = CreateDIBSection(NULL, &bmp, DIB_RGB_COLORS, (void**)&pBits, NULL, 0);

	hdcMem = CreateCompatibleDC(NULL);

	SelectObject(hdcMem, hBmp);
	
	/* Make the window visible on the screen */
    ShowWindow (hwnd, SW_SHOW);

    /* Run the message loop. It will run until GetMessage() returns 0 */
    while (GetMessage (&messages, NULL, 0, 0))
    {
        /* Translate virtual-key messages into character messages */
        TranslateMessage(&messages);
        /* Send message to WindowProcedure */
        DispatchMessage(&messages);
    }
}

