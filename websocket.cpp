#include <iostream>
#include <string>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <thread>

#pragma comment(lib, "Ws2_32.lib")
#define PORT "5173"
#define MAX_THREADS 100

WSADATA wsaData;
struct addrinfo *result = NULL, *ptr = NULL, hints;
SOCKET ListenSocket = INVALID_SOCKET, ClientSocket = INVALID_SOCKET;
std::thread thread_pool[MAX_THREADS];
typedef struct file_list_type
{
    std::string file_name;
    size_t file_size;
    std::string file_remote_ip;
    unsigned int file_port;
} FILEITEM;

int Init()
{
    int iResult;
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        std::cerr << "[INIT] Library Init failed" << "\n";
        return 1;
    }
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;
    iResult = getaddrinfo(NULL, PORT, &hints, &result);
    if (iResult != 0)
    {
        std::cerr << "[INIT] getaddrinfo failed: " << iResult << "\n";
        WSACleanup();
        return 1;
    }
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET)
    {
        std::cerr << "[INIT] Error at socket: " << WSAGetLastError() << "\n";
        WSACleanup();
        return 1;
    }
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult != 0)
    {
        std::cerr << "[INIT] Bind to the socket failed " << WSAGetLastError() << "\n";
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    freeaddrinfo(result);
    return 0;
}

void handle_client(SOCKET client)
{
    char buf[4096];
    std::cout << "[HANDLE_CLIENT] Connection established\n";
    
    while (true)
    {
        int n = recv(client, buf, sizeof(buf), 0);
        if (n > 0)
        {
            std::cout << "[HANDLE_CLIENT] Received " << n << " bytes\n";
            // Echo back
            send(client, buf, n, 0);
        }
        else if (n == 0)
        {
            std::cout << "[HANDLE_CLIENT] Client disconnected gracefully\n";
            break;
        }
        else
        {
            std::cerr << "[HANDLE_CLIENT] recv error: " << WSAGetLastError() << "\n";
            break;
        }
    }

    shutdown(client, SD_SEND);
    closesocket(client);
}

void accept_loop()
{
    while (true)
    {
        SOCKET client = accept(ListenSocket, nullptr, nullptr);
        if (client == INVALID_SOCKET)
        {
            std::cerr << "[ACCEPT] failed: " << WSAGetLastError() << "\n";
            break; // or continue, depending on your policy
        }

        // For now, handle synchronously in this thread:
        std::cout << "[ACCEPT] Success\n";
        handle_client(client);
    }

    closesocket(ListenSocket);
    WSACleanup();
}

int start_listening()
{
    if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        std::cerr << "[LISTEN] Listening Failed" << WSAGetLastError() << "\n";
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    return 0;
}

void cleanup()
{
    if (wsaData.szSystemStatus)
    {
        return;
    }
}

int main(int argc, char *argv[])
{
    int iResult = Init();
    if (iResult != 0)
    {
        std::cerr << "[MAIN] Init crashed\n";
        return 1;
    }
    iResult = start_listening();
    if (iResult != 0)
    {
        std::cerr << "[MAIN] Start Listening gave error\n";
    }
    accept_loop();

    return 0;
}