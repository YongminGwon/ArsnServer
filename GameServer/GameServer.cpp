#include "pch.h"

int main()
{
    // 1) Winsock 초기화 및 확장 함수 로딩
    SocketUtils::Init();

    // 2) 리스닝 소켓 생성
    SOCKET listenSock = SocketUtils::CreateSocket();
    if (listenSock == INVALID_SOCKET)
    {
        std::cerr << "CreateSocket failed: " << WSAGetLastError() << "\n";
        return -1;
    }

    // 3) SO_REUSEADDR 설정 (포트 바인딩 오류 방지)
    SocketUtils::SetReuseAddr(listenSock, TRUE);

    // 4) 로컬 0.0.0.0:8000 바인드
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

    // 6) 클라이언트 접속 대기 및 처리
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

        // 7) 데이터 수신 (최대 100바이트)
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

        // 8) 클라이언트 연결 종료
        ::closesocket(clientSock);
        std::cout << "Client disconnected.\n";
    }

    // 9) 정리
    SocketUtils::Close(listenSock);
    SocketUtils::Clear();
    return 0;
}