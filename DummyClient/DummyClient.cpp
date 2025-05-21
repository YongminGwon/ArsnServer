#include "pch.h"
#include "Service.h"
#include "Session.h"

char sendBuffer[] = "Hello World";

class ServerSession : public Session
{
public:
    virtual void OnConnected() override
    {
        cout << "Connected To Server" << endl;
        Send((BYTE*)sendBuffer, sizeof(sendBuffer));
    }

    virtual int32 OnRecv(BYTE* buffer, int32 len) override
    {
        //Echo
        cout << "OnRecv Len = " << len << endl;
        this_thread::sleep_for(1s);
        Send((BYTE*)sendBuffer, sizeof(sendBuffer));
        return len;
    }

    virtual void OnSend(int32 len) override
    {
        cout << "OnSend Len = " << len << endl;
    }

    virtual void OnDisconnected() override
    {
        cout << "Disconnected" << endl;
    }
};

int main()
{
    this_thread::sleep_for(1s);

    SessionFactory factory = []() -> std::shared_ptr<Session> {
        return std::make_shared<ServerSession>();
        };

    auto service = std::make_shared<ClientService>(
        NetAddr(L"127.0.0.1", 8000),  // 바인딩할 IP와 포트
        factory,     
        /*core=*/ make_shared<IOCPCore>(), // IOCPCore 레퍼런스
        /*maxSessionCnt=*/100          // 최대 동시 세션 수 (디폴트라 생략 가능)
    );

    ASSERT_CRASH(service->Start());

    for (int32 i = 0; i < 2; i++)
    {
        GCore.GetThreadManager().Launch([service]()
            {
                while (true)
                {
                    service->GetIOCPCore()->Dispatch(INFINITE);
                }
            }
        );
    }

    GCore.GetThreadManager().Join();
    return 0;
}