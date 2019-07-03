#pragma once
#include <minwindef.h>

extern CHAR folderPath[MAX_PATH];
extern CHAR serviceName[MAX_PATH];
DWORD WINAPI serveClient( LPVOID clSocket );
int socketInit( ULONG ip, USHORT port );
