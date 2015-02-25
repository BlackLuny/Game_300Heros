#include "pch.h"
#include "CSigMngr.h"
#include "GameCamera.h"

#define CAMERA_A_SIG "\xD8\xD1\xDF\xE0\xDD\xD9\xF6\xC4\x41\x75\x13\xDD\xD8"
#define CAMERA_B_SIG "\xD8\x1F\xDF\xE0\xF6\xC4\x05\x7A\x2C\xE8"
#define CAMERA_C_SIG "\xD8\x1F\xDF\xE0\xF6\xC4\x41\x75\x61"

#define CAMERA_A_SIG_BYTES sizeof(CAMERA_A_SIG)-1
#define CAMERA_B_SIG_BYTES sizeof(CAMERA_B_SIG)-1
#define CAMERA_C_SIG_BYTES sizeof(CAMERA_C_SIG)-1

GameCamera::GameCamera(void)
{
	
}


GameCamera::~GameCamera(void)
{
}

void GameCamera::Attach()
{
	unsigned char* Camera_A;
	unsigned char* Camera_B;
	unsigned char* Camera_C;

	Camera_A = (unsigned char*)FIND_MEMORY(GAME_BASE_ADDRESS,CAMERA_A_SIG);
	Camera_B = (unsigned char*)FIND_MEMORY(GAME_BASE_ADDRESS,CAMERA_B_SIG);
	Camera_C = (unsigned char*)FIND_MEMORY(GAME_BASE_ADDRESS,CAMERA_C_SIG);

	if(Camera_A && Camera_B && Camera_C)
	{
		mprotect(Camera_A,100,PAGE_EXECUTE_READWRITE);
		mprotect(Camera_B,100,PAGE_EXECUTE_READWRITE);
		mprotect(Camera_C,100,PAGE_EXECUTE_READWRITE);

		Camera_A[9] = 0xEB;
		Camera_B[7] = 0xEB;

		SetNopCode(&Camera_C[7],2);	//nop
	}
}