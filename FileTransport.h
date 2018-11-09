#pragma once
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include "UniversalSocket.h"

using namespace std;
using namespace std::chrono;
using namespace boost::asio;

typedef UniversalSocket* socket_ptr;

class FileTransport
{
private:
	constexpr static uint32_t sendBufferSize = 4 * 1024;
	constexpr static uint32_t receiveBufferSize = sendBufferSize;
	
	socket_ptr sock;
	char* data;
	time_point<steady_clock, nanoseconds> start;
	thread* speedThread;
	bool isShowSpeed;
	fstream file;

public:
	FileTransport(socket_ptr sock_in);
	void Send(string filenameFrom, string filenameTo);
	void Receive(string filenameFrom, string filenameTo);
	int64_t SendFileSize(string fileName);
};

