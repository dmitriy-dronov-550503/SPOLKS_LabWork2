// LabWork2.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include "Server.h"
#include "Client.h"
#include "CommandCenter.h"
#include <iostream>
#include <string>
using namespace std;

int main(int argc, char* argv[])
{
	string type = "";
	string protocol = "";

	if (argc >= 2 && argv[1] != nullptr)
	{
		type = argv[1];
	}
	else
	{
		cout << "-s server -c client: ";
		getline(cin, type);
	}

	if (argc >= 3 && argv[2] != nullptr)
	{
		protocol = argv[2];
	}
	else
	{
		cout << "udp/tcp: ";
		getline(cin, protocol);
	}

	if (type == "-s")
	{
		cout << "Start server" << endl;
		Server server(protocol);
	}

	if (type == "-c")
	{
		cout << "Start client" << endl;

		string ipAddress = "127.0.0.1";

		if (argc >= 4 && argv[3] != nullptr)
		{
			ipAddress = argv[3];
		}

		Client client(protocol, ipAddress);

	}

	return 0;
}