﻿// dllmain.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include <afxwin.h>
#include <afxdllx.h>
#include "KeyHook.h"
#include "ZhuiHuiMsg.h"
#include "zhfunclib.h"
#include <stdio.h>
#include "HookKeyChar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static AFX_EXTENSION_MODULE KeyHookDLL = { NULL, NULL };

#pragma data_seg("shareddata")
HWND glhDisplayWnd = NULL;
HHOOK glhHook = NULL;
HINSTANCE glhInstance = NULL;//DLL实例句柄 
#pragma data_seg()

bool hookChars[HOOKCHARSNUM];
bool isScanCoding;
int wxKey, alipayKey;

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	// Remove this if you use lpReserved
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		TRACE0("KeyHook.DLL Initializing!\n");
		
		// Extension DLL one-time initialization
		if (!AfxInitExtensionModule(KeyHookDLL, hInstance))
			return 0;

		// Insert this DLL into the resource chain
		// NOTE: If this Extension DLL is being implicitly linked to by
		//  an MFC Regular DLL (such as an ActiveX Control)
		//  instead of an MFC application, then you will want to
		//  remove this line from DllMain and put it in a separate
		//  function exported from this Extension DLL.  The Regular DLL
		//  that uses this Extension DLL should then explicitly call that
		//  function to initialize this Extension DLL.  Otherwise,
		//  the CDynLinkLibrary object will not be attached to the
		//  Regular DLL's resource chain, and serious problems will
		//  result.

		new CDynLinkLibrary(KeyHookDLL);
		glhInstance = hInstance;//插入保存DLL实例句柄
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		TRACE0("KeyHook.DLL Terminating!\n");

		// Terminate the library before destructors are called
		AfxTermExtensionModule(KeyHookDLL);
	}
	return 1;   // ok
}

LRESULT keyNumProc(PKBDLLHOOKSTRUCT p, int nCode,WPARAM wparam,LPARAM lparam)
{
	static KBDLLHOOKSTRUCT hookCode1;
	switch (p->flags)
	{
	case 0:
		hookCode1 = *p;
		break;
	case 128:
	case 129:
		{
			if (p->time - hookCode1.time > 35) 
			{
				isScanCoding = false;
				if (hookChars[HOOK_MANINPUT_NUM])
				{
					::SendMessage(glhDisplayWnd, ZHIHUI_MANINPUT_MSG, wparam, lparam); 
				}
				else
				{
					bool isHookManInput = hookChars[HOOK_MANINPUT_NUM];
					bool isHookScanCode = hookChars[HOOK_SCANCODE_NUM];
					hookChars[HOOK_MANINPUT_NUM] = false;
					hookChars[HOOK_SCANCODE_NUM] = false;
					keybd_event((BYTE)hookCode1.vkCode,0,0,0);
					hookChars[HOOK_MANINPUT_NUM] = isHookManInput;
					hookChars[HOOK_SCANCODE_NUM] = isHookScanCode;
					return CallNextHookEx(glhHook,nCode,wparam,lparam); 
				}
			}
			else{
				isScanCoding = true;
				::SendMessage(glhDisplayWnd, ZHIHUI_CODE_MSG, wparam, lparam); 
			}
		}
		break;
	default:
		break;
	}
	return 1;
}

//键盘钩子函数的实现
LRESULT CALLBACK KeyProc(int nCode,WPARAM wparam,LPARAM lparam) 
{
	PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT)lparam;
	//char tmpbuf[150];
	//sprintf(tmpbuf, "KeyProc keyboard code:[%04d] ,scan code: [%04d], char : %c, flag: %d, time: %d. ", p->vkCode, 
	//	p->scanCode, p->vkCode, p->flags, p->time);
	//ZHFuncLib::NativeLog("", tmpbuf, "a");

	if (hookChars[START_HOOK] && nCode>=0 && IsWindow(glhDisplayWnd)) 
	{
		switch (p->vkCode)
		{
		case VK_ESCAPE:
			{
				if (hookChars[HOOK_ESC])
				{
					if (p->flags < 128){
						::SendMessage(glhDisplayWnd, ZHIHUI_MANINPUT_MSG, wparam, lparam); 
					}
					return 1;
				}
				return CallNextHookEx(glhHook,nCode,wparam,lparam);
			}
			break;
		case VK_RETURN:
			{
				if (hookChars[HOOK_RETURN])
				{
					return keyNumProc(p,nCode,wparam,lparam);
				}
				else if (isScanCoding)
				{
					LRESULT tmpres = keyNumProc(p,nCode,wparam,lparam);
					if (p->vkCode >= 128)
						isScanCoding = false;
					return tmpres;
				}
				return CallNextHookEx(glhHook,nCode,wparam,lparam);
			}
			break;
		case VK_LSHIFT:
		case VK_RSHIFT:
		case VK_LCONTROL:
		case VK_RCONTROL:
			{
				::SendMessage(glhDisplayWnd, ZHIHUI_MANINPUT_MSG, wparam, lparam); 
				return CallNextHookEx(glhHook,nCode,wparam,lparam);
			}
			break;
		case 48:
		case 49:
		case 50:
		case 51:
		case 52:
		case 53:
		case 54:
		case 55:
		case 56:
		case 57:
		case '.':
		case VK_OEM_PERIOD:
		case VK_BACK:
			{
				if (hookChars[HOOK_MANINPUT_NUM] || hookChars[HOOK_SCANCODE_NUM]) // 不存在只拦截手动录入数字而不拦截扫码数字的情况
				{
					return keyNumProc(p,nCode,wparam,lparam);
				}
				return CallNextHookEx(glhHook,nCode,wparam,lparam);
			}
			break;
		default:
			{
				if (hookChars[HOOK_SETPAYKEY])
				{
					::SendMessage(glhDisplayWnd, ZHIHUI_MANINPUT_MSG, wparam, lparam); 
				}
				else if (p->vkCode == wxKey || p->vkCode == alipayKey)
				{
					if (hookChars[HOOK_WXKEY])
					{
						if (p->flags >= 128){
							::SendMessage(glhDisplayWnd, ZHIHUI_MANINPUT_MSG, wparam, lparam); 
						}
						return 1;
					}
				}
				return CallNextHookEx(glhHook,nCode,wparam,lparam);
			}
			break;
		}
	}


	return CallNextHookEx(glhHook,nCode,wparam,lparam); //继续传递消息 
} 

CKeyHook::CKeyHook(){}

CKeyHook::~CKeyHook()
{
	StopHook();
}

BOOL CKeyHook::StartHook(HWND hWnd)
{
	BOOL bReslt = FALSE;
	if (glhHook == NULL)
		glhHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyProc, glhInstance, 0);
	if (glhHook != NULL)
	{
		bReslt = TRUE;
	}
	glhDisplayWnd = hWnd;
	isScanCoding = false;
	return bReslt;
}

void CKeyHook::EnableInterception(int keyIndex, bool isNeedInterception)
{
	hookChars[keyIndex] = isNeedInterception;
}

bool CKeyHook::IsInterception(int keyIndex)
{
	return hookChars[keyIndex];
}

BOOL CKeyHook::StopHook()
{
	BOOL bResult = FALSE;
	if (glhHook)
	{
		bResult = UnhookWindowsHookEx(glhHook);
		if (bResult)
		{
			glhDisplayWnd = NULL;
		}

		glhHook = NULL;
	}
	return bResult;
}

void CKeyHook::SetPayKey(int inwxKey, int inalipayKey)
{
	wxKey = inwxKey;
	alipayKey = inalipayKey;
}