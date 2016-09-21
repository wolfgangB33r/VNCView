//  Copyright (C) 2014 Wolfgang Beer. All Rights Reserved.
//  email wolfgang@pocketvnc.com
//  This file is part of the MobileVNC viewer software.
//
#ifndef KEYCODES__H
#define KEYCODES__H

#define BACKSPACE		0xff08
#define TAB				0xff09
#define ENTER			0xff0d
#define ESCAPE			0xff1b
#define INSERT			0xff63
#define DELETEKEY		0xffff
#define HOME			0xff50
#define END				0xff57
#define PAGE_UP			0xff55
#define PAGE_DOWN		0xff56
#define LEFT			0xff51
#define UP				0xff52
#define RIGHT			0xff53
#define DOWN			0xff54
#define F1				0xffbe
#define F2				0xffbf
#define F3				0xffc0
#define F4				0xffc1
#define F5				0xffc2
#define F6				0xffc3
#define F7				0xffc4
#define F8				0xffc5
#define F9				0xffc6
#define F10				0xffc7
#define F11				0xffc8
#define F12				0xffc9
#define SHIFT_LEFT		0xffe1
#define SHIFT_RIGHT		0xffe2
#define CONTROL_LEFT	0xffe3
#define CONTROL_RIGHT	0xffe4
#define META_LEFT		0xffe7
#define META_RIGHT		0xffe8
#define ALT_LEFT		0xffe9
#define ALT_RIGHT		0xffea


/**
  * Translates the platform specific char/keycode into the VNC defined codes.
  */
int vncKeyCode(int code);

#endif KEYCODES__H
