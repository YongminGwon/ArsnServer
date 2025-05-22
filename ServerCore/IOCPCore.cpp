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
	::CloseHandle(iocpHandle_);
}

bool IOCPCore::Register(shared_ptr<IOCPObject> iocpObject)
{
	return ::CreateIoCompletionPort(iocpObject->GetHandle(), iocpHandle_, 0, 0);
}

bool IOCPCore::Dispatch(uint32 timeoutMs)
{
	DWORD numOfBytes = 0;
	ULONG_PTR key = 0;
	IOCPEvent* iocpEvent = nullptr;

	/*PLOG_DEBUG << "Waiting for IOCP event...";*/

	if (::GetQueuedCompletionStatus(iocpHandle_, &numOfBytes, &key, reinterpret_cast<LPOVERLAPPED*>(&iocpEvent), timeoutMs))
	{
		//PLOG_DEBUG << "IOCP event received. Bytes: " << numOfBytes;
		shared_ptr<IOCPObject> iocpObject = iocpEvent->owner;
		iocpObject->Dispatch(iocpEvent, numOfBytes);
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
	return true;
}
