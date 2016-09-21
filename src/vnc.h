//  Copyright (C) 2014 Wolfgang Beer. All Rights Reserved.
//  email wolfgang@pocketvnc.com
//  This file is part of the MobileVNC viewer software.
//
#ifndef VNC__H
#define VNC__H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern "C" {
#include "d3des.h"
}

#include "./win/mutex.h"
#include "./win/conmgr.h"

#define VNC_PROTOCOL_VERSION_003_003	0  // Original RFB Version 3.3
#define VNC_PROTOCOL_VERSION_003_004	1  // UltraVNC RFB Version 3.4
#define VNC_PROTOCOL_VERSION_003_006	2	// UltraVNC RFB Version 3.6
#define VNC_PROTOCOL_VERSION_003_007	3	// RFB Version 3.7
#define VNC_PROTOCOL_VERSION_003_008	4	// RFB Version 3.8

#define SECURITY_TYPE_INVALID			0
#define SECURITY_TYPE_NONE				1
#define SECURITY_TYPE_PWD				2

#define RAW								0
#define HEXTILE							5

#define LEFT_MOUSE_DOWN					0x01
#define LEFT_MOUSE_UP					0xFE
#define MIDDLE_MOUSE_DOWN				0x02
#define MIDDLE_MOUSE_UP					0xFD
#define RIGHT_MOUSE_DOWN				0x04
#define RIGHT_MOUSE_UP					0xFB

#define KEY_DOWN						1
#define KEY_UP							0

// Client to server message types
#define SET_PIXEL_FORMAT				0
#define SET_ENCODINGS					2
#define FRAME_BUFFER_UPDATE_REQUEST		3
#define VNC_KEY_EVENT					4
#define POINTER_EVENT					5
#define CLIENT_CUT_TEXT					6

// Server to client message types
#define FRAMEBUFFER_UPDATE				0 
#define SET_COLORMAP_ENTRIES			1 
#define BELL							2
#define SERVER_CUT_TEXT					3

// the general config information for connecting to a VNC server
struct CONF {
	char* adr;			// IP server address 
	int port;			// display number + 5900
	bool fullscreen;	// fullscreen mode enabled
	bool scrollbars;	// show scollbars enabled
	bool reconnect;		// automatic reconnect after connection loss enabled
	int bpp;			// Color depth in bit per pixel, 8,16,32
	char* pwd;			// password
};

extern CONF conf;

struct COLOR
{
	int r;
	int g;
	int b;
};

struct PIXEL
{
	int seq; // the pixel sequence/counter number within a given rectangle
	int x;
	int y;
	COLOR c;
};

struct FBRECT
{
	int x;
	int y;
	int width;
	int height;
};

struct PIXELFORMAT
{
	U8 bits_per_pixel;
	U8 depth;
	U8 big_endian_flag;
	U8 true_colour_flag;
	U16 red_max;
	U16 green_max;
	U16 blue_max;
	U8 red_shift;
	U8 green_shift;
	U8 blue_shift;
	U8 padding1;
	U8 padding2;
	U8 padding3;
};

/**
 * Opens and initializes a VNC client session, by using the already opened connection manager with 
 * a given username and password and an optional repeaterID.
 *@returns true if init of session was successful, false otherwise
 */
bool openSession(char* pwd, char* repeaterID = 0);

/**
  * Sends the format in which pixel values should be sent in FramebufferUpdate messages.
  * If the client does not send a SetPixelFormat message then the server sends pixel values
  * in its natural format as speci?ed in the ServerInit message.
  */
bool sendSetPixelFormat(PIXELFORMAT f);

/**
  * Sets the encoding types in which pixel data can be sent by the server. The order of the
  * encoding types given in this message is a hint by the client as to its preference (the ?rst
  * encoding speci?ed being most preferred). The server may or may not choose to make
  * use of this hint. Pixel data may always be sent in raw encoding even if not speci?ed
  * explicitly here.
  */
bool sendSetEncodings(int nr_enc_types, int enc_types[]);

/**
  * Noti?es the server that the client is interested in the area of the framebuffer speci?ed
  * by x-position, y-position, width and height.
  */
bool sendFramebufferUpdateRequest(int inc, int x, int y, int width, int height);

/**
  * A key press or release. Down-?ag is non-zero (true) if the key is now pressed, zero
  * (false) if it is now released. The key itself is speci?ed using the keysym values
  * de?ned by the X Window System. 
  */
bool sendKeyEvent(int down, int key);

/** Indicates either pointer movement or a pointer button press or release. The pointer is
  * now at (x-position, y-position), and the current state of buttons 1 to 8 are represented
  * by bits 0 to 7 of button-mask respectively, 0 meaning up, 1 meaning down (pressed).
  * On a conventional mouse, buttons 1, 2 and 3 correspond to the left, middle and right
  * buttons on the mouse. On a wheel mouse, each step of the wheel upwards is represented by 
  * a press and release of button 4, and each step downwards is represented by a 
  * press and release of button 5.
  */
bool sendPointerEvent(char mask, int x, int y);

/**
  * The client has new ISO 8859-1 (Latin-1) text in its cut buffer. Ends of lines are repre
  * sented by the linefeed / newline character (value 10) alone. No carriage-return (value
  * 13) is needed. There is currently no way to transfer text outside the Latin-1 character set.
  */
bool sendClientCutText(int len, char* text);

/**
  * Returns the width of the screen that was defined by the server init message.
  */
int getScreenWidth();

/**
  * Returns the height of the screen that was defined by the server init message.
  */
int getScreenHeight();

/**
  * Returns the name of the connected server.
  */
char* getName();

/**
  * Receives the next message type the server sent back to the client.
  */
bool receiveNextServerMsgType(int* msg);

/**
  * Receives the color map entry message.
  */
bool receiveSetColourMapEntries();

/**
  * Receives server cut text message.
  */
bool receiveServerCutText(int* len, char* text);

/*
 * Receives the incoming framebuffer update.
 * This function is the first stept to handle the incoming framebuffer.
 * The sequence is:
 * receiveFrameBufferUpdate(nrRec)
 * forall nrRec
 *	 receiveNextRect(rect)
 *	   forall pixels
 *	     receiveNextPixel(pixel)
 *
 *
 * This iterative approach avoids to allocate a huge amount of memory in order to decode the
 * pixel data at once.
 */
bool receiveFrameBufferUpdate(int* nrRec);

bool receiveNextRect(FBRECT* rect);

bool receiveNextPixel(PIXEL* pixel);


#endif VNC__H