#include "pch.h"

void ThreadMain()
{
	while (true)
	{
		cout << "Hello I'm Thread... " << LThreadId << endl;
		this_thread::sleep_for(1s);
	}
}
int main()
{
	auto& core = GlobalCore::Instance();
	for (int32 i = 0; i < 5; i++)
	{
		core.GetThreadManager().Launch(ThreadMain);
	}

	core.GetThreadManager().Join();

	return 0;
}