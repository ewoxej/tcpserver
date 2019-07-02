#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include "socket_serve.h"
#include <jsonrp.hpp>

jsonrpcpp::response_ptr parseRequest( std::string str );