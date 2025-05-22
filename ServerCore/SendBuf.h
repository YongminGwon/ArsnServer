#pragma once

class SendBuf : enable_shared_from_this<SendBuf>
{
public:
	SendBuf(int32 bufSize);
	~SendBuf();

	BYTE* Buffer() { return buf_.data(); }
	int32 WriteSize() { return writeSize_; }
	int32 Capacity() { return static_cast<int32>(buf_.size()); }

	void CopyData(void* data, int32 len);
private:
	vector<BYTE> buf_;
	int32 writeSize_ = 0;
};