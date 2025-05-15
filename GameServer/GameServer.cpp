#include "pch.h"

int main()
{
    // 1) Winsock �ʱ�ȭ �� Ȯ�� �Լ� �ε�
    SocketUtils::Init();

    // 2) ������ ���� ����
    SOCKET listenSock = SocketUtils::CreateSocket();
    if (listenSock == INVALID_SOCKET)
    {
        std::cerr << "CreateSocket failed: " << WSAGetLastError() << "\n";
        return -1;
    }

    // 3) SO_REUSEADDR ���� (��Ʈ ���ε� ���� ����)
    SocketUtils::SetReuseAddr(listenSock, TRUE);

    // 4) ���� 0.0.0.0:8000 ���ε�
    if (!SocketUtils::BindAnyAddress(listenSock, 8000))
    {
        std::cerr << "Bind failed: " << WSAGetLastError() << "\n";
        SocketUtils::Close(listenSock);
        return -1;
    }

    // 5) Listen
    if (!SocketUtils::Listen(listenSock, SOMAXCONN))
    {
        std::cerr << "Listen failed: " << WSAGetLastError() << "\n";
        SocketUtils::Close(listenSock);
        return -1;
    }

    std::cout << "GameServer listening on port 8000...\n";

    // 6) Ŭ���̾�Ʈ ���� ��� �� ó��
    while (true)
    {
        // blocking accept
        SOCKADDR_IN clientAddr;
        int addrLen = sizeof(clientAddr);
        SOCKET clientSock = ::accept(listenSock,
            reinterpret_cast<SOCKADDR*>(&clientAddr),
            &addrLen);
        if (clientSock == INVALID_SOCKET)
        {
            std::cerr << "accept failed: " << WSAGetLastError() << "\n";
            break;
        }

        WCHAR ipStr[INET_ADDRSTRLEN] = {};
        if (InetNtop(             // now calls InetNtopW
            AF_INET,
            &clientAddr.sin_addr,
            ipStr,
            INET_ADDRSTRLEN
        ) != nullptr)
        {
            std::wcout << L"Client connected: "
                << ipStr
                << L":" << ntohs(clientAddr.sin_port)
                << L"\n";
        }
        else
        {
            std::wcerr << L"InetNtopW failed: " << WSAGetLastError() << L"\n";
        }

        // 7) ������ ���� (�ִ� 100����Ʈ)
        char buffer[100] = {};
        int recvLen = ::recv(clientSock, buffer, (int)sizeof(buffer) - 1, 0);
        if (recvLen > 0)
        {
            buffer[recvLen] = '\0';
            std::cout << "Received (" << recvLen << " bytes): "
                << buffer << "\n";
        }
        else
        {
            std::cout << "recv returned " << recvLen
                << ", error " << WSAGetLastError() << "\n";
        }

        // 8) Ŭ���̾�Ʈ ���� ����
        ::closesocket(clientSock);
        std::cout << "Client disconnected.\n";
    }

    // 9) ����
    SocketUtils::Close(listenSock);
    SocketUtils::Clear();
    return 0;
}