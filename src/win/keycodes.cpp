//  Copyright (C) 2014 Wolfgang Beer. All Rights Reserved.
//  email wolfgang@pocketvnc.com
//  This file is part of the MobileVNC viewer software.
//
#include "keycodes.h"

int vncKeyCode(int code)
{
	switch(code) 
	{
	case 8:
		return BACKSPACE;
	case 9:
		return TAB;
	case 13:
		return ENTER;
	case 27:
		return ESCAPE;
	case 45:
		return INSERT;
	case 46:
		return DELETEKEY;
	case 36:
		return HOME;
	case 35:
		return END;
	case 33:
		return PAGE_UP;
	case 34:
		return PAGE_DOWN;
	case 37:
		return LEFT;
	case 38:
		return UP;
	case 39:
		return RIGHT;
	case 40:
		return DOWN;
	case 112:
		return F1;
	case 113:
		return F2;
	case 114:
		return F3;
	case 115:
		return F4;
	case 116:
		return F5;
	case 117:
		return F6;
	case 118:
		return F7;
	case 119:
		return F8;
	case 120:
		return F9;
	case 121:
		return F10;
	case 122:
		return F11;
	case 123:
		return F12;
	case 16:
		return SHIFT_LEFT;
	case 17:
		return CONTROL_LEFT;
	}
	return code;
}