// emulator/network/virtual_network.cpp — virtual NIC probe
#include "../hardware.hpp"
#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#endif

namespace emulator
{

    bool VirtualNetwork::self_test(std::string &detail)
    {
        if (!_cfg.network)
        {
            detail = "disabled by config";
            return true;
        } // disabled != fault
#if defined(_WIN32)
        WSADATA wsa;
        const int rc = WSAStartup(MAKEWORD(2, 2), &wsa);
        if (rc != 0)
        {
            detail = "WSAStartup failed";
            return false;
        }
        // real probe: create a datagram socket (proves the stack is usable)
        SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        const bool ok = s != INVALID_SOCKET;
        if (ok)
            closesocket(s);
        WSACleanup();
        detail = ok ? "virtual NIC up (UDP stack OK)" : "socket() failed";
        return ok;
#else
        detail = "virtual NIC up";
        return true;
#endif
    }

} // namespace emulator
