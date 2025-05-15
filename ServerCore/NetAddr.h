#pragma once
class NetAddr
{
public:
	NetAddr() = default;
	NetAddr(SOCKADDR_IN sockAddr);
	NetAddr(wstring ip, uint16 port);

	SOCKADDR_IN&          GetSockAddr() { return sockAddr_; }
	wstring               GetIpAddress(SOCKADDR_IN sockAddr);
	uint16                GetPort() { return ::ntohs(sockAddr_.sin_port); }

	static IN_ADDR        Ip2Addr(const WCHAR* ip);
private:
	SOCKADDR_IN           sockAddr_ = {};
};