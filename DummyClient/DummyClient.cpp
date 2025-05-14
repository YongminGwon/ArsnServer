#include "pch.h"

int main()
{
	WSADATA wsaData;
	if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		return 0;
	}

	SOCKET clientSocket = ::socket(AF_INET, SOCK_STREAM, 0);
	if (clientSocket == INVALID_SOCKET)
	{
		int32 errCode = ::WSAGetLastError();
		cout << "Socket ErrorCode : " << errCode << endl;
		return 0;
	}
	
	u_long on = 1;
	if (::ioctlsocket(clientSocket, FIONBIO, &on) == INVALID_SOCKET)
	{
		return 0;
	}
	
	SOCKADDR_IN serverAddr;
	::memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	/*serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");*/
	::inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
	serverAddr.sin_port = htons(8000);
	
	while(true)
	{
		if (::connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr))==SOCKET_ERROR)
		{
			if (::WSAGetLastError() == WSAEWOULDBLOCK)
			{
				continue;
			}
			if (::WSAGetLastError() == WSAEISCONN)
				break;
			break;
		}
	}

	cout << "Connected To Server" << endl;

	char sendBuf[100] = "HELLO WORLD";

	while (true)
	{	
		int32 res = ::send(clientSocket, sendBuf, sizeof(sendBuf), 0);
		if (res == SOCKET_ERROR)
		{
			if (::WSAGetLastError() == WSAEWOULDBLOCK)
			{
				continue;
			}
			//ERROR
			break;
		}

		cout << "Send Data Len = " << sizeof(sendBuf) << endl;

		while(true)
		{
			char recvBuf[1024];
			int32 recvByte = ::recv(clientSocket, recvBuf, sizeof(recvBuf), 0);
			if (recvByte == SOCKET_ERROR)
			{
				if (WSAGetLastError() == WSAEWOULDBLOCK)
				{
					continue;
				}
				break;
			}
			else if (recvByte == 0)
			{
				break;
			}
			cout << "Recv Data Len = " << recvByte << endl;
			break;
		}
		this_thread::sleep_for(1s);
	}

	::closesocket(clientSocket);
	::WSACleanup();
}