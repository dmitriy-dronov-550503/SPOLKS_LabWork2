#include "stdafx.h"
#include <fstream>

#include "Server.h"
#include "LogSystem.h"
#include "FileTransport.h"
#include "SocketLow.h"
#include "UniversalSocket.h"

using namespace boost::asio::ip;

Server::Server(string protocol)
{
	ServerThread(protocol);
}

Server::~Server()
{
}

void Server::Join()
{
	serverThread->join();
}

void Server::ServerThread(string protocol)
{
	isServerActive = true;

	const int bufferSize = 512;
	char buffer[bufferSize];

	UniversalSocket* us = nullptr;

	try
	{
		if (protocol == "tcp")
		{
			us = new SocketTcp("server");
			SocketLow::SetKeepAlive(us);
		}
		else if (protocol == "udp")
		{
			us = new SocketUdp("server");
			SocketLow::SetReceiveTimeout(us);
		}
		else throw "Unknown protocol";

		while (isServerActive)
		{

			size_t len = 10;
			us->receive(buffer, bufferSize);

			if (len > 0)
			{
				cout << log_time << "Got request > " << string(buffer) << endl;

				for (int i = 0; i < bufferSize - 1; i++)
				{
					if (buffer[i] == '\r' || buffer[i] == '\n' || buffer[i] == 204)
					{
						buffer[i + 1] = '\0';
						break;
					}
				}

				ParseCommand(us, string(buffer));
			}
			else
			{
				us->send("\r\n");
			}
		}
	}
	catch (boost::system::system_error &e)
	{
		error_code ec = e.code();
		cout << log_time << " : Exception caught in initialization " << ec.value() << " " << ec.category().name() << " " << ec.message() << endl;
	}

	if (us != nullptr)
	{
		delete us;
	}
}

void Server::ParseCommand(socket_ptr sock, string command)
{
	vector<string> cmds = CommandCenter::Parse(command);

	if (cmds[0] == "echo")
	{
		CmdEcho(sock, cmds);
	}
	else if (cmds[0] == "time")
	{
		CmdTime(sock, cmds);
	}
	else if (cmds[0] == "upload")
	{
		CmdReceiveFile(sock, cmds);
	}
	else if (cmds[0] == "download")
	{
		CmdSendFile(sock, cmds);
	}
	else if (cmds[0] == "halt")
	{
		isServerActive = false;
		sock->send("Server closed connection");
	}
	else
	{
		sock->send("Unrecognized command");
	}
}

void Server::CmdEcho(socket_ptr sock, vector<string> argv)
{
	string result = "";
	const uint32_t bufferSize = 512;
	char outputBuffer[bufferSize];

	for (uint32_t i = 1; i < argv.size(); i++)
	{
		result += argv[i] + " ";
	}

	strcpy_s(outputBuffer, bufferSize, result.c_str());
	sock->send(outputBuffer);
}

void Server::CmdTime(socket_ptr sock, vector<string> cmds)
{
	const uint32_t bufferSize = 512;
	char outputBuffer[bufferSize];

	string ct = LogSystem::CurrentDateTime();
	strcpy_s(outputBuffer, bufferSize, ct.c_str());

	sock->send(outputBuffer);
}

void Server::CmdSendFile(socket_ptr sock, vector<string> argv)
{
	if (argv.size() == 2)
	{
		argv.push_back(argv[1]);
	}
	if (argv.size() >= 3)
	{
		FileTransport ft(sock);
		ft.Send(argv[1], argv[2]);
	}

	// Get file ended
	sock->send("\r\n");
}


void Server::CmdReceiveFile(socket_ptr sock, vector<string> argv)
{
	if (argv.size() == 2)
	{
		argv.push_back(argv[1]);
	}
	if (argv.size() >= 3)
	{
		FileTransport ft(sock);
		ft.Receive(argv[1], argv[2]);
	}

	// Get file ended
	sock->send("\r\n");
}
