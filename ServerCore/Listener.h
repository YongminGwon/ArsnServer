#pragma once
#include "IOCPCore.h"
#include "NetAddr.h"

class AcceptEvent;
class ServerService;

class Listener : public IOCPObject
{
public:
	Listener() = default;
	~Listener();
public:
	virtual HANDLE       GetHandle() override;
	virtual void         Dispatch(class IOCPEvent* iocpEvent, int32 numOfBytes = 0) override;
public:
	bool                 StartAccept(shared_ptr<ServerService> service);
	void                 CloseSocket();
private:
	void                 RegisterAccept(AcceptEvent* acceptEvent);
	void                 ProcessAccept(AcceptEvent* acceptEvent);

protected:
	SOCKET socket_;
	vector<AcceptEvent*> acceptEvents_;
	shared_ptr<ServerService> service_;
};