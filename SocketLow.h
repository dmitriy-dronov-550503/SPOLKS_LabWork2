#pragma once
#include <iostream>
#include <boost/asio.hpp>

#include "UniversalSocket.h"

#ifdef _WIN32
#include <Mstcpip.h>
#endif

using namespace std;
using namespace boost::asio;

typedef UniversalSocket* socket_ptr;

class SocketLow
{
public:
	SocketLow();
	~SocketLow();
	static void SetReceiveTimeout(socket_ptr sock);
	static void SetKeepAlive(socket_ptr sock);
};

