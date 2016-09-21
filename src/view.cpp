//  Copyright (C) 2014 Wolfgang Beer. All Rights Reserved.
//  email wolfgang@pocketvnc.com
//  This file is part of the MobileVNC viewer software.
//

#include "view.h"
#include "./win/conmgr.h"

void usage(void)
{
	printf("view [options] host:display\n");
	printf("options:\n");
	printf(" -F Fullscreen mode\n");
	printf(" -R Reconnect\n");
	exit (8);
}

void setPassword(char* pwd)
{
	conf.pwd = pwd;
}

void setColorDepth(int bpp)
{
	conf.bpp = bpp;
}

void setFullscreen(bool f)
{
	conf.fullscreen = f;
}

void setScrollbars(bool s)
{
	conf.scrollbars = s;
}

void setAutoReconnect(bool r)
{
	conf.reconnect = r;
}

bool isAutoReconnect() 
{
	return conf.reconnect;
}

// Parses the given address string and sets address and port number
// 
bool getServerAdr(char* str) 
{
	conf.adr = str;
	while(str[0] != ':' && str[0] != '\0') 
	{
		str++;
	}
	if(str[0] == ':') 
	{
		str[0] = '\0';
		str++;
		conf.port = atoi(str) + 5900;
	}
	else 
	{
		printf(".no display given, using the default vnc port 5900\n");
		conf.port = 5900;
	}
	printf(".address %s Port:%d\n", conf.adr, conf.port);
	return true;
}

void rememberServer(char* adr)
{
	// first check if adr is already stored in file
	FILE* fp = fopen("servers.txt", "r+");
	if(fp != NULL)
	{
		char s[500];
		while ( fgets (s , 500 , fp) != NULL )
		{
			if(0 == strcmp(s, adr))
			{
				return;
			}
		}
		fclose(fp);
	}
	fp = fopen("servers.txt", "a");
	if(fp != NULL)
	{
		fprintf(fp, "%s\n", adr);
		fclose(fp);
	}
}

void connect()
{
	// remember server address
		rememberServer(conf.adr);
	if(init()) 
	{
		if(!openConnection(conf.adr, conf.port, conf.reconnect))
		{
			return;
		}
		if(!openSession(conf.pwd))
		{
			return;
		}
		
		// now show screen
		showWindow(conf.fullscreen);
	}
}

int main(int argc, char *argv[])
{
	int i = 0;
	printf("MobileVNC View\n");
	while ((argc > 1) && (argv[1][0] == '-'))
	{
		switch (argv[1][1])
		{
			case 'F':
				printf(".fullscreen mode enabled\n");
				conf.fullscreen = true;
				break;
			case 'R':
				printf(".automatic reconnect enabled\n");
				conf.reconnect = true;
				break;
			case 'P': 
				printf(".password: %s\n", argv[1]);
				conf.pwd = &argv[1][2];
				break;
			default:
				printf("Wrong Argument: %s\n", argv[1]);
				usage();
		}
		++argv;
		--argc;
	}
	// either we already have a cmd line address argument
	if(argc > 1 && getServerAdr(argv[1])) 
	{
		connect();
	}
	else // or we open the user dialog
	{
		if(showOptionsDialog()) 
		{
			// print user options on the console
			printf("server: %s:%d\n", conf.adr, conf.port);
			connect();
		}
		return (0);
	}


	return (0);
}

