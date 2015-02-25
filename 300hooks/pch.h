#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>

#include <Windows.h>
#include <WinNT.h>

#include "detours.h"

#define DECLARE_INSTANCE(_X_) static _X_* GetInstance(){	\
	static _X_* _instance = NULL;	\
	if(_instance == NULL){	\
		_instance = new _X_();	\
	}	\
	return _instance;	\
} 

#define GAME_BASE_ADDRESS 0x401000