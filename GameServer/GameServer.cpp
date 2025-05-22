#include "pch.h"
#include "Service.h"
#include "Session.h"
#include "GameSession.h"

int main()
{
    SessionFactory factory = []() -> std::shared_ptr<Session> {
        return std::make_shared<GameSession>();
        };

    auto service = std::make_shared<ServerService>(
        NetAddr(L"127.0.0.1", 8000),
        make_shared<IOCPCore>(),
        factory,
        100);

    ASSERT_CRASH(service->Start());

    for (int32 i = 0; i < 5; i++)
    {
        GCore.GetThreadManager().Launch([service]()
            {
                while (true)
                {
                    service->GetIOCPCore()->Dispatch();
                }
            });
    }

    GCore.GetThreadManager().Join();
    return 0;
}