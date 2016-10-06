#pragma once
#include <windows.h>

enum COVerlay_Key {
	CK_CAPS, CK_NUMLOCK
};

#define COVERLAY_MESSAGE (WM_USER + 15)

#define COVERLAY_ERROR_ALREADY_RUNNING 1
#define COVERLAY_ERROR_HOOK_FAILED 2
#define COVERLAY_ERROR_DLL 3