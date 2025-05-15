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

    SOCKADDR_IN serverAddr = {};
    serverAddr.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
    serverAddr.sin_port = htons(8000);

    // blocking connect
    if (connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        cerr << "connect failed: " << WSAGetLastError() << endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }
    cout << "Connected To Server" << endl;

    // send 데이터 전송
    const char sendBuf[] = "HELLO WORLD";
    int sent = send(clientSocket, sendBuf, (int)strlen(sendBuf), 0);
    if (sent == SOCKET_ERROR)
        cerr << "send failed: " << WSAGetLastError() << endl;
    else
        cout << "Sent " << sent << " bytes" << endl;

    shutdown(clientSocket, SD_SEND);
    Sleep(100);

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
