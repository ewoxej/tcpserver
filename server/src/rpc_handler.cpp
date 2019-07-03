#include <Windows.h>
#include "rpc_handler.h"
#include <experimental/filesystem>
#include <iostream>
#include <memory>

namespace fs = std::experimental::filesystem;

jsonrpcpp::response_ptr RequestHandler::filelist( const jsonrpcpp::Id& id, const jsonrpcpp::Parameter& params )
{
   std::string fPath = params.get( 1 );
   Json files;
   WIN32_FIND_DATAA data;

   HANDLE hfile = FindFirstFileA( ( std::string( fPath ) + "\\*.*" ).c_str(), &data );
   bool res = true;
   while( res )
   {
      if( strcmp( data.cFileName, "." ) != 0 && strcmp( data.cFileName, ".." ) != 0 )
      {
         files.push_back( data.cFileName );
      }
      res = FindNextFileA( hfile, &data );
   }
   FindClose( hfile );
   return std::make_shared<jsonrpcpp::Response>( id, files );
}

jsonrpcpp::response_ptr RequestHandler::copyfile( const jsonrpcpp::Id& id, const jsonrpcpp::Parameter& params )
{
   std::string pathSrc = params.get( 0 );
   std::string pathDest = params.get( 1 );
   std::error_code code;
   fs::copy( pathSrc.c_str(), pathDest.c_str(), fs::copy_options::recursive | fs::copy_options::skip_existing, code );
   return std::make_shared<jsonrpcpp::Response>( id, (bool)( code ) );
}

jsonrpcpp::response_ptr RequestHandler::sync( const jsonrpcpp::Id& id, const jsonrpcpp::Parameter& params )
{
   std::string pathSrc = params.get( 0 );
   std::string pathDest = params.get( 1 );
   WIN32_FIND_DATAA data;

   HANDLE hfile = FindFirstFileA( ( std::string( pathSrc ) + "\\*.*" ).c_str(), &data );
   bool res = true;
   while( res )
   {
      if( strcmp( data.cFileName, "." ) != 0 && strcmp( data.cFileName, ".." ) != 0 )
      {
         if( data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
            fs::copy( ( pathSrc + "\\" + data.cFileName ).c_str(), ( pathDest + "\\" + data.cFileName ).c_str(), fs::copy_options::recursive | fs::copy_options::skip_existing );
         else
            fs::copy( ( pathSrc + "\\" + data.cFileName ).c_str(), ( pathDest + "\\" + data.cFileName ).c_str(), fs::copy_options::skip_existing );
      }
      res = FindNextFileA( hfile, &data );
   }
   FindClose( hfile );
   hfile = FindFirstFileA( ( std::string( pathDest ) + "\\*.*" ).c_str(), &data );
   res = true;
   while( res )
   {
      if( strcmp( data.cFileName, "." ) != 0 && strcmp( data.cFileName, ".." ) != 0 )
      {
         if( data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
            fs::copy( ( pathDest + "\\" + data.cFileName ).c_str(), ( pathSrc + "\\" + data.cFileName ).c_str(), fs::copy_options::recursive | fs::copy_options::skip_existing );
         else
            fs::copy( ( pathDest + "\\" + data.cFileName ).c_str(), ( pathSrc + "\\" + data.cFileName ).c_str(), fs::copy_options::skip_existing );
      }
      res = FindNextFileA( hfile, &data );
   }
   FindClose( hfile );
   return std::make_shared<jsonrpcpp::Response>( id, true );
}

jsonrpcpp::response_ptr RequestHandler::parseRequest( std::string str )
{
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
      else if( request->method == "download" || request->method == "upload" )
      {
         std::string srcpath = request->params.get( 0 );
         std::string filename = request->params.get( 1 );
         request->params.param_array.clear();
         if( request->method == "download" )
            request->params.param_array.push_back( std::string( folderPath ) + "\\" + filename );
         request->params.param_array.push_back( srcpath + "\\" + filename );
         if( request->method == "upload" )
            request->params.param_array.push_back( std::string( folderPath ) + "\\" + filename );
         return std::dynamic_pointer_cast<jsonrpcpp::Response>( copyfile( 0, request->params ) );
      }
      else if( request->method == "sync" )
      {
         request->params.param_array.push_back( folderPath );
         jsonrpcpp::response_ptr resp = std::dynamic_pointer_cast<jsonrpcpp::Response> ( sync( 0, request->params ) );
         return std::dynamic_pointer_cast<jsonrpcpp::Response>( sync( 0, request->params ) );
      }
      else
      {
         return nullptr;
      }
   }
   return nullptr;
}