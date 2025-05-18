#pragma once

#include "ThreadManager.h"
#include "IOCPCore.h"
#include <plog/Log.h>
#include <plog/Init.h>
#include <plog/Formatters/TxtFormatter.h>
#include <plog/Appenders/ConsoleAppender.h>
#include <plog/Appenders/RollingFileAppender.h>
#include <filesystem>

class GlobalCore
{
public:
	static GlobalCore& Instance()
	{
		static GlobalCore inst;
		return inst;
	}

	ThreadManager& GetThreadManager() { return threadManager_; }
	IOCPCore& GetIOCPCore() { return iocpCore_; }

private:
	GlobalCore():consoleAppender_(), fileAppender_("logs/GameServer.log", 5*1024*1024, 3), threadManager_(), iocpCore_()
	{
		std::filesystem::create_directories("logs");
		plog::init(plog::debug, &consoleAppender_).addAppender(&fileAppender_);

		PLOG_INFO << "=== GlobalCore Initialized ===";
	}

	~GlobalCore()
	{
	}

	GlobalCore(const GlobalCore&) = delete;
	GlobalCore& operator=(const GlobalCore&) = delete;

	ThreadManager threadManager_;
	IOCPCore iocpCore_;

	plog::ConsoleAppender<plog::TxtFormatter> consoleAppender_;
	plog::RollingFileAppender<plog::TxtFormatter> fileAppender_;
};