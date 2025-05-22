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

void Session::Send(shared_ptr<SendBuf> sendBuffer)
{
	if (IsConnected() == false)
	{
		return;
	}
	bool registerSend = false;
	{
		scoped_lock guard(lock_);
		sendQueue_.push(sendBuffer);

		if (sendRegistered_.exchange(true) == false)
		{
			registerSend = true;
		}
	}
	if (registerSend)
		RegisterSend();
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
		ProcessSend(numOfBytes);
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

	connectEvent_.Init();
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
	disconnectEvent_.Init();
	disconnectEvent_.owner = shared_from_this();
	if (SocketUtils::DisconnectEx(socket_, &disconnectEvent_, TF_REUSE_SOCKET, 0) == false)
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

	recvEvent_.Init();
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
		}
	}
}

void Session::RegisterSend()
{
	if (IsConnected() == false)
	{
		PLOG_DEBUG << "Session::RegisterSend Failed : Disconnected";
		return;
	}

	sendEvent_.Init();
	sendEvent_.owner = shared_from_this();

	{
		mutex mutex;
		scoped_lock scplock(mutex);

		int32 writeSize = 0;
		while (sendQueue_.empty() == false)
		{
			shared_ptr<SendBuf> sendBuffer = sendQueue_.front();
			writeSize += sendBuffer->WriteSize();

			sendQueue_.pop();
			sendEvent_.sendBuffers.push_back(sendBuffer);
		}
	}

	vector<WSABUF> wsaBufs;
	wsaBufs.reserve(sendEvent_.sendBuffers.size());
	for (auto sendBuffer : sendEvent_.sendBuffers)
	{
		WSABUF wsaBuf;
		wsaBuf.buf = reinterpret_cast<char*>(sendBuffer->Buffer());
		wsaBuf.len = static_cast<LONG>(sendBuffer->WriteSize());
		wsaBufs.push_back(wsaBuf);
	}

	DWORD numOfBytes = 0;
	if (::WSASend(socket_, wsaBufs.data(), static_cast<DWORD>(wsaBufs.size()), &numOfBytes, 0, &sendEvent_, nullptr) == SOCKET_ERROR)
	{
		int32 errCode = ::WSAGetLastError();
		if (errCode != WSA_IO_PENDING)
		{
			PLOG_FATAL << "RegisterSend Failed";
			sendEvent_.owner = nullptr;
			sendEvent_.sendBuffers.clear();
			sendRegistered_.store(false);
		}
	}
}

void Session::ProcessConnect()
{
	connectEvent_.owner = nullptr;

	connected_ = true;
	GetService()->AddSession(GetSessionRef());

	OnConnected();

	RegisterRecv();
}

void Session::ProcessDisconnect()
{
	disconnectEvent_.owner = nullptr;
	OnDisconnected();
	GetService()->ReleaseSession(GetSessionRef());
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

void Session::ProcessSend(int32 numOfBytes)
{
	sendEvent_.owner = nullptr;
	sendEvent_.sendBuffers.clear();

	if (numOfBytes == 0)
	{
		Disconnect(L"Send 0");
		return;
	}
	OnSend(numOfBytes);
	mutex mutex;
	scoped_lock scplock(mutex);
	if (sendQueue_.empty())
	{
		sendRegistered_.store(false);
	}
	else
	{
		RegisterSend();
	}
}