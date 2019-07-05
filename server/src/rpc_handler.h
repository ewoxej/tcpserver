#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include "socket_serve.h"
#include <jsonrp.hpp>

class RequestHandler
{
public:
   jsonrpcpp::response_ptr parseRequest( std::string str, SOCKET clSocket );
private:
   SOCKET clientSocket;
   bool sendFile( std::string path );
   bool receiveFile( std::string path );
   jsonrpcpp::response_ptr filelist( const jsonrpcpp::Id& id, const jsonrpcpp::Parameter& params );
   jsonrpcpp::response_ptr download( const jsonrpcpp::Id& id, const jsonrpcpp::Parameter& params );
   jsonrpcpp::response_ptr upload( const jsonrpcpp::Id& id, const jsonrpcpp::Parameter& params );
   jsonrpcpp::response_ptr echo( const jsonrpcpp::Id& id, const jsonrpcpp::Parameter& params );
   jsonrpcpp::response_ptr isExist( const jsonrpcpp::Id& id, const jsonrpcpp::Parameter& params );
   //jsonrpcpp::response_ptr sync( const jsonrpcpp::Id& id, const jsonrpcpp::Parameter& params );
};
