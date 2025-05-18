#include "pch.h"
#include "IOCPEvent.h"

IOCPEvent::IOCPEvent(EventType type) : type_(type)
{
	Init();
}

void IOCPEvent::Init()
{
	OVERLAPPED::hEvent = 0;
	OVERLAPPED::Internal = 0;
	OVERLAPPED::InternalHigh = 0;
	OVERLAPPED::Offset = 0;
	OVERLAPPED::OffsetHigh = 0;
}
