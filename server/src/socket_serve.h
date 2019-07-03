#pragma once
#include <Winsock2.h>
#include <minwindef.h>

extern CHAR folderPath[MAX_PATH];
extern CHAR serviceName[MAX_PATH];
void serveClient( SOCKET clSocket );
int socketInit( ULONG ip, USHORT port );
