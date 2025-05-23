#pragma once

class Session;

enum class EventType : uint8
{
	Connect,
	Disconnect,
	Accept,
	Recv,
	Send,
};

class IOCPEvent : public OVERLAPPED
{
public:
	IOCPEvent(EventType type);

	void                      Init();
	EventType                 eventType_;
	shared_ptr<IOCPObject>    owner;
};

class ConnectEvent : public IOCPEvent
{
public:
	ConnectEvent() :IOCPEvent(EventType::Connect) {}
};

class DisconnectEvent : public IOCPEvent
{
public:
	DisconnectEvent() :IOCPEvent(EventType::Disconnect) {}
};

class AcceptEvent : public IOCPEvent
{
public:
	AcceptEvent() :IOCPEvent(EventType::Accept) {}
	shared_ptr<Session>       session_ = nullptr;
};

class RecvEvent : public IOCPEvent
{
public:
	RecvEvent() :IOCPEvent(EventType::Recv) {}
};

class SendEvent : public IOCPEvent
{
public:
	SendEvent() :IOCPEvent(EventType::Send) {}

	vector<shared_ptr<SendBuf>> sendBuffers;
};