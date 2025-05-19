#include "pch.h"
#include "Session.h"
#include "Service.h"

Session::Session()
{
	socket_ = SocketUtils::CreateSocket();
}

Session::~Session()
{
	SocketUtils::Close(socket_);
}

void Session::Disconnect(const WCHAR* cause)
{
}

HANDLE Session::GetHandle()
{
	return reinterpret_cast<HANDLE>(socket_);
}

void Session::Dispatch(IOCPEvent* iocpEvent, int32 numOfBytes)
{
	switch (iocpEvent->eventType_)
	{
	case EventType::Connect:
		ProcessConnect();
		break;
	case EventType::Recv:
		ProcessRecv(numOfBytes);
		break;
	case EventType::Send:
		ProcessSend(numOfBytes);
		break;
	default:
		break;
	}
}

void Session::RegisterConnect()
{
}

void Session::RegisterRecv()
{
	if (IsConnected() == false)
	{
		PLOG_WARNING << "Session::RegisterRecv failed, Is Not Connected";
		return;
	}

	recvEvent_.owner = shared_from_this();

	WSABUF wsaBuf;
	wsaBuf.buf = reinterpret_cast<char*>(recvBuf_);
	wsaBuf.len = sizeof(recvBuf_);

	DWORD numOfBytes = 0;
	DWORD flags = 0;
	if (::WSARecv(socket_, &wsaBuf, 1, &numOfBytes, &flags, &recvEvent_, nullptr) == SOCKET_ERROR)
	{
		int32 errCode = ::WSAGetLastError();
		if (errCode != WSA_IO_PENDING)
		{
			PLOG_FATAL << "RegisterRecv Failed ErrCode : " << errCode;
			recvEvent_.owner = nullptr;
			return;
		}
	}
}

void Session::RegisterSend()
{
}

void Session::ProcessConnect()
{
	connected_.store(true);
	GetService()->AddSession(GetSessionRef());

	OnConnected();

	RegisterRecv();
}

void Session::ProcessRecv(int32 numOfBytes)
{
	recvEvent_.owner = nullptr;
	if (numOfBytes == 0)
	{
		Disconnect(L"Recv 0");
		return;
	}

	cout << "Recv Data Len = " << numOfBytes << endl;

	RegisterRecv();
}

void Session::ProcessSend(int32 numOfBytes)
{
}