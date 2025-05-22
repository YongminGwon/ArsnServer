#pragma once

class RecvBuf
{
	enum{EXPAND_BUFFER = 10};
public:
	RecvBuf(int32 bufSize);
	~RecvBuf();

	void                Clean();
	bool                OnRead(int32 numOfBytes);
	bool                OnWrite(int32 numOfBytes);

	BYTE*               ReadPos(){ return &buf_[readPos_]; }
	BYTE*               WritePos(){ return &buf_[writePos_]; }
	int32               DataSize(){ return writePos_ - readPos_; }
	int32               FreeSize(){ return capacity_ - writePos_; }
private:
	int32               capacity_ = 0;
	int32               bufSize_ = 0;
	int32               readPos_ = 0;
	int32               writePos_ = 0;
	vector<BYTE>        buf_;
};