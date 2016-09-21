//  Copyright (C) 2014 Wolfgang Beer. All Rights Reserved.
//  email wolfgang@pocketvnc.com
//  This file is part of the MobileVNC viewer software.
//
#include "mutex.h"

HANDLE hMutex; 

bool getMutex(void)
{
	hMutex = CreateMutex (NULL, FALSE, NULL);
	if (NULL == hMutex)
	{
		return false;
	}
	return true;
}

bool freeMutex()
{
	ReleaseMutex(hMutex);
	return true;
}
