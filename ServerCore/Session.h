#pragma once
#include "IOCPCore.h"
#include "IOCPEvent.h"
#include "NetAddr.h"

class Session : public IOCPObject
{
public:
	Session();
	virtual ~Session();
public:
	void SetNetAddress(NetAddr address) { netAddress_ = address; }
	NetAddr GetAddress() { return netAddress_; }
	SOCKET GetSocket() { return socket_; }
public:
	virtual HANDLE GetHandle() override;
	virtual void Dispatch(class IOCPEvent* iocpEvent, int32 numOfBytes = 0) override;
public:
	char recvBuf_[1000];
private:
	SOCKET              socket_ = INVALID_SOCKET;
	NetAddr             netAddress_ = {};
	atomic<bool>        connected_ = false;
};