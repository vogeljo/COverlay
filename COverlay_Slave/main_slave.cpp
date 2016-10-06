#include <windows.h>

#include "../struct.h"

/*
  Not needed, as exports are listed in .def file.
*/
//#define EXPORT_FUNCTION extern "C" __declspec(dllexport)
#define EXPORT_FUNCTION

bool gInitialized = false;
HWND hWindow = NULL;

EXPORT_FUNCTION LRESULT CALLBACK COverlay_LowLevelKeyboardProc(int nCode, WPARAM wMessage, LPARAM lParam) {
	KBDLLHOOKSTRUCT *kbd = (KBDLLHOOKSTRUCT*)lParam;
	if (gInitialized && nCode >= 0) {
		switch (wMessage) {
		case WM_KEYUP:
			switch (kbd->vkCode) {
			case VK_CAPITAL:
				PostMessage(hWindow, COVERLAY_MESSAGE, COVerlay_Key::CK_CAPS, GetKeyState(VK_CAPITAL) & 1);
				break;
			case VK_NUMLOCK:
				PostMessage(hWindow, COVERLAY_MESSAGE, COVerlay_Key::CK_NUMLOCK, GetKeyState(VK_NUMLOCK) & 1);
				break;
			}
		}
	}
	return CallNextHookEx(NULL, nCode, wMessage, lParam);	
}

EXPORT_FUNCTION void COverlay_Initialize(HWND window) {
	hWindow = window;
	gInitialized = true;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD dwReason, LPVOID lpvReserved) {
	/*switch(dwReason)
	{
	case DLL_PROCESS_ATTACH:
		break;
	case DLL_PROCESS_DETACH:
		break;
	}*/
	return TRUE;
}
