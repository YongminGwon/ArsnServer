#include "pch.h"
#include "RecvBuf.h"

RecvBuf::RecvBuf(int32 bufSize):bufSize_(bufSize)
{
	capacity_ = bufSize * EXPAND_BUFFER;
	buf_.resize(capacity_);
}

RecvBuf::~RecvBuf()
{
}

void RecvBuf::Clean()
{
	int32 dataSize = DataSize();
	if (dataSize == 0)
	{
		readPos_ = 0;
		writePos_ = 0;
	}
	else 
	{
		if (FreeSize() < bufSize_ && readPos_ >= bufSize_)
		{
			::memcpy(&buf_, &buf_[readPos_], dataSize);
			readPos_ = 0;
			writePos_ = dataSize;
		}
	}
}

bool RecvBuf::OnRead(int32 numOfBytes)
{
	if (numOfBytes > DataSize())
	{
		return false;
	}
	readPos_ += numOfBytes;
	return true;
}

bool RecvBuf::OnWrite(int32 numOfBytes)
{
	if (numOfBytes > FreeSize())
	{
		return false;
	}
	writePos_ += numOfBytes;
	return true;
}