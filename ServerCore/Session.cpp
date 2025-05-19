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

void Session::Send(BYTE* buffer, int32 len)
{
	SendEvent* sendEvent = new SendEvent;
	sendEvent->owner = shared_from_this();
	sendEvent->sendBuf.resize(len);
	::memcpy(sendEvent->sendBuf.data(), buffer, len);

	RegisterSend(sendEvent);
}

void Session::Disconnect(const WCHAR* cause)
{
	if (connected_.exchange(false) == false)
	{
		return;
	}

	PLOG_DEBUG << "Disconnect: " << cause;
	OnDisconnected();
	SocketUtils::Close(socket_);
	GetService()->ReleaseSession(GetSessionRef());
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
		ProcessSend(static_cast<SendEvent*>(iocpEvent), numOfBytes);
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

//TODO : Scatter and Gather
void Session::RegisterSend(SendEvent* sendEvent)
{
	if (IsConnected() == false)
	{
		PLOG_DEBUG << "Session::RegisterSend Failed : Disconnected";
		return;
	}
	WSABUF wsaBuf;
	wsaBuf.buf = (char*)sendEvent->sendBuf.data();
	wsaBuf.len = (ULONG)sendEvent->sendBuf.size();

	DWORD numOfBytes = 0;
	if (::WSASend(socket_, &wsaBuf, 1, &numOfBytes, 0, sendEvent, nullptr) == SOCKET_ERROR)
	{
		int32 errCode = ::WSAGetLastError();
		if (errCode != WSA_IO_PENDING)
		{
			PLOG_FATAL << "RegisterSend Failed";
			sendEvent->owner = nullptr;
			delete sendEvent;
		}
	}
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

	OnRecv(recvBuf_, numOfBytes);

	RegisterRecv();
}

void Session::ProcessSend(SendEvent* sendEvent, int32 numOfBytes)
{
	sendEvent->owner = nullptr;
	delete sendEvent;

	if (numOfBytes == 0)
	{
		Disconnect(L"Send 0");
		return;
	}
	OnSend(numOfBytes);
}