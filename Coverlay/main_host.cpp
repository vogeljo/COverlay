#include <windows.h>
#include <CommCtrl.h>
#include <windowsx.h>
#include <iostream>

#include "resource.h"
#include "../struct.h"

#define COVERLAY_MUTEX TEXT("COverlayMutex.01")

#define COVERLAY_WINDOW_HIDE_TIMER 0x123a
#define COVERLAY_WINDOW_CLASS TEXT("COverlay_Window")
#define COVERLAY_WINDOW_HEIGHT 70

typedef void(*INITFUNC)(HWND window);

HINSTANCE gInstance = NULL;
HWND hWindow = NULL;
HWND hLabel = NULL;
HANDLE hTimer = NULL;
HFONT hFont = NULL;
HBRUSH hBackYes = NULL, hBackNo = NULL;
HBRUSH hBackLabel = NULL;
HANDLE hMutex = NULL;

void PresentWindow(TCHAR *text = NULL, HBRUSH brush = NULL, HWND hWnd = NULL, UINT timeout = 500) {
	if (!hWnd)
		hWnd = hWindow;

	if (brush) {
		hBackLabel = brush;
		RedrawWindow(hLabel, NULL, NULL, RDW_ERASE);
		UpdateWindow(hLabel);
	}
	if (text)
		Static_SetText(hLabel, text);

	RECT rect;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);
	SetWindowPos(hWnd, HWND_TOP, rect.left, rect.bottom - COVERLAY_WINDOW_HEIGHT, rect.right - rect.left, COVERLAY_WINDOW_HEIGHT, SWP_NOCOPYBITS);

	SetTimer(hWnd, COVERLAY_WINDOW_HIDE_TIMER, timeout, NULL);
	ShowWindow(hWnd, SW_SHOW);
}

LRESULT WINAPI WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	bool isPushed = !!lParam;

	switch (uMsg) {
	case WM_CREATE:
		hFont = CreateFont(0, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, TEXT("Arial"));
		hBackYes = CreateSolidBrush(RGB(0, 200, 0));
		hBackNo = CreateSolidBrush(RGB(200, 0, 0));

		hLabel = CreateWindow(WC_STATIC, TEXT("COverlay"), WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE, 0, 0, 100, 100, hWnd, NULL, gInstance, NULL);
		SendMessage(hLabel, WM_SETFONT, (WPARAM)hFont, TRUE);
		break;
	case COVERLAY_MESSAGE:
		switch ((COVerlay_Key)wParam) {
		case COVerlay_Key::CK_CAPS:
			PresentWindow(isPushed ? TEXT("CAPS LOCK is ON") : TEXT("CAPS LOCK is OFF"), isPushed ? hBackYes : hBackNo);
			break;
		case COVerlay_Key::CK_NUMLOCK:
			PresentWindow(isPushed ? TEXT("NUM LOCK is ON") : TEXT("NUM LOCK is OFF"), isPushed ? hBackYes : hBackNo);
			break;
		}
		break;
	case WM_TIMER:
		if (wParam == COVERLAY_WINDOW_HIDE_TIMER) {
			ShowWindow(hWindow, SW_HIDE);
		}
		break;
	case WM_SIZE:
		SetWindowPos(hLabel, NULL, NULL, NULL, LOWORD(lParam), HIWORD(lParam), SWP_NOMOVE | SWP_NOZORDER);
		break;
	case WM_CTLCOLORSTATIC:
		if ((HWND)lParam == hLabel) {
			HDC hdc = (HDC)wParam;
			SetBkMode(hdc, TRANSPARENT);

			return (LRESULT)hBackLabel;
		}
		break;
	case WM_DESTROY:
		DeleteObject(hFont);
		DeleteObject(hBackYes);
		DeleteObject(hBackNo);
		KillTimer(hWindow, COVERLAY_WINDOW_HIDE_TIMER);
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	gInstance = hInstance;

	hMutex = CreateMutex(NULL, FALSE, COVERLAY_MUTEX);
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		return -1;
	}

	HMODULE hDll = LoadLibrary(TEXT("COverlay_Slave.dll"));
	HHOOK hHook = NULL;
	int errcode = 0;

	if (hDll) {
		WNDCLASS cls;
		ZeroMemory(&cls, sizeof(cls));
		cls.lpszClassName = COVERLAY_WINDOW_CLASS;
		cls.hCursor = LoadCursor(NULL, IDC_ARROW);
		cls.lpfnWndProc = WndProc;
		cls.hbrBackground = CreateSolidBrush(RGB(100, 200, 200));
		cls.hInstance = GetModuleHandle(0);
		RegisterClass(&cls);

		hWindow = CreateWindowEx(WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW, COVERLAY_WINDOW_CLASS, TEXT("COverlay"), WS_POPUP, 0, 0, 100, 100, NULL, NULL, GetModuleHandle(0), NULL);

		SetLayeredWindowAttributes(hWindow, NULL, 220, LWA_ALPHA);

		HOOKPROC proc = (HOOKPROC)GetProcAddress(hDll, "COverlay_LowLevelKeyboardProc");
		INITFUNC procInit = (INITFUNC)GetProcAddress(hDll, "COverlay_Initialize");

		if (proc && procInit) {
			procInit(hWindow);
			hHook = SetWindowsHookEx(WH_KEYBOARD_LL, proc, hDll, 0);
			if (hHook) {
				procInit(hWindow);
			}
			else {
				std::cerr << "SetWindowsHookEx failed.\n";
				errcode = COVERLAY_ERROR_HOOK_FAILED;
			}
		}
		else {
			std::cerr << "GetProcAddress failed.\n";
			errcode = COVERLAY_ERROR_HOOK_FAILED;
		}
	}
	else {
		std::cerr << "LoadLibrary failed.\n";
		errcode = COVERLAY_ERROR_HOOK_FAILED;
	}

	if (errcode)
		return -errcode;
	
	// Initialization successful

	MSG msg;
	while (GetMessage(&msg, NULL, NULL, NULL) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	UnhookWindowsHookEx(hHook);
	FreeLibrary(hDll);

	CloseHandle(hMutex);
	hMutex = NULL;

	return (int)msg.wParam;
}