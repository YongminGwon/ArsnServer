#pragma once

#define WIN32_LEAN_AND_MEAN

using namespace std;

/*----------------------
    ServerCoreIncludes
-----------------------*/
#include "Types.h"
#include "GlobalCore.h"
#include "CoreTLS.h"
#include "CoreMacro.h"
#include "ThreadManager.h"

/*----------------------
         STL
-----------------------*/
#include <iostream>
#include <thread>
#include <chrono>
#include <functional>
#include <mutex>
#include <memory>

/*----------------------
       Container
-----------------------*/
#include <list>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <map>
#include <queue>
#include <stack>

/*----------------------
        NetWork
-----------------------*/
#include <winsock2.h>
#include <MSWSock.h>
#include <WS2tcpip.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")

/*----------------------
          Log
-----------------------*/
#include <plog/Log.h>
#include <plog/Init.h>
#include <plog/Formatters/TxtFormatter.h>
#include <plog/Appenders/ConsoleAppender.h>
#include <plog/Appenders/RollingFileAppender.h>