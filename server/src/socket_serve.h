#pragma once
#include <string>
#include <vector>
#include <winsock2.h>
#include <windows.h>
#pragma comment(lib,"ws2_32.lib")

extern CHAR folderPath[MAX_PATH];
extern CHAR serviceName[MAX_PATH];
DWORD WINAPI ServeClient( LPVOID client_socket );
int SocketInit( ULONG ip, USHORT port );
