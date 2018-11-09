#pragma once

#include <iostream>
#include <thread>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include "CommandCenter.h"

using namespace std;
using namespace boost::asio;
using namespace boost::asio::ip;

class UniversalSocket
{
protected:
	boost::asio::io_service io_service;
	boost::system::error_code lastError;
	constexpr static int maxBufferSize = 4 * 1024;

public:
	UniversalSocket();
	~UniversalSocket();

	virtual void* getNativeHandle() = 0;
	virtual string getRemoteIp() = 0;

	virtual size_t send(char* str) = 0;
	virtual size_t send(char* str, int length) = 0;
	virtual size_t receive(char* str, int length) = 0;
};

class SocketUdp : public UniversalSocket
{
private:
	udp::socket* socket;
	udp::endpoint* endpoint;

public:
	SocketUdp(string type);
	~SocketUdp();

	void* getNativeHandle();
	string getRemoteIp();

	size_t send(char* str);
	size_t send(char* str, int length);
	size_t receive(char* str, int length);
};

class SocketTcp : public UniversalSocket
{
private:
	tcp::socket* socket;
	tcp::endpoint* endpoint;
	tcp::acceptor* acceptor;

public:
	SocketTcp(string type);
	~SocketTcp();

	void* getNativeHandle();
	string getRemoteIp();

	size_t send(char* str);
	size_t send(char* str, int length);
	size_t receive(char* str, int length);
};

