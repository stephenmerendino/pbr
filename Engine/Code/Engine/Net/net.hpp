#pragma once

#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <WS2tcpip.h>

//winsock2 lib
#pragma comment(lib, "ws2_32.lib")

bool net_system_init();
void net_system_shutdown();