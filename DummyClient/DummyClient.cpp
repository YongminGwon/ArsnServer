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

    // ���� �ɼ� ����
    BOOL optVal = TRUE;
    if (setsockopt(clientSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&optVal, sizeof(optVal)) == SOCKET_ERROR)
    {
        cerr << "setsockopt failed: " << WSAGetLastError() << endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    // LINGER �ɼ� ����
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

    // ���� �õ� (Ÿ�Ӿƿ� ����)
    fd_set writeSet;
    FD_ZERO(&writeSet);
    FD_SET(clientSocket, &writeSet);

    timeval timeout;
    timeout.tv_sec = 5;  // 5�� Ÿ�Ӿƿ�
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

        // select�� ���� �Ϸ� ���
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

    // ������ �۽�
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

    // ���� ���� �� ��� ���
    Sleep(100);

    // �������� ����
    shutdown(clientSocket, SD_BOTH);
    closesocket(clientSocket);
    WSACleanup();
    return 0;
}