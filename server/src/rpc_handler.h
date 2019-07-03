#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include "socket_serve.h"
#include <jsonrp.hpp>

class RequestHandler
{
public:
   jsonrpcpp::response_ptr parseRequest( std::string str );
private:
   jsonrpcpp::response_ptr filelist( const jsonrpcpp::Id& id, const jsonrpcpp::Parameter& params );
   jsonrpcpp::response_ptr copyfile( const jsonrpcpp::Id& id, const jsonrpcpp::Parameter& params );
   jsonrpcpp::response_ptr sync( const jsonrpcpp::Id& id, const jsonrpcpp::Parameter& params );
};
