//  Copyright (C) 2014 Wolfgang Beer. All Rights Reserved.
//  email wolfgang@pocketvnc.com
//  This file is part of the MobileVNC viewer software.
//
#ifndef MUTEX__H
#define MUTEX__H

#include <windows.h>

/**
  * Request the sending Mutex.
  */
bool getMutex(void);

/**
  * Release the sending Mutex.
  */ 
bool freeMutex(void);

#endif MUTEX__H