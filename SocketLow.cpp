#include "stdafx.h"
#include "SocketLow.h"

#ifndef SOCKET
#define SOCKET int
#endif

SocketLow::SocketLow()
{
}


SocketLow::~SocketLow()
{
}

void SocketLow::SetReceiveTimeout(socket_ptr sock)
{
	int timeout = 10;
        if (setsockopt(*(SOCKET*)(sock->getNativeHandle()), SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) < 0)
	{
		cout << __FUNCTION__ << " error" << endl;
	}
}

void SocketLow::SetKeepAlive(socket_ptr sock)
{
#ifdef WIN32
	tcp_keepalive alive;
	alive.onoff = 1;
	alive.keepaliveinterval = 3;
	alive.keepalivetime = 3000;
	DWORD ret;
        DWORD Res = WSAIoctl(*(SOCKET*)(sock->getNativeHandle()), SIO_KEEPALIVE_VALS, &alive, sizeof(alive), NULL, 0, &ret, NULL, NULL);
	if (Res == SOCKET_ERROR)
	{
		cout << "error with keepalive" << endl;
	}
#else
	int keepalive = 1;
        if (setsockopt(*(SOCKET*)(sock->getNativeHandle()), SOL_SOCKET, SO_KEEPALIVE, &keepalive, (socklen_t)sizeof(keepalive)) < 0)
	{

		perror("setsockopt(keepalive)");
	}
	int keepcnt = 1;
        if (setsockopt(*(SOCKET*)(sock->getNativeHandle()), IPPROTO_TCP, TCP_KEEPCNT, &keepcnt, (socklen_t)sizeof(int))<0)
	{

		perror("setsockopt(keepcnt)");
	}
	int keepidle = 30;
        if (setsockopt(*(SOCKET*)(sock->getNativeHandle()), IPPROTO_TCP, TCP_KEEPIDLE, &keepidle, (socklen_t)sizeof(int))<0)
	{

                perror("setsockopt(keepidle)");
	}
	int keepintvl = 1;
        if (setsockopt(*(SOCKET*)(sock->getNativeHandle()), IPPROTO_TCP, TCP_KEEPINTVL, &keepintvl, (socklen_t)sizeof(int))<0)
	{
		perror("setsockopt(keepalive)");
	}
#endif
}
