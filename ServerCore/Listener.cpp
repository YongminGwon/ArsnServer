#include "pch.h"
#include "Listener.h"
#include "SocketUtils.h"
#include "IOCPEvent.h"
#include "Session.h"

Listener::~Listener()
{
    SocketUtils::Close(socket_);
    for (AcceptEvent* acceptEvent : acceptEvents_)
    {
        delete(acceptEvent);
    }
}

HANDLE Listener::GetHandle()
{
    return reinterpret_cast<HANDLE>(socket_);
}

void Listener::Dispatch(IOCPEvent* iocpEvent, int32 numOfBytes)
{
    ASSERT_CRASH(iocpEvent->eventType_ == EventType::Accept);
    AcceptEvent* acceptEvent = static_cast<AcceptEvent*>(iocpEvent);
    ProcessAccept(acceptEvent);
}

bool Listener::StartAccept(NetAddr netAddress)
{
    socket_ = SocketUtils::CreateSocket();
    if (socket_ == INVALID_SOCKET)
    {
        return false;
    }
    if (GlobalCore::Instance().GetIOCPCore().Register(this) == false)
    {
        return false;
    }
    if (SocketUtils::SetReuseAddr(socket_, true) == false)
    {
        return false;
    }
    if (SocketUtils::SetLinger(socket_, 0, 0) == false)
    {
        return false;
    }
    if (SocketUtils::Bind(socket_, netAddress) == false)
    {
        return false;
    }
    if (SocketUtils::Listen(socket_) == false)
    {
        return false;
    }
    const int32 acceptCnt = 1;
    for (int32 i = 0; i < acceptCnt; i++)
    {
        AcceptEvent* acceptEvent = new AcceptEvent;
        acceptEvent->owner = shared_from_this();
        acceptEvents_.push_back(acceptEvent);
        RegisterAccept(acceptEvent);
    }

    PLOG(plog::info) << "Accept Registered";
    return true;
}

void Listener::CloseSocket()
{
    SocketUtils::Close(socket_);
}

void Listener::RegisterAccept(AcceptEvent* acceptEvent)
{
    const DWORD addrLen = sizeof(SOCKADDR_IN) + 16;

    shared_ptr<Session> session = make_shared<Session>();
    acceptEvent->Init();
    acceptEvent->session_ = session;

    DWORD bytesRecvd = 0;
    
    BOOL ret = SocketUtils::AcceptEx(socket_, session->GetSocket(), session->recvBuf_, 0, addrLen, addrLen, &bytesRecvd, static_cast<LPOVERLAPPED>(acceptEvent));
    int error = WSAGetLastError();
    if (!ret && error != ERROR_IO_PENDING)
    {
        RegisterAccept(acceptEvent);
    }
}

void Listener::ProcessAccept(AcceptEvent* acceptEvent)
{
    shared_ptr<Session> session = acceptEvent->session_;
    if (SocketUtils::SetUpdateAcceptSocket(session->GetSocket(), socket_) == false)
    {
        RegisterAccept(acceptEvent);
        return;
    }

    SOCKADDR_IN peerAddr = {};
    int peerLen = sizeof(peerAddr);
    if (getpeername(session->GetSocket(), reinterpret_cast<SOCKADDR*>(&peerAddr), &peerLen) == 0)
    {
        //Got peer Name
    }
    else
    {
        //plog getpeername failed
        RegisterAccept(acceptEvent);
        return;
    }

    session->SetNetAddress(NetAddr(peerAddr));

    // PLOG Client Connected

    RegisterAccept(acceptEvent);
}
