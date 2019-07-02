#pragma once
#include <string>
#include <vector>
#include <winsock2.h>
#include <windows.h>
#pragma comment(lib,"ws2_32.lib")

extern CHAR folderPath[MAX_PATH];
extern CHAR serviceName[MAX_PATH];
DWORD WINAPI serveClient( LPVOID clSocket );
int socketInit( ULONG ip, USHORT port );
