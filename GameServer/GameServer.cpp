#include "pch.h"
#include "Listener.h"

int main()
{
    auto& core = GlobalCore::Instance();
    Listener listener;
    listener.StartAccept(NetAddr(L"127.0.0.1", 8000));
    
    for (int32 i = 0; i < 5; i++)
    {
        core.GetThreadManager().Launch([&core]()
            {
                while (true)
                {
                    core.GetIOCPCore().Dispatch(INFINITE);
                }
            }
        );
    }

    core.GetThreadManager().Join();
    return 0;
}