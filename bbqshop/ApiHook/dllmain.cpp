﻿// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

#pragma comment(lib, "detours.lib")
#pragma comment(lib, "Ws2_32.lib")

#include <cstdio>
#include <windows.h>
#include <detours.h> 
#include <cstring>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include "ZhuiHuiMsg.h"
#include "AllWindowTitle.h"
#include "HookKeyChar.h"

static FILE * fp = NULL;

char bbqPath[MAX_PATH];
char logBuff[255];

#define PROJECTREGIEDT L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"
#define PROJCETNAME L"bbqpay"

char *GetWorkPath()
{
	if (strlen(bbqPath) > 0)
		return bbqPath;
	HKEY hAppKey = 0;
	if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, PROJECTREGIEDT, 0, KEY_READ, &hAppKey))
		return "";
	DWORD dataType;
	DWORD dataSize;
	LONG res = RegQueryValueEx(hAppKey, PROJCETNAME, 0, &dataType, 0, &dataSize);
	if (res != ERROR_SUCCESS) 
	{
		RegCloseKey(hAppKey);
		return "";
	}
	if (dataType == REG_SZ || dataType == REG_EXPAND_SZ)
		dataSize += 2;
	else if (dataType == REG_MULTI_SZ)
		dataSize += 4;
	static unsigned char *odata = new unsigned char[dataSize];
	res = RegQueryValueEx(hAppKey, PROJCETNAME, 0, 0, odata, &dataSize);
	if (res != ERROR_SUCCESS) {
		RegCloseKey(hAppKey);
		delete []odata;
		return "";
	}
	// 去除空格
	int charpos = 0;
	for (int i = 0; i < dataSize; ++i)
	{
		if (odata[i] != '\0' && odata[i] != ' ')
		{
			bbqPath[charpos] = odata[i];
			++charpos;
		}
	}
	bbqPath[charpos] = 0;
	// 找到路径
	int namePos = -1;
	for (int i = 0; i < strlen(bbqPath); ++i)
	{

		if (bbqPath[i] == '\\')
			namePos = i;
	}
	bbqPath[namePos] = 0;
	RegCloseKey(hAppKey);
	delete []odata;
	return bbqPath;
}

BOOL (WINAPI *SysWriteFile)(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped) = WriteFile;
BOOL WINAPI HookWriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped)
{
	//std::string workpath = GetWorkPath();
	//workpath += "/";
	//workpath += HOOKAPIDIR;
	//workpath += "/";
	//workpath += HOOKWRITEFUNC;
	fopen_s(&fp, "D:\\QinAn\\CompanyProgram\\GitProj\\bbqshop\\bbqshop\\Debug\\hookWriteFile.txt", "w");
	if(fp != NULL)
	{
		//sprintf(logBuff, "write file toWrite: %d, byte written: %d\r\n", nNumberOfBytesToWrite, *lpNumberOfBytesWritten);
		//fwrite(logBuff, strlen(logBuff), 1, fp);
		fwrite(lpBuffer, nNumberOfBytesToWrite, 1, fp);
		fclose(fp);
		fp = NULL;
	}

	return SysWriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
}

FILE * fp1 = NULL;
HANDLE (WINAPI *SysCreateFileA)(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) = CreateFileA;
HANDLE WINAPI HookCreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	fopen_s(&fp1, "D:\\QinAn\\CompanyProgram\\GitProj\\bbqshop\\bbqshop\\Debug\\hookCriteFileA.txt", "w");
	if(fp1 != NULL)
	{
		fwrite(lpFileName, strlen(lpFileName), 1, fp1);
		fclose(fp1);
		fp1 = NULL;
	}
	
	return SysCreateFileA(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

FILE * fp2 = NULL;
HANDLE (WINAPI *SysCreateFileW)(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) = CreateFileW;
HANDLE WINAPI HookCreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	fopen_s(&fp2, "D:\\QinAn\\CompanyProgram\\GitProj\\bbqshop\\bbqshop\\Debug\\hookCriteFileW.txt", "w");
	if(fp2 != NULL)
	{

		fwrite(lpFileName, wcslen(lpFileName), 1, fp2);
		fclose(fp2);
		fp2 = NULL;
	}

	return SysCreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

BOOL APIENTRY DllMain( HMODULE hModule,
					  DWORD  ul_reason_for_call,
					  LPVOID lpReserved
					  )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
		{
			//DisableThreadLibraryCalls(hModule);
			DetourTransactionBegin();
			DetourUpdateThread(GetCurrentThread());
			LONG errNo1 = DetourAttach(&(PVOID&)SysWriteFile, HookWriteFile);
			LONG errNo2 = DetourAttach(&(PVOID&)SysCreateFileA, HookCreateFileA);
			//LONG errNo3 = DetourAttach(&(PVOID&)SysCreateFileW, HookCreateFileW);
			LONG errNo = DetourTransactionCommit();
			fopen_s(&fp, "D:\\QinAn\\CompanyProgram\\GitProj\\bbqshop\\bbqshop\\Debug\\hookdllmain.txt", "w");
			if(fp != NULL)
			{
				sprintf(logBuff, "DetourAttach errNo1: %d; errNo2: %d; errNo3: %d, no errNoCommit: %d", errNo1, errNo2, -1, errNo);
				fwrite(logBuff, strlen(logBuff), 1, fp);
				fclose(fp);
				fp = NULL;
			}
			bbqPath[0] = 0;
		}
		break;
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		DetourTransactionBegin();	//Detach
		DetourUpdateThread(GetCurrentThread());
		DetourDetach(&(PVOID&)SysWriteFile, HookWriteFile);
		DetourDetach(&(PVOID&)SysCreateFileA, HookCreateFileA);
		//DetourDetach(&(PVOID&)SysCreateFileW, HookCreateFileW);
		DetourTransactionCommit();
		break;
	}
	return TRUE;
}