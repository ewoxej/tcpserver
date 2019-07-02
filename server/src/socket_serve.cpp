#include "socket_serve.h"
#include <tchar.h>
#include <vector>
#include <memory>
#include <thread>
#include "rpc_handler.h"

std::vector<std::thread*> threads;
const int bufferSize = 1024;

int SocketInit( ULONG ip, USHORT port )
{
   char buff[bufferSize];
   if( WSAStartup( 0x0202, (WSADATA *)&buff[0] ) )
      return -1;

   SOCKET mysocket;
   if( ( mysocket = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
   {
      WSACleanup();
      return -1;
   }

   sockaddr_in local_addr;
   local_addr.sin_family = AF_INET;
   local_addr.sin_port = htons( port );
   local_addr.sin_addr.s_addr = htonl( ip );

   if( bind( mysocket, (sockaddr *)&local_addr,
      sizeof( local_addr ) ) )
   {
      closesocket( mysocket );
      WSACleanup();
      return -1;
   }

   if( listen( mysocket, 0x100 ) )
   {
      closesocket( mysocket );
      WSACleanup();
      return -1;
   }
   SOCKET client_socket;
   sockaddr_in client_addr;
   int client_addr_size = sizeof( client_addr );
   while( ( client_socket = accept( mysocket, (sockaddr *)
      &client_addr, &client_addr_size ) ) )
   {
      threads.push_back( new std::thread( ServeClient, &client_socket ) );
   }
   for( auto i : threads )
      i->join();
   return 0;

}


DWORD WINAPI ServeClient( LPVOID client_socket )
{
   SOCKET my_sock;
   my_sock = ( (SOCKET *)client_socket )[0];
   char buff[bufferSize];
   send( my_sock, serviceName, sizeof( serviceName ), 0 );
   int bytes_recv = 0;
   while( bytes_recv != SOCKET_ERROR )
   {
      bytes_recv = recv( my_sock, &buff[0], sizeof( buff ), 0 );
      if( bytes_recv > 0 )
         buff[bytes_recv] = 0;
      jsonrpcpp::response_ptr resp = parseRequest( buff );
      if( !resp ) send( my_sock, "Incorrect request", std::string( "Incorrect request" ).length(), 0 );
      else
      {
         std::string strRes = resp->result.dump();
         send( my_sock, strRes.c_str(), strRes.length(), 0 );
      }
   }

   closesocket( my_sock );
   return 0;
}