#include "pch.h"
#include "Service.h"
#include "Session.h"

char sendData[] = "Hello World";

class ServerSession : public Session
{
public:
    virtual void OnConnected() override
    {
        cout << "Connected To Server" << endl;

        shared_ptr<SendBuf> sendBuffer = make_shared<SendBuf>(4096);
        sendBuffer->CopyData(sendData, sizeof(sendData));
        Send(sendBuffer);
    }

    virtual int32 OnRecv(BYTE* buffer, int32 len) override
    {
        //Echo
        cout << "OnRecv Len = " << len << endl;
        this_thread::sleep_for(1s);

        shared_ptr<SendBuf> sendBuffer = make_shared<SendBuf>(4096);
        sendBuffer->CopyData(sendData, sizeof(sendData));
        Send(sendBuffer);
        
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
        NetAddr(L"127.0.0.1", 8000),
        make_shared<IOCPCore>(), 
        factory,        
        100);

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