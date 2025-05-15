#include "pch.h"
#include "NetAddr.h"

NetAddr::NetAddr(SOCKADDR_IN sockAddr) : sockAddr_(sockAddr)
{
}

NetAddr::NetAddr(wstring ip, uint16 port)
{
    ::memset(&sockAddr_, 0, sizeof(sockAddr_));
    sockAddr_.sin_family = AF_INET;
    sockAddr_.sin_addr = Ip2Addr(ip.c_str());
    sockAddr_.sin_port = ::htons(port);
}

wstring NetAddr::GetIpAddress(SOCKADDR_IN sockAddr)
{
    WCHAR buffer[100];
    ::InetNtopW(AF_INET, &sockAddr_.sin_addr, buffer, sizeof(buffer)/sizeof(WCHAR));
    return wstring(buffer);
}

IN_ADDR NetAddr::Ip2Addr(const WCHAR* ip)
{
    IN_ADDR address;
    ::InetPtonW(AF_INET, ip, &address);
    return address;
}
