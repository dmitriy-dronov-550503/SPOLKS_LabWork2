#pragma once
#include <iostream>
#include <thread>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include "CommandCenter.h"
#include "UniversalSocket.h"

using namespace std;
using namespace boost::asio;

typedef UniversalSocket* socket_ptr;

class Server
{
private:
	thread* serverThread;
	void ServerThread(string protocol);
	bool isServerActive;

public:
	Server(string protocol = "tcp");
	~Server();

	void Join();

	void ParseCommand(socket_ptr sock, string command);

	void CmdEcho(socket_ptr sock, vector<string> cmds);

	void CmdTime(socket_ptr sock, vector<string> cmds);

	void CmdReceiveFile(socket_ptr sock, vector<string> cmds);

	void CmdSendFile(socket_ptr sock, vector<string> cmds);
};

