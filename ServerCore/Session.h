#pragma once
#include "IOCPCore.h"
#include "IOCPEvent.h"
#include "NetAddr.h"

class Service;

class Session : public IOCPObject
{
public:
	Session();
	virtual ~Session();
public:
	void                 SetNetAddress(NetAddr address) { netAddress_ = address; }
	NetAddr              GetAddress() { return netAddress_; }
	SOCKET               GetSocket() { return socket_; }
	bool                 IsConnected() { return connected_; }
	shared_ptr<Session>  GetSessionRef() { return static_pointer_cast<Session>(shared_from_this()); }
public:
	void                 Send(BYTE* buffer, int32 len);
	bool                 Connect();
	void                 Disconnect(const WCHAR* cause);
	shared_ptr<Service>  GetService() { return service_.lock(); }
	void                 SetService(shared_ptr<Service> service) { service_ = service; }

public:
	virtual HANDLE       GetHandle() override;
	virtual void         Dispatch(class IOCPEvent* iocpEvent, int32 numOfBytes = 0) override;
public:
	bool                 RegisterConnect();
	bool                 RegisterDisconnect();
	void                 RegisterRecv();
	void                 RegisterSend(SendEvent* sendEvent);

	void                 ProcessConnect();
	void                 ProcessDisconnect();
	void                 ProcessRecv(int32 numOfBytes);
	void                 ProcessSend(SendEvent* sendEvent, int32 numOfBytes);

	BYTE                 recvBuf_[1000];
public:
	virtual void         OnConnected() {}
	virtual int32        OnRecv(BYTE* buffer, int32 len) { return len; }
	virtual void         OnSend(int32 len) {}
	virtual void         OnDisconnected() {}

private:
	SOCKET               socket_ = INVALID_SOCKET;
	NetAddr              netAddress_ = {};
	atomic<bool>         connected_ = false;
	weak_ptr<Service>    service_;

	mutex                lock_;
	RecvEvent            recvEvent_;
	ConnectEvent         connectEvent_;
	DisconnectEvent      disconnectEvent_;
};