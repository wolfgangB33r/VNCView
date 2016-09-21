//  Copyright (C) 2014 Wolfgang Beer. All Rights Reserved.
//  email wolfgang@pocketvnc.com
//  This file is part of the MobileVNC viewer software.
//
#ifndef VIEW__H
#define VIEW__H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vnc.h"

#include "./win/ui.h"
#include "./win/dialog.h"

bool getServerAdr(char* str);

void setPassword(char* pwd);

void setColorDepth(int bpp);

void setFullscreen(bool f);

void setScrollbars(bool s);

void setAutoReconnect(bool r);

#endif VIEW__H