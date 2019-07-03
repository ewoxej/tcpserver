#pragma once
#define MAX_PATH 260
extern char folderPath[MAX_PATH];
extern char serviceName[MAX_PATH];
void serveClient( unsigned long int* clSocket );
int socketInit( long ip, short port );
