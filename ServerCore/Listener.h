#pragma once
#include "IOCPCore.h"
#include "NetAddr.h"

class AcceptEvent;

class Listener : public IOCPObject
{
public:
	Listener() = default;
	~Listener();
public:
	virtual HANDLE       GetHandle() override;
	virtual void         Dispatch(class IOCPEvent* iocpEvent, int32 numOfBytes = 0) override;
public:
	bool                 StartAccept(NetAddr netAddress);
	void                 CloseSocket();
private:
	void                 RegisterAccept(AcceptEvent* acceptEvent);
	void                 ProcessAccept(AcceptEvent* acceptEvent);

protected:
	SOCKET socket_;
	vector<AcceptEvent*> acceptEvents_;
};