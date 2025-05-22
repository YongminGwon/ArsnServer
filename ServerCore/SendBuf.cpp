#include "pch.h"
#include "SendBuf.h"

SendBuf::SendBuf(int32 bufSize)
{
	buf_.resize(bufSize);
}

SendBuf::~SendBuf()
{
}

void SendBuf::CopyData(void* data, int32 len)
{
	ASSERT_CRASH(Capacity() >= len);
	::memcpy(buf_.data(), data, len);
	writeSize_ = len;
}