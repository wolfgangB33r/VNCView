//  Copyright (C) 2014 Wolfgang Beer. All Rights Reserved.
//  email wolfgang@pocketvnc.com
//  This file is part of the MobileVNC viewer software.
//
#ifndef CONMGR__H
#define CONMGR__H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock.h>

typedef unsigned long					U32;
typedef unsigned short					U16;
typedef unsigned char					U8;
typedef short							S16;

/**
 * Initializes the Winsock stack
 *@returns true if stack was initialized, false otherwise
 */
bool init();

/**
 * Opens a TCP socket connection to the given server address and port
 *@returns true if connection is open, false otherwise
 */
bool openConnection(char* adr, int port, bool reconnect);

/**
 * Receive len number of bytes payload data from the opened socket.
 *@returns true if bytes could be read, false otherwise
 */
bool receive(int len, char *data);

/**
  * Receives an U8 from the socket.
  */
bool receiveU8(U8 *val);

/**
  * Receives an U16 from the socket and converts it to host byte order.
  */
bool receiveU16(U16 *val);

/**
  * Receives an U32 from the socket and converts it to host byte order.
  */
bool receiveU32(U32 *val);

/**
  * Converts an U32 value to network byte order and sends it. 
  *
  * /return send succeeded
  */
bool sendU32(U32 val);

/**
  * Converts an U16 value to network byte order and sends it. 
  *
  * /return send succeeded
  */
bool sendU16(U16 val);

/**
  * Sends an U8 value. 
  *
  * /return send succeeded
  */
bool sendU8(U8 val); 

/**
 * Send len number of bytes payload data to the opened socket.
 *@returns true if bytes could be sent, false otherwise
 */
bool send(int len, const char *data);

/**
 * Close the socket.
 *@returns true if socket was closed, false otherwise
 */
bool closeConnection();

/**
 * Closes the socket and do all the cleanup work
 *@returns true if cleanup was successful, false otherwise
 */
bool cleanup();

#endif CONMGR__H