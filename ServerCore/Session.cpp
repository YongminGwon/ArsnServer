#include "pch.h"
#include "Session.h"
#include "Service.h"

Session::Session() : recvBuf_(BUFFER_SIZE)
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

bool Session::Connect()
{
	return RegisterConnect();
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

	RegisterDisconnect();
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
	case EventType::Disconnect:
		ProcessDisconnect();
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

bool Session::RegisterConnect()
{
	if (IsConnected())
	{
		return false;
	}
	if (GetService()->GetServiceType() != ServiceType::Client)
	{
		return false;
	}
	if (SocketUtils::SetReuseAddr(socket_, true) == false)
	{
		return false;
	}
	if (SocketUtils::BindAnyAddress(socket_, 0) == false)
	{
		return false;
	}

	connectEvent_.owner = shared_from_this();
	
	DWORD numOfBytes = 0;
	SOCKADDR_IN sockAddr = GetService()->GetNetAddr().GetSockAddr();
	if (SocketUtils::ConnectEx(socket_, reinterpret_cast<SOCKADDR*>(&sockAddr), sizeof(sockAddr), nullptr, 0, &numOfBytes, &connectEvent_) == false)
	{
		int32 errCode = ::WSAGetLastError();
		if (errCode != WSA_IO_PENDING)
		{
			connectEvent_.owner = nullptr;
			return false;
		}
	}

	return true;
}

bool Session::RegisterDisconnect()
{
	disconnectEvent_.owner = shared_from_this();
	if (SocketUtils::DisconnectEx(socket_, &disconnectEvent_, TF_REUSE_SOCKET, 0))
	{
		int32 errCode = ::WSAGetLastError();
		if (errCode != WSA_IO_PENDING)
		{
			disconnectEvent_.owner = nullptr;
			return false;
		}
	}
	return true;
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
	wsaBuf.buf = reinterpret_cast<char*>(recvBuf_.WritePos());
	wsaBuf.len = recvBuf_.FreeSize();

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
	connectEvent_.owner = nullptr;

	connected_.store(true);
	GetService()->AddSession(GetSessionRef());

	OnConnected();

	RegisterRecv();
}

void Session::ProcessDisconnect()
{
	disconnectEvent_.owner = nullptr;
}

void Session::ProcessRecv(int32 numOfBytes)
{
	recvEvent_.owner = nullptr;

	if (numOfBytes == 0)
	{
		Disconnect(L"Recv 0");
		return;
	}

	if (recvBuf_.OnWrite(numOfBytes) == false)
	{
		Disconnect(L"OnWrite Failed");
		return;
	}

	int32 dataSize = recvBuf_.DataSize();
	int32 processDataLen = OnRecv(recvBuf_.ReadPos(), dataSize);

	if (processDataLen < 0 || dataSize < processDataLen || recvBuf_.OnRead(processDataLen) == false)
	{
		Disconnect(L"OnRead Failed");
		return;
	}

	recvBuf_.Clean();

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