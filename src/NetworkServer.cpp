#include "NetworkServer.h"
#include <cmath>
#include <stdexcept>

NetworkServer::NetworkServer(NetworkVRDriver &driver_ref) : driver(driver_ref) {

}

NetworkServer::~NetworkServer() {

}

void NetworkServer::start() {
    this->runServer = true;
    this->recv_thread = std::thread(&NetworkServer::socket_thread, this);
}

void NetworkServer::socket_thread() {
    sd = INVALID_SOCKET;
    struct sockaddr_in server, client;

    sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sd == INVALID_SOCKET) {
        vr::VRDriverLog()->Log("Could not create socket");
        return;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8082);

    if (bind(sd, (struct sockaddr *) &server, sizeof(server)) == SOCKET_ERROR) {
        int error = WSAGetLastError();

        vr::VRDriverLog()->Log("bind failed with err");

        return;
    }


    int len, packetLength;
    len = sizeof(client);

    while (this->runServer) {
        packetLength = recvfrom(sd, (char *) socket_buffer, RECV_BUFFER_SIZE, 0, (struct sockaddr *) &client, &len);
        this->handleClientMessage(packetLength, &client);
    }
}

int NetworkServer::initialize_winsock() {
    WSADATA wsaData;
    return WSAStartup(MAKEWORD(2, 2), &wsaData);
}

void NetworkServer::handleClientMessage(int packetLength, struct sockaddr_in *client_addr) {
    ClientMessage message;
    NetworkClient *client = findClientByAddress(client_addr);

    driver.LogMessage("New client message!");
    if (message.ParseFromArray(this->socket_buffer, packetLength)) {
        // OVNRDMessage was successfully decoded!
        for (int i = 0; i < message.requests_size(); i++) {
            const ClientMessage_Request &request = message.requests(i);
            switch (request.request_case()) {
                case ClientMessage_Request::kHello:
                    client = handleHello(request.hello(), client_addr);
                    break;
                case ClientMessage_Request::kUpdateDevice:
                    handleUpdateDevice(request.update_device());
                    break;
                case ClientMessage_Request::kRegisterDevices:
                    handleRegisterDevice(request.register_devices(), client);
                    break;
                case ClientMessage_Request::kAddComponents:
                    handleAddComponent(request.add_components());
                    break;
                case ClientMessage_Request::REQUEST_NOT_SET:
                    break;

            }
        }
    } else {
        NetworkVRDriver::LogMessage("Failed to decode OVRND message!");
    }
}

NetworkClient *NetworkServer::handleHello(const HelloRequest &request, struct sockaddr_in *client_addr) {
    auto client = findClientBySerial(request.serial());
    if (client == nullptr) {
        client = new NetworkClient(driver, request.serial(), client_addr);
        clients.push_back(client);
    }

    return client;
}

void NetworkServer::handleRegisterDevice(const RegisterDevicesRequest &request, NetworkClient *client) {
    driver.LogMessage("Register!");
    for (int i = 0; i < request.devices_size(); i++) {
        const auto &device_data = request.devices(i);
        auto device = driver.RegisterDevice(static_cast<vr::ETrackedDeviceClass>(device_data.device_class()),
                                            device_data.serial());
        if (client != nullptr) {
            device->AddClient(*client);
        }
    }
}

void NetworkServer::handleUpdateDevice(const UpdateDeviceRequest &request) {
    NetworkDevice *device = driver.FindDeviceByIndex(request.tracker_id());
    if (device == nullptr) return;

    // Update properties
    for (int i = 0; i < request.property_updates_size(); i++) {
        auto propertyUpdate = request.property_updates(i);
        auto property = static_cast<vr::ETrackedDeviceProperty>(propertyUpdate.property());
        switch (propertyUpdate.value_case()) {

            case UpdateDeviceRequest_PropertyUpdate::kStrValue:
                device->setStringProperty(property, propertyUpdate.str_value());
                break;
            case UpdateDeviceRequest_PropertyUpdate::kFloatValue:
                device->setFloatProperty(property, propertyUpdate.float_value());
                break;
            case UpdateDeviceRequest_PropertyUpdate::kBoolValue:
                device->setBoolProperty(property, propertyUpdate.bool_value());
                break;
            case UpdateDeviceRequest_PropertyUpdate::VALUE_NOT_SET:
                break;
        }
    }
}

void NetworkServer::stop() {
    if (this->recv_thread.joinable()) {
        this->runServer = false;
        this->recv_thread.join();
    }
}

void NetworkServer::sendClientMessage(const ServerMessage &message, NetworkClient &client) const {
    size_t size = message.ByteSizeLong();
    char *buffer = new char[size];
    message.SerializeToArray(buffer, size);
    sendto(sd, buffer, size, 0, (struct sockaddr *) client.getAddressPtr(), sizeof(sockaddr_in));

    delete[] buffer;
}

void NetworkServer::updateClients() {
    for (const auto &client: clients) {
        client->flushUpdates();
    }
}

NetworkClient *NetworkServer::findClientBySerial(const std::string &serial) {
    for (const auto &client: clients) {
        if (client->getSerial() == serial) {
            return client;
        }
    }

    return nullptr;
}

NetworkClient *NetworkServer::findClientByAddress(const struct sockaddr_in *address) {
    for (const auto &client: clients) {
        if (sockaddr_in_equal(client->getAddressPtr(), address)) {
            return client;
        }
    }

    return nullptr;
}

void NetworkServer::handleAddComponent(const AddComponentsRequest &request) {
    NetworkDevice *device = driver.FindDeviceByIndex(request.device_id());
    if (device == nullptr) return;

    auto container = device->getPropertyContainerHandle();
    for (int i = 0; i < request.components_size(); i++) {
        const auto &component = request.components(i);
        switch (component.config_case()) {

            case AddComponentsRequest_Component::kBoolean:
                device->addBooleanComponent(component.name(), component.component());
                break;
            case AddComponentsRequest_Component::kScalar:
                device->addScalarComponent(component.name(), component.component(),
                                           static_cast<vr::EVRScalarType>(component.scalar().type()),
                                           static_cast<vr::EVRScalarUnits>(component.scalar().units()));
                break;
            case AddComponentsRequest_Component::kHaptic:
                device->addHapticComponent(component.name(), component.component());
                break;
            case AddComponentsRequest_Component::kSkeleton:
                break;
            case AddComponentsRequest_Component::CONFIG_NOT_SET:
                break;
        }
    }
}

