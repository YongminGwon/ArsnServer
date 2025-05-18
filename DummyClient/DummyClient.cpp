#include "pch.h"

int main()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        return 1;

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET)
    {
        cerr << "socket failed: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }

    // 소켓 옵션 설정
    BOOL optVal = TRUE;
    if (setsockopt(clientSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&optVal, sizeof(optVal)) == SOCKET_ERROR)
    {
        cerr << "setsockopt failed: " << WSAGetLastError() << endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    // LINGER 옵션 설정
    LINGER linger;
    linger.l_onoff = 1;
    linger.l_linger = 0;
    if (setsockopt(clientSocket, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof(linger)) == SOCKET_ERROR)
    {
        cerr << "setsockopt failed: " << WSAGetLastError() << endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    SOCKADDR_IN serverAddr = {};
    serverAddr.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
    serverAddr.sin_port = htons(8000);

    // 연결 시도 (타임아웃 설정)
    fd_set writeSet;
    FD_ZERO(&writeSet);
    FD_SET(clientSocket, &writeSet);

    timeval timeout;
    timeout.tv_sec = 5;  // 5초 타임아웃
    timeout.tv_usec = 0;

    if (connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        if (WSAGetLastError() != WSAEWOULDBLOCK)
        {
            cerr << "connect failed: " << WSAGetLastError() << endl;
            closesocket(clientSocket);
            WSACleanup();
            return 1;
        }

        // select로 연결 완료 대기
        int result = select(0, NULL, &writeSet, NULL, &timeout);
        if (result == 0)
        {
            cerr << "Connection timeout" << endl;
            closesocket(clientSocket);
            WSACleanup();
            return 1;
        }
        else if (result == SOCKET_ERROR)
        {
            cerr << "select failed: " << WSAGetLastError() << endl;
            closesocket(clientSocket);
            WSACleanup();
            return 1;
        }
    }

    cout << "Connected To Server" << endl;

    // 데이터 송신
    const char sendBuf[] = "HELLO WORLD";
    int sent = send(clientSocket, sendBuf, (int)strlen(sendBuf), 0);
    if (sent == SOCKET_ERROR)
    {
        cerr << "send failed: " << WSAGetLastError() << endl;
    }
    else
    {
        cout << "Sent " << sent << " bytes" << endl;
    }

    // 연결 종료 전 잠시 대기
    Sleep(100);

    // 정상적인 종료
    shutdown(clientSocket, SD_BOTH);
    closesocket(clientSocket);
    WSACleanup();
    return 0;
}