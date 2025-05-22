#pragma once
#include "IOCPCore.h"
#include "IOCPEvent.h"
#include "NetAddr.h"
#include "RecvBuf.h"

class Service;

class Session : public IOCPObject
{
	friend class Listener;
	friend class IOCPCore;
	friend class Service;

	enum
	{
		BUFFER_SIZE = 0x10000, // 64KB
	};
	
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
	void                 Send(shared_ptr<SendBuf> sendBuffer);
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
	void                 RegisterSend();

	void                 ProcessConnect();
	void                 ProcessDisconnect();
	void                 ProcessRecv(int32 numOfBytes);
	void                 ProcessSend(int32 numOfBytes);

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

	RecvBuf              recvBuf_;
	queue<shared_ptr<SendBuf>> sendQueue_;
	atomic<bool>         sendRegistered_ = false;

	mutex                lock_;
	RecvEvent            recvEvent_;
	SendEvent            sendEvent_;
	ConnectEvent         connectEvent_;
	DisconnectEvent      disconnectEvent_;
};