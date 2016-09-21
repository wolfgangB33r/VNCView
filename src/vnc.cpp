//  Copyright (C) 2014 Wolfgang Beer. All Rights Reserved.
//  email wolfgang@pocketvnc.com
//  This file is part of the MobileVNC viewer software.
//
#include "vnc.h"

int protocol;		// the negotiated vnc protocol used for communication
char* password;		// vnc password
int width;			// framebuffer width
int height;			// framebuffer height
char* name;			// server name
PIXELFORMAT format;

CONF conf;

// actual framebuffer decoding state
int encoding;
int pixCounter;
int maxPix;
int x;
int y;
int rectX;
int rectWidth;
int rectHeight;

// actual hextile decoding state
int tileOffs; 
int tileWidth;
int tileHeight;
COLOR tile[16][16];

void encrypt(unsigned char* plain, char* pwd) 
{
	unsigned char p[8];
	int i;
	// create the key for the DES
	for(i = 0; i < 8; i++) 
	{
		if(i < (int)strlen(pwd)) 
		{
			p[i] = pwd[i];
		}
		else 
		{
			p[i] = 0;
		}
	}
	// set the key
	deskey(p, EN0);
	// encode the plain challenge
    for (i = 0; i < 16; i += 8) 
	{
		des(plain+i, plain+i);
    }
}

bool security() 
{
	if(protocol < VNC_PROTOCOL_VERSION_003_007) 
	{
		U32 sec;
		char* reasonStr;
		if(!receiveU32(&sec))
		{
			return false;
		}
		printf(".server sent security type: %d\n", sec);
		switch(sec) 
		{
		case SECURITY_TYPE_INVALID:
			// failed according to a specific reason
			U32 len;
			if(!receiveU32(&len)) 
			{
				return false;
			}
			reasonStr = (char*)malloc(len);
			receive(len, reasonStr);
			printf(".invalid security type: %s\n", reasonStr);
			free(reasonStr);
			return false;
			break;
		case SECURITY_TYPE_NONE:
			// nothing to do
			return true;
			break;
		case SECURITY_TYPE_PWD:
			// read 16byte random challenge
			char challenge[16];
			if(!receive(16, challenge)) 
			{
				return false;
			}
			// DES encrypt challenge with user pwd 
			encrypt((unsigned char*)challenge, password);
			// send the encrypted response
			send(16, challenge);
			// read security result message
			U32 secRes;
			if(!receiveU32(&secRes)) 
			{
				return false;
			}
			if(secRes == 0) 
			{
				printf(".login ok\n");
				return true;
			}
			return false;
			break;
		default:
			return false;
		}
	}
	else 
	{
		// todo
		return false;
	}
	return true;
}

bool readServerInit() 
{
	U8 v8;
	U16 v16;
	U32 v32;
	if(!receiveU16(&v16))
	{
		return false;
	}
	width = v16;
	if(!receiveU16(&v16))
	{
		return false;
	}
	height = v16;
	if(!receiveU8(&v8))
	{
		return false;
	}
	format.bits_per_pixel = v8;
	if(!receiveU8(&v8))
	{
		return false;
	}
	format.depth = v8;
	if(!receiveU8(&v8))
	{
		return false;
	}
	format.big_endian_flag = v8;
	if(!receiveU8(&v8))
	{
		return false;
	}
	format.true_colour_flag = v8;
	if(!receiveU16(&v16))
	{
		return false;
	}
	format.red_max = v16;
	if(!receiveU16(&v16))
	{
		return false;
	}
	format.green_max = v16;
	if(!receiveU16(&v16))
	{
		return false;
	}
	format.blue_max = v16;
	if(!receiveU8(&v8))
	{
		return false;
	}
	format.red_shift = v8;
	if(!receiveU8(&v8))
	{
		return false;
	}
	format.green_shift = v8;
	if(!receiveU8(&v8))
	{
		return false;
	}
	format.blue_shift = v8;
	// read padding
	if(!receive(3, (char*)&v32))
	{
		return false;
	}
	// read name length
	if(!receiveU32(&v32))
	{
		return false;
	}
	name = (char*)malloc(v32 + 1);
	if(!receive(v32, name)) 
	{
		return false;
	}
	name[v32] = '\0';
	printf(".server init msg was read W:%d H:%d bpp:%d %s\n", width, height, format.bits_per_pixel, name);
	return true;
}

int getScreenWidth()
{
	return width;
}

int getScreenHeight()
{
	return height;
}

char* getName()
{
	return name;
}

bool openSession(char* pwd, char* repeaterID)
{
	password = pwd;
	// negotiate vnc protocol version to use 
	char rBuf[12];
	if(!receive(12, rBuf))
	{
		return false;
	}
	printf(".server sent vnc version: %s\n", rBuf);
	switch(rBuf[10]) 
	{
	case '3':
		protocol = VNC_PROTOCOL_VERSION_003_003;
		break;
	case '4':
		protocol = VNC_PROTOCOL_VERSION_003_003;
		break;
	case '6':
		protocol = VNC_PROTOCOL_VERSION_003_003;
		break;
	case '7':
		protocol = VNC_PROTOCOL_VERSION_003_003;
		break;
	case '8':
		protocol = VNC_PROTOCOL_VERSION_003_003;
		break;
	default:
		protocol = VNC_PROTOCOL_VERSION_003_003;
		break;
	}
	// at the moment we just support 3.3
	if(!send(12, "RFB 003.003\0")) 
	{
		return false;
	}
	if(!security())
	{
		return false;
	}
	// send client initialization msg
	sendU8(1);
	// finally read the server init msg
	if(!readServerInit())
	{
		return false;
	}
	// send pixel format message
	PIXELFORMAT f;
	if(conf.bpp == 8)
	{
		f.bits_per_pixel = 8;	
		f.depth = 8;				
		f.big_endian_flag = 0;
		f.true_colour_flag = 1;
		f.red_max = 7;
		f.green_max = 7;
		f.blue_max = 3;
		f.red_shift = 0;
		f.green_shift = 3;
		f.blue_shift = 6;
		sendSetPixelFormat(f);
	}
	else if (conf.bpp == 16)
	{
		f.bits_per_pixel = 16;	
		f.depth = 16;				
		f.big_endian_flag = 0;
		f.true_colour_flag = 1;
		f.red_max = 31;
		f.green_max = 63;
		f.blue_max = 31;
		f.red_shift = 11;
		f.green_shift = 5;
		f.blue_shift = 0;
		sendSetPixelFormat(f);
	}
	else if(conf.bpp == 32)
	{
		f.bits_per_pixel = 32;	
		f.depth = 24;				
		f.big_endian_flag = 0;
		f.true_colour_flag = 1;
		f.red_max = 255;
		f.green_max = 255;
		f.blue_max = 255;
		f.red_shift = 16;
		f.green_shift = 8;
		f.blue_shift = 0;
		sendSetPixelFormat(f);
	}
	else 
	{
		f.bits_per_pixel = 16;	
		f.depth = 16;				
		f.big_endian_flag = 0;
		f.true_colour_flag = 1;
		f.red_max = 31;
		f.green_max = 63;
		f.blue_max = 31;
		f.red_shift = 11;
		f.green_shift = 5;
		f.blue_shift = 0;
		sendSetPixelFormat(f);
	}
}

// Examples for valid Pixel Formats
// taken from UltraVNC Viewer:
// PixelFormatMsg trueCol=1 bPP8 d8 rM7 gM7 bM3 rS0 gS3 bS6
// PixelFormatMsg trueCol=1 bPP16 d16 rM31 gM63 bM31 rS11 gS5 bS0
//

bool sendSetPixelFormat(PIXELFORMAT f)
{
	format = f; // set new pixel format
	while(!getMutex()) {}
	unsigned char buf[20];
	*buf = SET_PIXEL_FORMAT;		// send message type (value:0)	
	// 3 byte padding
	// start sending pixel format
	*(buf+4) = f.bits_per_pixel;
	*(buf+5) = f.depth;
	*(buf+6) = f.big_endian_flag;
	*(buf+7) = f.true_colour_flag;
	*(short *) (buf+8) = (u_short)htons(f.red_max);
	*(short *) (buf+10) = (u_short)htons(f.green_max);
	*(short *) (buf+12) = (u_short)htons(f.blue_max);
	*(buf+14) = f.red_shift;
	*(buf+15) = f.green_shift;
	*(buf+16) = f.blue_shift;
	// last 3 bytes just padding
	send(20, (const char*)buf);
	freeMutex();
	return true;
}

bool sendSetEncodings(int nr_enc_types, int enc_types[])
{
	while(!getMutex()) {}
	unsigned char* dyn_buf = (unsigned char*)malloc(4 + nr_enc_types * 4);
	*dyn_buf = SET_ENCODINGS;		// send message type (value:2)			
	// second byte is just padding
	*(short *) (dyn_buf+2) = (u_short)htons(nr_enc_types); // number of encodings
	for(int i = 0; i < nr_enc_types; i++)
	{
		*(u_long *) (dyn_buf+4+(4*i)) = (u_long)htonl(enc_types[i]); 
	}
	send(4 + nr_enc_types * 4, (const char*)dyn_buf);
	free(dyn_buf);
	freeMutex();
	return true;
}

bool sendFramebufferUpdateRequest(int inc, int x, int y, int width, int height)
{
	while(!getMutex()) {}
	unsigned char buf[10];
	*buf = FRAME_BUFFER_UPDATE_REQUEST;		// send message type (value:3)
	*(buf+1) = inc;	
	*(short *) (buf+2) = (u_short)htons(x);
	*(short *) (buf+4) = (u_short)htons(y);
	*(short *) (buf+6) = (u_short)htons(width);
	*(short *) (buf+8) = (u_short)htons(height);
	send(10, (const char*)buf);
	freeMutex();
	return true;
}

bool sendKeyEvent(int down, int key)
{
	while(!getMutex()) {}
	unsigned char buf[8];
	*buf = VNC_KEY_EVENT;	// send message type (value:4)
	*(buf+1) = down;	
	// data between 2 and 4 byte is padding
	*(u_long *) (buf+4) = (u_long)htonl(key);
	send(8, (const char*)buf);
	freeMutex();
	return true;
}

bool sendPointerEvent(char mask, int x, int y)
{
	while(!getMutex()) {}
	unsigned char buf[6];
	*buf = POINTER_EVENT;	
	*(buf+1) = mask;
	*(short *) (buf+2) = (u_short)htons(x);
	*(short *) (buf+4) = (u_short)htons(y);
	send(6, (const char*)buf);
	freeMutex();
	return true;
}

bool sendClientCutText(int len, char* text)
{
	while(!getMutex()) {}
	unsigned char* dyn_buf = (unsigned char*)malloc(8 + len);
	*dyn_buf = CLIENT_CUT_TEXT;	// send message type (value:6)
	// 3 bytes padding
	*(u_long *) (dyn_buf+4) = (u_long)htonl(len);
	for(int i = 0; i < len; i++)
	{
		*(dyn_buf+8+i) = text[i];
	}
	send(len, (const char*)dyn_buf);
	free(dyn_buf);
	freeMutex();
	return true;
}

bool receiveNextServerMsgType(int* msg)
{
	U8 msgType;
	bool ret = receiveU8(&msgType);
	*msg = msgType;
	return ret;
}

bool receiveSetColourMapEntries()
{
	U16 r, g, b;
	U8 padding;
	U16 fc;
	U16 c;
	if(!receiveU8(&padding)) 
	{
		return false;
	}
	if(!receiveU16(&fc)) 
	{
		return false;
	}
	if(!receiveU16(&c)) 
	{
		return false;
	}
	for(int i = 0; i < c; i++) 
	{
		if(!receiveU16(&r) || !receiveU16(&g) || !receiveU16(&b)) 
		{
			return false;
		}
	}
	return true;
}

bool receiveServerCutText(int* len, char* text)
{
	char padding[3];
	if(!receive(3, padding))
	{
		return false;
	}
	U32 l;
	if(!receiveU32(&l))
	{
		return false;
	}
	*len = l;
	text = (char*) malloc(l);
	if(!receive(l, text))
	{
		free(text);
		return false;
	}
	return true;
}

bool receiveFrameBufferUpdate(int* nrRec)
{
	U8 padding;
	if(!receiveU8(&padding)) 
	{
		return false;
	}
	U16 nrOfRects;
	if(!receiveU16(&nrOfRects)) 
	{
		return false;
	}
	*nrRec = nrOfRects;
	return true;
}

bool receiveNextRect(FBRECT* rect)
{
	U16 rx, ry, rw, rh;
	U32 enc;
	if(!receiveU16(&rx) || !receiveU16(&ry) || !receiveU16(&rw) || !receiveU16(&rh) || !receiveU32(&enc)) 
	{
		return false;
	}
	//printf(".rect x:%d y:%d w:%d h:%d enc:%d\n", rx, ry, rw, rh, enc);
	rect->x = rx;
	rect->y = ry;
	rect->width = rw;
	rect->height = rh;
	// initialize pixel iterator state
	encoding = enc;
	pixCounter = 0;
	rectX = rx;
	rectWidth = rw;
	rectHeight = rh;
	x = rx;
	y = ry;
	maxPix = rw * rh;
	return true;
}

bool readColor(COLOR* col)
{
	if(format.bits_per_pixel == 8) 
	{
		char p;
		unsigned char buf[1];
		if(!receive(1, (char*)buf)) 
		{
			return false;
		}
		memcpy(&p, buf, sizeof(U8));
		col->r = (p >> format.blue_shift) & format.red_max;
		col->g = (p >> format.green_shift) & format.green_max;
		col->b = (p >> format.red_shift) & format.blue_max; 
		col->r = 256 / format.red_max * col->r;
		col->g = 256 / format.green_max * col->g;
		col->b = 256 / format.blue_max * col->b;
	}
	else if(format.bits_per_pixel == 16)
	{
		u_short p;
		unsigned char buf[2];
		if(!receive(2, (char*)buf)) 
		{
			return false;
		}
		memcpy(&p, buf, sizeof(U16));
		col->r = (p >> format.blue_shift) & format.red_max;
		col->g = (p >> format.green_shift) & format.green_max;
		col->b = (p >> format.red_shift) & format.blue_max; 
		col->r = 255 / format.red_max * col->r;
		col->g = 255 / format.green_max * col->g;
		col->b = 255 / format.blue_max * col->b;
	}
	else if(format.bits_per_pixel == 24)
	{
		u_long p;
		unsigned char buf[3];
		if(!receive(3, (char*)buf)) 
		{
			return false;
		}
		memcpy(&p, buf, 3);
		col->r = (p >> format.blue_shift) & format.red_max;
		col->g = (p >> format.green_shift) & format.green_max;
		col->b = (p >> format.red_shift) & format.blue_max; 
	}
	else if(format.bits_per_pixel == 32)
	{
		u_long p;
		unsigned char buf[4];
		if(!receive(4, (char*)buf)) 
		{
			return false;
		}
		memcpy(&p, buf, sizeof(U32));
		col->r = (p >> format.blue_shift) & format.red_max;
		col->g = (p >> format.green_shift) & format.green_max;
		col->b = (p >> format.red_shift) & format.blue_max; 
	}
	return false;
}

bool receivePixelFromRaw(PIXEL* pixel) 
{
	if(pixCounter >= maxPix) // frame rect has no more pixel
	{
		return false;
	}
	pixel->seq = pixCounter;
	pixel->x = x;
	pixel->y = y;
	COLOR col;
	readColor(&col);
	pixel->c = col;
	// set index to next pixel
	pixCounter++;
	x++;
	if(x >= rectWidth) 
	{
		x = rectX;
		y++;
	}
	return true;
}

int tileNr = 0;
COLOR fg, bg;

void fetchHextile() 
{
	//printf("tileNr:%d %d %d\n", tileNr, x, y);
	tileWidth = 16; 
	tileHeight = 16;
	if((x + 16) >= rectWidth) 
	{
		tileWidth = rectWidth - x; // if rectWidth is not a multiple of 16
	}
	if((y + 16) >= rectHeight) 
	{
		tileHeight = rectHeight - y; // if rectHeight is not a multiple of 16
	}
	tileOffs = 0;
	// read subencoding type byte and decode tile colors into tile array
	U8 subMask;
	receiveU8(&subMask);
	if((subMask & 1) == 1) // RAW tile, ignore all other bits in bitmask! 
	{
		// read raw tile
		for(int x=0; x < tileWidth; x++) 
		{
			for(int y=0; y < tileHeight; y++) 
			{
				readColor(&tile[x][y]);
			}
		}
	}
	else
	{
		bool subRecColored = false;
		COLOR sc;
		U8 nrSubRecs = 0;
		if((subMask & 2) == 2) // BackgroundSpecified
		{
			readColor(&bg);
		}
		if((subMask & 4) == 4) // ForegroundSpecified
		{
			readColor(&fg);
		}
		if((subMask & 8) == 8) // AnySubrects
		{
			receiveU8(&nrSubRecs);
		}
		if((subMask & 16) == 16) // SubrectsColored 
		{
			subRecColored = true;
		}
		// clear tile with bg color
		for(int x=0; x < tileWidth; x++) 
		{
			for(int y=0; y < tileHeight; y++) 
			{
				memcpy(&tile[x][y], &bg, sizeof(COLOR));
			}
		}
		// now read all the subrects
		for(int i = 0; i < nrSubRecs; i++) 
		{
			if(subRecColored)
			{
				readColor(&sc);
			}
			U8 a;
			U8 b;
			receiveU8(&a); // x and y
			int sx = (a >> 4) & 0x0F;
			int sy = a & 0x0F;
			receiveU8(&b); // width and height
			int sw = ((b >> 4) & 0x0F) + 1;
			int sh = (b & 0x0F) + 1;
			// now color the subrect with the foreground color
			for(int tx = sx; tx < sx + sw; tx++)
			{
				for(int ty = sy; ty < sy + sh; ty++)
				{
					if(subRecColored)
					{
						memcpy(&tile[tx][ty], &sc, sizeof(COLOR));
					}
					else 
					{
						memcpy(&tile[tx][ty], &fg, sizeof(COLOR));
					}
				}
			}
		}
	}
	tileNr++;
}

bool receivePixelFromHextile(PIXEL* pixel) 
{
	if(pixCounter == 0) 
	{
		tileOffs = 0;
		tileNr = 0;
		fetchHextile();
	}
	if(tileOffs >= tileWidth * tileHeight) 
	{
		x += 16;
		if(x >= rectWidth)
		{
			y += 16;
			x = rectX;
		}
		fetchHextile();
	}
	// 
	int tileX = tileOffs % tileWidth;
	int tileY = tileOffs / tileWidth;
	pixel->c = tile[tileX][tileY];
	pixel->x = x + tileX;
	pixel->y = y + tileY;
	pixel->seq = pixCounter;
	pixCounter++;
	tileOffs++;
	return true;
}

bool receiveNextPixel(PIXEL* pixel)
{
	switch(encoding) 
	{
	case RAW:
		receivePixelFromRaw(pixel);
		break;
	case HEXTILE:
		receivePixelFromHextile(pixel);
		break;
	}
	return true;
}
