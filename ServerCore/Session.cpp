#include "pch.h"
#include "Session.h"

Session::Session()
{
	socket_ = SocketUtils::CreateSocket();
}

Session::~Session()
{
	SocketUtils::Close(socket_);
}

HANDLE Session::GetHandle()
{
	return reinterpret_cast<HANDLE>(socket_);
}

void Session::Dispatch(IOCPEvent* iocpEvent, int32 numOfBytes)
{
}
