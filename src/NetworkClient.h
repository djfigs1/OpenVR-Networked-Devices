//
// Created by djfig on 11/26/2023.
//
#ifndef OPENVRNETWORKEDDEVICES_NETWORKCLIENT_H
#define OPENVRNETWORKEDDEVICES_NETWORKCLIENT_H

#include "network.h"
#include "NetworkVRDriver.h"
#include "NetworkDevice.h"
#include <mutex>

class NetworkVRDriver;

class NetworkClient {

public:
    explicit NetworkClient(NetworkVRDriver &driver_ref, const std::string &serial, struct sockaddr_in* address);
    const struct sockaddr_in* getAddressPtr();
    const std::string& getSerial();
    void notifyDeviceActivation(NetworkDevice& device);
    void NotifyEvent(vr::VREvent_t &vrEvent);
    void setAddress(sockaddr_in* addr);
    void sendMessage(const ServerMessage &msg);
    void flushUpdates();

private:
    std::string serial;
    struct sockaddr_in address{};
    NetworkVRDriver& driver;
    DeviceActivationResponse *activationResponse = nullptr;
    EventsResponse *eventsResponse = nullptr;
    NetworkDevice *reference;
    ServerMessage message;
    std::mutex *update_mutex;
};


#endif //OPENVRNETWORKEDDEVICES_NETWORKCLIENT_H
