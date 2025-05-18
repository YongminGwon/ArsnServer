#pragma once
class IOCPObject
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

	bool                  Register(IOCPObject* iocpObject);
	bool                  Dispatch(uint32 timeoutMs);
private: 
	HANDLE iocpHandle_;
};