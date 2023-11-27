//
// Created by djfig on 11/25/2023.
//
#include "network.h"

std::string sockaddr_to_string(sockaddr_in *sockaddr) {
    char ip_addr_str[16];
    int port = ntohs(sockaddr->sin_port);
    inet_ntop(AF_INET, &sockaddr->sin_addr, ip_addr_str, sizeof(ip_addr_str));
    char addr_str[25];
    snprintf(addr_str, sizeof(addr_str), "%s:%d", ip_addr_str, port);
    std::string address_string = std::string(addr_str);
    return address_string;
}

bool sockaddr_in_equal(const sockaddr_in *a, const sockaddr_in *b) {
    if (a == nullptr || b == nullptr) return false;
    return (a->sin_family == b->sin_family) &&
           (a->sin_port == b->sin_port) &&
           (std::memcmp(&a->sin_addr, &b->sin_addr, sizeof(a->sin_addr)) == 0);
}