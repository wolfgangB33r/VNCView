//  Copyright (C) 2014 Wolfgang Beer. All Rights Reserved.
//  email wolfgang@pocketvnc.com
//  This file is part of the MobileVNC viewer software.
//
#include "conmgr.h"

SOCKET oSock;

bool init() 
{
	// Initialize WinSock
	WSADATA wsaData;
	if(WSAStartup(MAKEWORD(1,1), &wsaData) != 0) 
	{
		printf(".!conMgr init failed\n");
		return false;
	}
	return true;
}

bool openConnection(char* adr, int port, bool reconnect) 
{
	SOCKADDR_IN dest_sin;        // Server socket address
	struct in_addr addr;

	printf(".opening connection\n");
	
	if(isalpha(adr[0])) // hostname is a URL
	{ 
		printf(".address is a domain address\n");
		hostent* hInfo; 
		hInfo = gethostbyname(adr); 
		if(hInfo == NULL)
		{
			printf(".failed to get ip from hostname\n");
			return false;
		}
		addr.s_addr = *(u_long *) hInfo->h_addr_list[0];
	}
	else // host is an IP
	{ 
		printf(".address is an IP address\n");
		addr.s_addr = inet_addr (adr);
	}

	// Create a TCP/IP socket that is bound to the server.
	oSock = socket(AF_INET, SOCK_STREAM, 0);
	if (oSock == INVALID_SOCKET)
	{
		printf(".failed to create an outgoing socket\n");
		return false;
	}
	dest_sin.sin_family = AF_INET;
	dest_sin.sin_port = htons (port);  
	dest_sin.sin_addr.s_addr = addr.s_addr ;
	
	if (connect (oSock, (PSOCKADDR) &dest_sin, sizeof (dest_sin)) == SOCKET_ERROR) 
	{
		shutdown(oSock, 2);
		closesocket (oSock);
		printf(".failed to connect the outgoing socket\n");
		return false;
	}
	return true;
}

bool receive(int len, char *data) 
{
	int read = 0;
	int rec = recv(oSock, data + read, len - read, 0);
	if(rec != SOCKET_ERROR && rec != 0) 
	{
		read += rec;
	}
	while(read < len && rec != 0 && rec != SOCKET_ERROR)
	{
		rec = recv(oSock, data+read, len - read, 0);
		if(rec != SOCKET_ERROR && rec != 0) 
		{
			read += rec;
		}
	}
	if(len != read) 
	{
		// any error occured
		int err = WSAGetLastError();
		printf(".read from socket failed %d\n", err);
		return false;
	}
	return true;
}

bool receiveU8(U8 *val)
{
	unsigned char buf[1];
	if(!receive(1, (char*)buf))
	{
		return false;
	}
	*val = (U8)buf[0];
	return true;
}

bool receiveU16(U16 *val)
{
	unsigned char buf[2];
	if(!receive(2, (char*)buf))
	{
		return false;
	}
	U16 netShort;
	memcpy(&netShort, buf, sizeof(U16));
	*val = ntohs(netShort);
	return true;
}

bool receiveU32(U32 *val)
{
	unsigned char buf[4];
	if(!receive(4, (char*)buf)) 
	{
		return false;
	}
	U32 netLong;
	memcpy(&netLong, buf, sizeof(U32));
	U32 hLong = ntohl(netLong);
	*val = hLong;
	return true;
}

bool send(int len, const char *data)
{
	if(len != send(oSock, data, len, 0)) 
	{
		// any error occured
		int err = WSAGetLastError();
		printf(".send to socket failed %d\n", err);
		return false;
	}
	return true;
}

bool sendU32(U32 val) 
{
	U32 net = htonl(val);
	int sent = send(sizeof(U32), (const char*)&net);

	return sent == sizeof(U32);
}

bool sendU16(U16 val) 
{
	U16 net = htons(val);
	int sent = send(sizeof(U16), (const char*)&net);

	return sent == sizeof(U16);
}

bool sendU8(U8 val) 
{
	int sent = send(sizeof(U8), (const char*)&val);
	return sent == sizeof(U8);
}

bool closeConnection()
{
	return true;
}

bool cleanup() 
{
	shutdown(oSock, 2);
	closesocket(oSock);

	// Clean up Winsock
	if(WSACleanup() == SOCKET_ERROR) 
	{
		printf(".!could not cleanup sockets");
		return false;
	}
	return true;
}