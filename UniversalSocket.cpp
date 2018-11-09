#include "stdafx.h"
#include "UniversalSocket.h"
#include "SocketLow.h"

UniversalSocket::UniversalSocket()
{
}

UniversalSocket::~UniversalSocket()
{
}

SocketUdp::SocketUdp(string type)
{
	string ipAddress = "127.0.0.1";

	if (type == "server")
	{
		socket = new udp::socket(io_service, udp::endpoint(udp::v4(), 7805));
		endpoint = new udp::endpoint();
	}
	else if (type == "client")
	{
		socket = new udp::socket(io_service);
		endpoint = new udp::endpoint(ip::address::from_string(ipAddress), 7805);
		socket->open(udp::v4());
	}
}

SocketUdp::~SocketUdp()
{
	delete socket;
	delete endpoint;
}

void* SocketUdp::getNativeHandle()
{
	return &(socket->native_handle());
}

string SocketUdp::getRemoteIp()
{
	return endpoint->address().to_string();
}

size_t SocketUdp::send(char* str)
{
	return send(str, strlen(str) + 1);
}

size_t SocketUdp::send(char* str, int length)
{
	size_t sentLength = 0;

	char buffer[1];

	int tries = 30;
	while (tries)
	{
		//cout << "S1" << endl;
		socket->send_to(boost::asio::buffer("?", 1), *endpoint, 0, lastError);

		//cout << "S2" << endl;
		// Get ACK
		socket->receive_from(boost::asio::buffer(buffer, 1), *endpoint, 0, lastError);
		if (buffer[0] != '!')
		{
			tries--;
			continue;
		}

		//cout << "S3 " << str << " length " << length << endl;
		// Send packet
		sentLength = socket->send_to(boost::asio::buffer(str, length), *endpoint, 0, lastError);

		break;
	}

	/*if (lastError && lastError != boost::asio::error::message_size)
	{
		throw boost::system::system_error(lastError);
	}*/

	return sentLength;
}

size_t SocketUdp::receive(char* str, int length)
{
	size_t receivedLength = 0;

	char buffer[1];

	int tries = 30;
	while (tries)
	{
		//cout << "R1" << endl;
		socket->receive_from(boost::asio::buffer(buffer, 1), *endpoint, 0, lastError);
		if (buffer[0] != '?')
		{
			tries--;
			continue;
		}

		//cout << "R2" << endl;
		socket->send_to(boost::asio::buffer("!", 1), *endpoint, 0, lastError);

		//cout << "R3" << endl;
		// Receive packet
		receivedLength = socket->receive_from(boost::asio::buffer(str, length), *endpoint, 0, lastError);

		break;
	}

	/*if (lastError && lastError != boost::asio::error::message_size)
	{
		throw boost::system::system_error(lastError);
	}*/

	return receivedLength;
}




///////////////////////////////////////
//
//
//               TCP
//
//
///////////////////////////////////////






SocketTcp::SocketTcp(string type)
{
	string ipAddress = "127.0.0.1";

	if (type == "server")
	{
		endpoint = new tcp::endpoint(tcp::v4(), 7850); // listen on 7850
		acceptor = new tcp::acceptor(io_service, *endpoint);
		socket = new tcp::socket(io_service);
		acceptor->accept(*socket);
	}
	else if (type == "client")
	{
		endpoint = new tcp::endpoint(address::from_string(ipAddress.c_str()), 7850);
		socket = new ip::tcp::socket(io_service);
		boost::system::error_code ec;
		socket->connect(*endpoint, ec);
		if (!ec)
		{
			io_service.run();
		}
	}
}

SocketTcp::~SocketTcp()
{
	delete socket;
	delete endpoint;
	delete acceptor;
}

void* SocketTcp::getNativeHandle()
{
	return &(socket->native_handle());
}

string SocketTcp::getRemoteIp()
{
	return socket->remote_endpoint().address().to_string();
}

size_t SocketTcp::send(char* str)
{
	return send(str, strlen(str) + 1);
}

size_t SocketTcp::send(char* str, int length)
{
	size_t sentLength = 0;

	//cout << "S1 " << str << " length " << length << endl;
	// Send packet
	sentLength = socket->write_some(boost::asio::buffer(str, length), lastError);

	/*if (lastError && lastError != boost::asio::error::message_size)
	{
		throw boost::system::system_error(lastError);
	}*/

	return sentLength;
}

size_t SocketTcp::receive(char* str, int length)
{
	size_t receivedLength = 0;

	//cout << "R1" << endl;
	// Receive packet
	receivedLength = socket->read_some(boost::asio::buffer(str, length), lastError);

	/*if (lastError && lastError != boost::asio::error::message_size)
	{
		throw boost::system::system_error(lastError);
	}*/

	return receivedLength;
}




