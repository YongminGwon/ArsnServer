#include "pch.h"
#include "IOCPCore.h"
#include "IOCPEvent.h"

IOCPCore::IOCPCore()
{
	iocpHandle_ = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	ASSERT_CRASH(iocpHandle_ != INVALID_HANDLE_VALUE);
}

IOCPCore::~IOCPCore()
{
}

bool IOCPCore::Register(IOCPObject* iocpObject)
{
	return ::CreateIoCompletionPort(iocpObject->GetHandle(), iocpHandle_, 0, 0);
}

bool IOCPCore::Dispatch(uint32 timeoutMs)
{
	DWORD numOfBytes = 0;
	ULONG_PTR key = 0;
	IOCPEvent* iocpEvent = nullptr;

	PLOG(plog::debug) << "Waiting for IOCP event...";

	if (::GetQueuedCompletionStatus(iocpHandle_, &numOfBytes, &key, reinterpret_cast<LPOVERLAPPED*>(&iocpEvent), timeoutMs))
	{
		PLOG(plog::debug) << "IOCP event received. Bytes: " << numOfBytes;
		shared_ptr<IOCPObject> iocpObject = iocpEvent->owner;
		iocpObject->Dispatch(iocpEvent, numOfBytes);
		return true;
	}
	else
	{
		int32 errCode = ::WSAGetLastError();
		switch (errCode)
		{
		case WAIT_TIMEOUT:
			return false;
		default:
			shared_ptr<IOCPObject> iocpObject = iocpEvent->owner;
			iocpObject->Dispatch(iocpEvent, numOfBytes);
			break;
		}
	}
	return false;
}
