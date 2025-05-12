#pragma once

#include <mutex>
#include <thread>
#include <vector>
#include <functional>

class ThreadManager
{
public:
	ThreadManager();
	~ThreadManager();

	void Launch(function<void()> callback);
	void Join();

	static void InitTLS();
	static void DestroyTLS();
private:
	mutex                   lock_;
	vector<thread>          threads_;
};