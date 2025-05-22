#include "pch.h"
#include "Listener.h"
#include "SocketUtils.h"
#include "IOCPEvent.h"
#include "Session.h"
#include "Service.h"

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

bool Listener::StartAccept(shared_ptr<ServerService> service)
{
    service_ = service;
    if (service_ == nullptr)
    {
        return false;
    }
    socket_ = SocketUtils::CreateSocket();
    if (socket_ == INVALID_SOCKET)
    {
        return false;
    }
    if (service_->GetIOCPCore()->Register(shared_from_this()) == false)
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
    if (SocketUtils::Bind(socket_, service_->GetNetAddr()) == false)
    {
        return false;
    }
    if (SocketUtils::Listen(socket_) == false)
    {
        return false;
    }
    const int32 acceptCnt = service_->GetMaxSessionCnt();
    for (int32 i = 0; i < acceptCnt; i++)
    {
        AcceptEvent* acceptEvent = new AcceptEvent;
        acceptEvent->owner = shared_from_this();
        acceptEvents_.push_back(acceptEvent);
        RegisterAccept(acceptEvent);
    }

    /*PLOG_INFO << "Accept Registered";*/
    return true;
}

void Listener::CloseSocket()
{
    SocketUtils::Close(socket_);
}

void Listener::RegisterAccept(AcceptEvent* acceptEvent)
{
    shared_ptr<Session> session = service_->CreateSession();
    acceptEvent->Init();
    acceptEvent->session_ = session;

    DWORD bytesRecved = 0;
    if (SocketUtils::AcceptEx(socket_, session->GetSocket(), session->recvBuf_.WritePos(), 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &bytesRecved, static_cast<LPOVERLAPPED>(acceptEvent)) == false)
    {
        const int32 errCode = ::WSAGetLastError();
        if (errCode != WSA_IO_PENDING)
        {
            RegisterAccept(acceptEvent);
        }
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
    if (getpeername(session->GetSocket(), reinterpret_cast<SOCKADDR*>(&peerAddr), &peerLen) == SOCKET_ERROR)
    {
        RegisterAccept(acceptEvent);
        return;
    }

    session->SetNetAddress(NetAddr(peerAddr));
    session->ProcessConnect();
    // PLOG Client Connected

    RegisterAccept(acceptEvent);
}
