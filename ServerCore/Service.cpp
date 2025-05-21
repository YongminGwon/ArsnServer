#include "pch.h"
#include "Service.h"
#include "Session.h"
#include "Listener.h"

Service::Service(ServiceType type, NetAddr address, SessionFactory factory, const shared_ptr<IOCPCore>& core, int32 maxSessionCnt)
	:type_(type), netAddress_(std::move(address)), sessionFactory_(std::move(factory)), iocpCore_(core), maxSessionCnt_(maxSessionCnt)
{
}

void Service::CloseService()
{
}

shared_ptr<Session> Service::CreateSession()
{
	shared_ptr<Session> session = sessionFactory_();
	session->SetService(shared_from_this());
	if (iocpCore_->Register(session) == false)
	{
		return nullptr;
	}
	return session;
}

void Service::AddSession(shared_ptr<Session> session)
{
	scoped_lock guard(lock_);
	sessionCnt_++;
	sessions_.insert(session);
}

void Service::ReleaseSession(shared_ptr<Session> session)
{
	scoped_lock guard(lock_);
	ASSERT_CRASH(sessions_.erase(session) != 0);
	sessionCnt_--;
}

ClientService::ClientService(NetAddr targetAddress, SessionFactory factory, const shared_ptr<IOCPCore>& core, int32 maxSessionCnt)
	:Service(ServiceType::Client, targetAddress, factory, core, maxSessionCnt)
{
}

bool ClientService::Start()
{
	if (CanStart() == false)
	{
		return false;
	}

	const int32 sessionCnt = GetMaxSessionCnt();
	for (int32 i = 0; i < sessionCnt; i++)
	{
		shared_ptr<Session> session = CreateSession();
		if (session->Connect() == false)
		{
			return false;
		}
	}
	return true;
}

ServerService::ServerService(NetAddr targetAddress, SessionFactory factory, const shared_ptr<IOCPCore>& core, int32 maxSessionCnt)
	:Service(ServiceType::Server, targetAddress, factory, core, maxSessionCnt)
{
}

bool ServerService::Start()
{
	if (CanStart() == false)
	{
		return false;
	}
	listener_ = make_shared<Listener>();
	if (listener_ == nullptr)
	{
		return false;
	}

	shared_ptr<ServerService> service = static_pointer_cast<ServerService>(shared_from_this());
	if (listener_->StartAccept(service) == false)
	{
		return false;
	}
	return true;
}

void ServerService::CloseService()
{
	Service::CloseService();
}
