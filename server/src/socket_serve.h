#pragma once
#include <minwindef.h>

extern CHAR folderPath[MAX_PATH];
extern CHAR serviceName[MAX_PATH];
void serveClient( ULONG* clSocket );
int socketInit( ULONG ip, USHORT port );
