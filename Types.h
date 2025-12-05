#pragma once
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <functional>
#include <winsock2.h>


namespace Proto {
    const std::string GET = "GET";
    const std::string SEND = "SEND";
    const std::string EXEC = "EXEC";
    
    // Protocol handler registry
    extern std::unordered_set<std::string> allowed_protocols;
    extern std::unordered_map<std::string, std::function<void(SOCKET, std::vector<std::string>)>> handlers;
    
    void init_handlers();
    bool is_valid_protocol(const std::string& proto);
}
