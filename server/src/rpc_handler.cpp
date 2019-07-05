#define _WINSOCK2API_
#include <Windows.h>
#include "rpc_handler.h"
#include <experimental/filesystem>
#include <iostream>
#include <memory>
#pragma comment(lib,"Mswsock.lib")

namespace fs = std::experimental::filesystem;

bool RequestHandler::sendFile( std::string path )
{
   const int bufferSize = 1024;
   CHAR buffer[bufferSize];
   DWORD bytesRecv = 0;
   HANDLE file = CreateFileA( path.c_str(), FILE_ALL_ACCESS, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
   DWORD losize = GetFileSize( file, NULL );
   if( file == INVALID_HANDLE_VALUE )
      losize = 0;
   char arr[4];
   arr[0] = ( losize >> 24 );
   arr[1] = ( losize >> 16 );
   arr[2] = ( losize >> 8 );
   arr[3] = losize;
   send( clientSocket, (char*)arr, 4, 0 );
   TransmitFile( clientSocket, file, 0, 0, NULL, NULL, TF_WRITE_BEHIND );
   CloseHandle( file );
   bytesRecv = recv( clientSocket, &buffer[0], sizeof( buffer ), 0 );
   if( strcmp( buffer, "ok" ) )
      return true;
   else
      return false;
}

bool RequestHandler::receiveFile( std::string path )
{
   const int bufferSize = 1024;
   CHAR buffer[bufferSize];
   char arr[4]{ 0 };
   recv( clientSocket, &arr[0], 4, 0 );
   unsigned long sz = 0;
   sz |= ( arr[0] << 24 ) & 0xff000000;
   sz |= ( arr[1] << 16 ) & 0x00ff0000;
   sz |= ( arr[2] << 8 ) & 0x0000ff00;
   sz |= arr[3] & 0x000000ff;
   if( sz == 0 ) return false;
   HANDLE file = CreateFileA( path.c_str(), FILE_ALL_ACCESS, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
   if( file == INVALID_HANDLE_VALUE )
   {

      int pos = path.size() - 1;
      for( int i = path.size() - 1; i >= 0; i-- )
      {
         if( path[i] == '/' or path[i] == '\\' )
         {
            pos = i;
            break;
         }
      }
      std::string folderpath = path.substr( 0, pos );
      CreateDirectoryA( folderpath.c_str(), NULL );
      file = CreateFileA( path.c_str(), FILE_ALL_ACCESS, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
   }
   unsigned long receivedBytes = 0;
   int rBytes = 0;
   while( receivedBytes < sz )
   {
      rBytes = recv( clientSocket, &buffer[0], sizeof( buffer ), 0 );
      WriteFile( file, buffer, rBytes, NULL, NULL );
      receivedBytes += rBytes;
   }
   CloseHandle( file );
   return true;
}

void enumerateFilesInFolder( std::string path, Json* files )
{
   WIN32_FIND_DATAA data;
   Json nestedFiles;
   HANDLE hfile = FindFirstFileA( ( path + "\\*.*" ).c_str(), &data );
   bool res = true;
   while( res )
   {
      if( strcmp( data.cFileName, "." ) != 0 && strcmp( data.cFileName, ".." ) != 0 )
      {
         nestedFiles.push_back( data.cFileName );
         if( data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
            enumerateFilesInFolder( path + "\\" + data.cFileName, &nestedFiles );
      }
      res = FindNextFileA( hfile, &data );
   }
   FindClose( hfile );
   files->push_back( nestedFiles );
}
jsonrpcpp::response_ptr RequestHandler::filelist( const jsonrpcpp::Id& id, const jsonrpcpp::Parameter& params )
{
   std::string fPath = params.get( 0 );
   Json files;
   WIN32_FIND_DATAA data;

   HANDLE hfile = FindFirstFileA( ( std::string( fPath ) + "\\*.*" ).c_str(), &data );
   bool res = true;
   while( res )
   {
      if( strcmp( data.cFileName, "." ) != 0 && strcmp( data.cFileName, ".." ) != 0 )
      {
         files.push_back( data.cFileName );
         if( data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
            enumerateFilesInFolder( fPath + "\\" + data.cFileName, &files );
      }
      res = FindNextFileA( hfile, &data );
   }
   FindClose( hfile );
   return std::make_shared<jsonrpcpp::Response>( id, files );
}

jsonrpcpp::response_ptr RequestHandler::download( const jsonrpcpp::Id& id, const jsonrpcpp::Parameter& params )
{
   std::string pathSrc = params.get( 0 );
   bool res = sendFile( pathSrc );
   return std::make_shared<jsonrpcpp::Response>( id, res );
}
jsonrpcpp::response_ptr RequestHandler::upload( const jsonrpcpp::Id& id, const jsonrpcpp::Parameter& params )
{
   std::string pathSrc = params.get( 0 );
   bool res = receiveFile( pathSrc );
   return std::make_shared<jsonrpcpp::Response>( id, res );
}
jsonrpcpp::response_ptr RequestHandler::echo( const jsonrpcpp::Id & id, const jsonrpcpp::Parameter & params )
{
   return std::make_shared<jsonrpcpp::Response>( id, params.get( 0 ) );
}
jsonrpcpp::response_ptr RequestHandler::isExist( const jsonrpcpp::Id & id, const jsonrpcpp::Parameter & params )
{
   std::string pathSrc = params.get( 0 );
   bool res = false;
   HANDLE file = CreateFileA( pathSrc.c_str(), FILE_ALL_ACCESS, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
   if( file != INVALID_HANDLE_VALUE )
      res = true;
   CloseHandle( file );
   return std::make_shared<jsonrpcpp::Response>( id, res );
}

jsonrpcpp::response_ptr RequestHandler::parseRequest( std::string str, SOCKET clSocket )
{
   clientSocket = clSocket;
   jsonrpcpp::Parser parser;
   jsonrpcpp::entity_ptr entity;
   try
   {
      entity = parser.parse( str );
   }
   catch( jsonrpcpp::ParseErrorException )
   {
      return nullptr;
   }
   if( entity->is_request() )
   {
      jsonrpcpp::request_ptr request = std::dynamic_pointer_cast<jsonrpcpp::Request>( entity );
      if( request->method == "filelist" )
      {
         request->params.param_array.push_back( folderPath );
         return std::dynamic_pointer_cast<jsonrpcpp::Response>( filelist( 0, request->params ) );
      }
      else if( request->method == "exist" )
      {
         std::string srcpath = request->params.get( 0 );
         request->params.param_array.clear();
         request->params.param_array.push_back( std::string( folderPath ) + "\\" + srcpath );
         return std::dynamic_pointer_cast<jsonrpcpp::Response>( isExist( 0, request->params ) );
      }
      else if( request->method == "echo" )
      {
         std::string srcpath = request->params.get( 0 );
         request->params.param_array.clear();
         request->params.param_array.push_back( std::string( folderPath ) + "\\" + srcpath );
         return std::dynamic_pointer_cast<jsonrpcpp::Response>( echo( 0, request->params ) );
      }
      else if( request->method == "download" )
      {
         std::string srcpath = request->params.get( 0 );
         request->params.param_array.clear();
         request->params.param_array.push_back( std::string( folderPath ) + "\\" + srcpath );
         return std::dynamic_pointer_cast<jsonrpcpp::Response>( download( 0, request->params ) );
      }
      else if( request->method == "upload" )
      {
         std::string srcpath = request->params.get( 0 );
         request->params.param_array.clear();
         request->params.param_array.push_back( std::string( folderPath ) + "\\" + srcpath );
         return std::dynamic_pointer_cast<jsonrpcpp::Response>( upload( 0, request->params ) );
      }
      else
      {
         return nullptr;
      }
   }
   return nullptr;
}