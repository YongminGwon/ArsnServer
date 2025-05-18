#include "pch.h"
#include "Listener.h"

int main()
{
    shared_ptr<Listener> listener = make_shared<Listener>();
    listener->StartAccept(NetAddr(L"127.0.0.1", 8000));
    
    for (int32 i = 0; i < 5; i++)
    {
         GCore.GetThreadManager().Launch([]()
            {
                while (true)
                {
                    GCore.GetIOCPCore().Dispatch(INFINITE);
                }
            }
        );
    }

    GCore.GetThreadManager().Join();
    return 0;
}