// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <stdio.h>
#include <stdint.h>

#pragma pack(1)

struct net_header
{
	uint32_t length;
	uint32_t timestamp;
	uint16_t idenfitier;
};

struct net_destroy_packet
{
	net_header hdr;
	uint8_t zero;
	uint8_t serial;
	uint8_t index[2];
	uint32_t check1;
	uint32_t check2;
};

#pragma pack()

// TODO: reference additional headers your program requires here
