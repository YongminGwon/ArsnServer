#pragma once
#include "Session.h"
#include "NetAddr.h"

class Listener;

enum class ServiceType : uint8
{
	Server,
	Client,
};

using SessionFactory = function<shared_ptr<Session>(void)>;

class Service : public enable_shared_from_this<Service>
{
public:
	Service(ServiceType type, NetAddr address, SessionFactory factory, int32 maxSessionCnt = 1, IOCPCore& core = GCore.GetIOCPCore());
	virtual ~Service() = default;

	virtual bool                     Start() = 0;
	virtual void                     CloseService();

	bool                             CanStart() const { return sessionFactory_ != nullptr; }
	int32                            GetCurrentSessionCnt() const { return sessionCnt_; }
	int32                            GetMaxSessionCnt() const { return maxSessionCnt_; }
	ServiceType                      GetServiceType() const { return type_; }
	NetAddr                          GetNetAddr() const { return netAddress_; }
	IOCPCore&                        GetIOCPCore() const { return iocpCore_; }
	
	shared_ptr<Session>              CreateSession();
	void                             AddSession(shared_ptr<Session> session);
	void                             ReleaseSession(shared_ptr<Session> session);

protected:
	IOCPCore&                        iocpCore_;
	mutex                            lock_;
	ServiceType                      type_;
	NetAddr                          netAddress_ = {};
	set<shared_ptr<Session>>         sessions_;
	int32                            sessionCnt_ = 0;
	int32                            maxSessionCnt_ = 0;
	SessionFactory                   sessionFactory_;
};

class ClientService : public Service
{
public:
	ClientService(NetAddr targetAddress, SessionFactory factory, int32 maxSessionCnt, IOCPCore& core);
	virtual ~ClientService() {}

	virtual bool                      Start() override;
};

class ServerService : public Service
{
public:
	ServerService(NetAddr targetAddress, SessionFactory factory, int32 maxSessionCnt, IOCPCore& core);
	virtual ~ServerService() {}

	virtual bool                      Start() override;
	virtual void                      CloseService() override;

private:
	shared_ptr<Listener>              listener_ = nullptr;
};