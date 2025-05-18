#pragma once

class Session;

enum class EventType : uint8
{
	Connect,
	Accept,
	Recv,
	Send,
};

class IOCPEvent : public OVERLAPPED
{
public:
	IOCPEvent(EventType type);

	void Init();
	EventType GetType() { return type_; }

protected:
	EventType type_;
};

class ConnectEvent : public IOCPEvent
{
public:
	ConnectEvent() :IOCPEvent(EventType::Connect) {}
};

class AcceptEvent : public IOCPEvent
{
public:
	AcceptEvent() :IOCPEvent(EventType::Accept) {}

	void SetSession(Session* session) { session_ = session; }
	Session* GetSession() { return session_; }
private:
	Session* session_ = nullptr;
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
};