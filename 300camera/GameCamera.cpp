#include "pch.h"
#include "CSigMngr.h"
#include "GameCamera.h"

#define CAMERA_A_SIG "\x8B\x80\x54\xBF\x02\x00\x52\x51\x8D\x54\x24\x20\x52\x8D\x4C\x24\x2C\x51\x8D\x94\x24\x84\x00\x00\x00\x52\x8B\xC8"
#define CAMERA_A_SIG_BYTES sizeof(CAMERA_A_SIG)-1

#define CAMERA_RESET_SIG "\x8B\x8D\xC4\xBE\x02\x00\x83\xEC\x0C\xD9\x54\x24\x20\x8B\x54\x24\x20\xD9\x54\x24\x24"
#define CAMERA_RESET_SIG_BYTES sizeof(CAMERA_RESET_SIG)-1

float zoomMax = 24.0f;
float zoomMin = 1.0f;


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
	unsigned char *Camera_Reset;
	unsigned char *Camera_ResetMax;
	unsigned char *Camera_ResetMin;
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

	Camera_Reset = (unsigned char*)FIND_MEMORY(GAME_BASE_ADDRESS,CAMERA_RESET_SIG);

	if( Camera_Reset )
	{
		Camera_ResetMax = Camera_Reset - 0x4B;
		Camera_ResetMin = Camera_Reset - 0x22;

		if(*Camera_ResetMax == 0xD9 && *Camera_ResetMin == 0xD9)
		{
			Camera_ResetMax += 2;
			Camera_ResetMin += 2;
			mprotect(Camera_ResetMax,0x10,PAGE_EXECUTE_READWRITE);
			mprotect(Camera_ResetMin,0x10,PAGE_EXECUTE_READWRITE);

			*(float**)Camera_ResetMax = &zoomMax;
			*(float**)Camera_ResetMin = &zoomMin;
		}
	}
}