#include "pch.h"
#include "Listener.h"
#include "Service.h"
#include "Session.h"

int main()
{
    SessionFactory factory = []() -> std::shared_ptr<Session> {
        return std::make_shared<Session>();
        };

    auto service = std::make_shared<ServerService>(
        NetAddr(L"127.0.0.1", 8000),  // ���ε��� IP�� ��Ʈ
        factory,                      // SessionFactory
        /*maxSessionCnt=*/100,          // �ִ� ���� ���� �� (����Ʈ�� ���� ����)
        /*core=*/ GCore.GetIOCPCore() // IOCPCore ���۷���
    );

    ASSERT_CRASH(service->Start());

    for (int32 i = 0; i < 5; i++)
    {
         GCore.GetThreadManager().Launch([service]()
            {
                while (true)
                {
                    service->GetIOCPCore().Dispatch(INFINITE);
                }
            }
        );
    }

    GCore.GetThreadManager().Join();
    return 0;
}