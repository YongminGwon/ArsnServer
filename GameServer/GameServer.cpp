#include "pch.h"
#include "Service.h"
#include "Session.h"

class GameSession : public Session
{
public:
    virtual int32 OnRecv(BYTE* buffer, int32 len) override
    {
        //Echo
        cout << "OnRecv Len = " << len << endl;
        Send(buffer, len);
        return len;
    }
    virtual void OnSend(int32 len) override
    {
        cout << "OnSend Len = " << len << endl;
    }
};

int main()
{
    SessionFactory factory = []() -> std::shared_ptr<Session> {
        return std::make_shared<GameSession>();
        };

    auto service = std::make_shared<ServerService>(
        NetAddr(L"127.0.0.1", 8000),  // ���ε��� IP�� ��Ʈ
        factory,                      // SessionFactory
        /*core=*/ make_shared<IOCPCore>(), // IOCPCore ���۷���
        /*maxSessionCnt=*/100          // �ִ� ���� ���� ��
    );

    ASSERT_CRASH(service->Start());

    for (int32 i = 0; i < 5; i++)
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