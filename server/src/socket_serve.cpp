#include "socket_serve.h"
#include <string>
#include <vector>
#include <Windows.h>
#pragma comment(lib,"ws2_32.lib")
#include <tchar.h>
#include <vector>
#include <memory>
#include <thread>
#include "rpc_handler.h"

std::vector<std::thread*> threads;
SOCKET activeSocket;
const int bufferSize = 1024;
const int indent = 3;
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
   activeSocket = mainSocket;
   while( clientSocket = accept( mainSocket, reinterpret_cast<sockaddr*>( &clientAdress ), &clientAdressSize ) )
   {
      if( !isWorking ) break;
      //serveClient( &clientSocket );
      threads.push_back( new std::thread( serveClient, clientSocket ) );
   }
   for( auto i : threads )
   {
      i->join();
   }
   return 0;

}


void serveClient( SOCKET clSocket )
{
   SOCKET clientSocket;
   clientSocket = clSocket;
   char buff[bufferSize];
   send( clientSocket, serviceName, sizeof( serviceName ), 0 );
   int bytesRecv = 0;
   while( bytesRecv != SOCKET_ERROR )
   {
      buff[0] = 0;
      bytesRecv = recv( clientSocket, &buff[0], sizeof( buff ), 0 );
      if( bytesRecv > 0 )
      {
         buff[bytesRecv] = 0;
      }
      RequestHandler handler;
      jsonrpcpp::response_ptr resp = handler.parseRequest( buff, clSocket );
      if( !resp )
      {
         send( clientSocket, buff, strlen(buff), 0 );
      }
      else
      {
         std::string strRes = resp->result.dump( indent );
         send( clientSocket, strRes.c_str(), strRes.length(), 0 );
      }
   }

   closesocket( clientSocket );
}