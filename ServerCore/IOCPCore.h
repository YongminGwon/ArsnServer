#pragma once
class IOCPObject : public enable_shared_from_this<IOCPObject>
{
public:
	virtual HANDLE        GetHandle() = 0;
	virtual void          Dispatch(class IOCPEvent* IOCPEvent, int32 numOfBytes) = 0;
};

class IOCPCore
{
public:
	IOCPCore();
	~IOCPCore();

	HANDLE                GetHandle() { return iocpHandle_; }

	bool                  Register(shared_ptr<IOCPObject> iocpObject);
	bool                  Dispatch(uint32 timeoutMs);
private: 
	HANDLE iocpHandle_;
};