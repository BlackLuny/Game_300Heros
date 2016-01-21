#include "pch.h"
#include "CSigMngr.h"
#include "GameCamera.h"

#define CAMERA_A_SIG "\x8B\x80\x54\xBF\x02\x00\x52\x51\x8D\x54\x24\x20\x52\x8D\x4C\x24\x2C\x51\x8D\x94\x24\x84\x00\x00\x00\x52\x8B\xC8"
#define CAMERA_A_SIG_BYTES sizeof(CAMERA_A_SIG)-1

GameCamera::GameCamera(void)
{
	
}


GameCamera::~GameCamera(void)
{
}

void GameCamera::Attach()
{
	unsigned char *Camera_A;
	unsigned char *Camera_B;
	unsigned char *Camera_C;
	DWORD Ptr = 0;


	Camera_A = (unsigned char*)FIND_MEMORY(GAME_BASE_ADDRESS,CAMERA_A_SIG);

	if(Camera_A)
	{
		Camera_B = Camera_A - 0xAC;
		if(*Camera_B == 0xE8){
			Camera_C = Camera_B + *(DWORD*)(Camera_B + 1) + 5;
			if(*Camera_C == 0xB8){
				Ptr = *(DWORD*)(Camera_C+0x1);
			}
		}
	}

	if(Ptr)
	{
		*(float*)(Ptr + 0x1E4) = 24.0f;
		*(float*)(Ptr + 0x1EC) = 1.0f;
	}
}