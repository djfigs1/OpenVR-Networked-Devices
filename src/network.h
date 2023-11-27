//
// Created by djfig on 11/25/2023.
//

#include <string>
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <WinSock2.h>
#include <WS2tcpip.h>
#include "ovrnd.pb.h"

#ifndef OPENVRNETWORKEDDEVICES_NETWORK_UTIL_H
#define OPENVRNETWORKEDDEVICES_NETWORK_UTIL_H

std::string sockaddr_to_string(sockaddr_in *sockaddr_in);

bool sockaddr_in_equal(const sockaddr_in *a, const sockaddr_in *b);

#endif //OPENVRNETWORKEDDEVICES_NETWORK_UTIL_H