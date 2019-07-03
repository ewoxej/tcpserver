#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "socket_serve.h"
#include <string>
#include <vector>
#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")
#include <tchar.h>
#include <vector>
#include <memory>
#include <thread>
#include "rpc_handler.h"

std::vector<std::thread*> threads;
const int bufferSize = 1024;

int socketInit( ULONG ip, USHORT port )
{
   char buff[bufferSize];
   if( WSAStartup( 0x0202, reinterpret_cast<WSADATA*>( &buff[0] ) ) )
   {
      return -1;
   }
   SOCKET mainSocket;
   if( ( mainSocket = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
   {
      WSACleanup();
      return -1;
   }

   sockaddr_in localAdress;
   localAdress.sin_family = AF_INET;
   localAdress.sin_port = htons( port );
   localAdress.sin_addr.s_addr = htonl( ip );

   if( bind( mainSocket, reinterpret_cast<sockaddr*>( &localAdress ),
      sizeof( localAdress ) ) )
   {
      closesocket( mainSocket );
      WSACleanup();
      return -1;
   }

   if( listen( mainSocket, 0x100 ) )
   {
      closesocket( mainSocket );
      WSACleanup();
      return -1;
   }
   SOCKET clientSocket;
   sockaddr_in clientAdress;
   int clientAdressSize = sizeof( clientAdress );
   while( clientSocket = accept( mainSocket, reinterpret_cast<sockaddr*>( &clientAdress ), &clientAdressSize ) )
   {
      threads.push_back( new std::thread( serveClient, reinterpret_cast<unsigned long int*>( &clientSocket ) ) );
   }
   for( auto i : threads )
   {
      i->join();
   }
   return 0;

}


DWORD WINAPI serveClient( LPVOID clSocket )
{
   SOCKET clientSocket;
   clientSocket = ( static_cast<SOCKET*>( clSocket ) )[0];
   char buff[bufferSize];
   send( clientSocket, serviceName, sizeof( serviceName ), 0 );
   int bytesRecv = 0;
   while( bytesRecv != SOCKET_ERROR )
   {
      bytesRecv = recv( clientSocket, &buff[0], sizeof( buff ), 0 );
      if( bytesRecv > 0 )
      {
         buff[bytesRecv] = 0;
      }
      RequestHandler handler;
      jsonrpcpp::response_ptr resp = handler.parseRequest( buff );
      if( !resp )
      {
         send( clientSocket, "Incorrect request", std::string( "Incorrect request" ).length(), 0 );
      }
      else
      {
         std::string strRes = resp->result.dump();
         send( clientSocket, strRes.c_str(), strRes.length(), 0 );
      }
   }

   closesocket( clientSocket );
}