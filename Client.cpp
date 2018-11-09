#include "stdafx.h"
#include <fstream>
#include <chrono>

#include "Client.h"
#include "CommandCenter.h"
#include "LogSystem.h"
#include "FileTransport.h"
#include "SocketLow.h"
#include "UniversalSocket.h"

using boost::asio::ip::udp;

Client::Client(string protocol, string ipAddress)
{
	ClientThread(protocol, ipAddress);
}

Client::~Client()
{
}

void Client::Join()
{
	clientThread->join();
}

void Client::ClientThread(string protocol, string ipAddress)
{
	cout << "GOT IP: " << ipAddress << endl;

	char buffer[512];

	UniversalSocket* us = nullptr;

	try
	{
		if (protocol == "tcp")
		{
			us = new SocketTcp("client");
			SocketLow::SetKeepAlive(us);
		}
		else if (protocol == "udp")
		{
			us = new SocketUdp("client");
			SocketLow::SetReceiveTimeout(us);
		}
		else throw "Unknown protocol";

		while (true)
		{
			string str;
			cout << endl << "> ";
			getline(cin, str);

			if (!str.empty())
			{
				vector<string> cmds = CommandCenter::Parse(str);

				strcpy(buffer, str.c_str());

				us->send(buffer);

				if (cmds[0] == "upload") {
					SendFile(us, cmds);
				}
				else if (cmds[0] == "download") {
					ReceiveFile(us, cmds);
				}

				us->receive(buffer, 512);
				cout << buffer << endl;
			}

			if (str == "exit" || str == "halt") break;
		}
	}
	catch (std::exception& e)
	{
		cout << "Exception " << e.what() << endl;
	}

	if (us != nullptr)
	{
		delete us;
	}
}

void Client::SendFile(socket_ptr sock, vector<string> argv)
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
}

void Client::ReceiveFile(socket_ptr sock, vector<string> argv)
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
}
