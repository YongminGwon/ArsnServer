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

/*----------------------
         STL
-----------------------*/
#include <iostream>
#include <thread>
#include <chrono>

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