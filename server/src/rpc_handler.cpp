#include "rpc_handler.h"

jsonrpcpp::response_ptr filelist( const jsonrpcpp::Id& id, const jsonrpcpp::Parameter& params )
{
   std::string fPath = params.get( 0 );
   Json files;
   WIN32_FIND_DATAA data;

   HANDLE hfile = FindFirstFileA( ( std::string( fPath ) + "\\*.*" ).c_str(), &data );
   bool res = true;
   while( res )
   {
      if( strcmp( data.cFileName, "." ) != 0 && strcmp( data.cFileName, ".." ) != 0 )
         files.push_back( data.cFileName );
      res = FindNextFileA( hfile, &data );
   }
   FindClose( hfile );
   return std::make_shared<jsonrpcpp::Response>( id, files );
}

jsonrpcpp::response_ptr copyfile( const jsonrpcpp::Id& id, const jsonrpcpp::Parameter& params )
{
   std::string path_src = params.get( 0 );
   std::string path_dest = params.get( 1 );
   bool res = CopyFileA( path_src.c_str(), path_dest.c_str(), TRUE );
   return std::make_shared<jsonrpcpp::Response>( id, res );
}

jsonrpcpp::response_ptr sync( const jsonrpcpp::Id& id, const jsonrpcpp::Parameter& params )
{
   std::string path_src = params.get( 0 );
   std::string path_dest = params.get( 1 );
   WIN32_FIND_DATAA data;

   HANDLE hfile = FindFirstFileA( ( std::string( path_src ) + "\\*.*" ).c_str(), &data );
   bool res = true;
   while( res )
   {
      if( strcmp( data.cFileName, "." ) != 0 && strcmp( data.cFileName, ".." ) != 0 )
         CopyFileA( ( path_src + "\\" + data.cFileName ).c_str(), ( path_dest + "\\" + data.cFileName ).c_str(), TRUE );
      res = FindNextFileA( hfile, &data );
   }
   FindClose( hfile );
   hfile = FindFirstFileA( ( std::string( path_dest ) + "\\*.*" ).c_str(), &data );
   res = true;
   while( res )
   {
      if( strcmp( data.cFileName, "." ) != 0 && strcmp( data.cFileName, ".." ) != 0 )
         CopyFileA( ( path_dest + "\\" + data.cFileName ).c_str(), ( path_src + "\\" + data.cFileName ).c_str(), TRUE );
      res = FindNextFileA( hfile, &data );
   }
   FindClose( hfile );
   return std::make_shared<jsonrpcpp::Response>( id, true );
}

jsonrpcpp::response_ptr parseRequest( std::string str )
{
   jsonrpcpp::Parser parser;
   jsonrpcpp::entity_ptr entity;
   try
   {
      entity = parser.parse( str );
   }
   catch( ... )
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
      else if( request->method == "upload" )
      {
         request->params.param_array.push_back( folderPath );
         return std::dynamic_pointer_cast<jsonrpcpp::Response>( copyfile( 0, request->params ) );
      }
      else if( request->method == "download" )
      {
         request->params.param_array.insert( request->params.param_array.begin(), folderPath );
         return std::dynamic_pointer_cast<jsonrpcpp::Response>( copyfile( 0, request->params ) );
      }
      else if( request->method == "sync" )
      {
         request->params.param_array.push_back( folderPath );
         jsonrpcpp::response_ptr resp = std::dynamic_pointer_cast<jsonrpcpp::Response> ( sync( 0, request->params ) );
         return std::dynamic_pointer_cast<jsonrpcpp::Response>( sync( 0, request->params ) );
      }
      else
         throw jsonrpcpp::MethodNotFoundException( *request );
   }
   return nullptr;
}