#include "NetworkVRDriver.h"
#include <string>
#include <memory>

NetworkVRDriver::NetworkVRDriver() {
    this->globalQuaternion.w = 1.0;
    this->globalQuaternion.x = this->globalQuaternion.y = this->globalQuaternion.z = 0;
    ZeroMemory(this->globalTranslation, sizeof(double[3]));
}

NetworkVRDriver::~NetworkVRDriver() {
    delete this->p_socketServer;
}

vr::EVRInitError NetworkVRDriver::Init(vr::IVRDriverContext *pDriverContext) {
    VR_INIT_SERVER_DRIVER_CONTEXT(pDriverContext);
    this->p_socketServer = new NetworkServer(*this);
    LogMessage("OpenVR Networked Devices has been initialized!");

    int winsock_result = NetworkServer::initialize_winsock();
    if (winsock_result == 0) {
        LogMessage("WinSock has been initialized successfully!");
        this->p_socketServer->start();
    }

    return vr::VRInitError_None;
}

void NetworkVRDriver::Cleanup() {
    LogMessage("NetworkVR cleanup!");
    delete this->p_socketServer;
}

const char *const *NetworkVRDriver::GetInterfaceVersions() {
    return vr::k_InterfaceVersions;
}

void NetworkVRDriver::RunFrame() {
    p_socketServer->updateClients();

    vr::VREvent_t vrEvent{};
    while (vr::VRServerDriverHost()->PollNextEvent(&vrEvent, sizeof(vrEvent))) {
        auto device = FindDeviceByIndex(vrEvent.trackedDeviceIndex);
        if (device != nullptr) {
            device->WithEachClient([&vrEvent](auto client) {
                client->NotifyEvent(vrEvent);
            });
        }
    }
}

void NetworkVRDriver::LogMessage(const std::string &message) {
    vr::VRDriverLog()->Log(message.c_str());
}

bool NetworkVRDriver::ShouldBlockStandbyMode() {
    return false;
}

void NetworkVRDriver::EnterStandby() {

}

void NetworkVRDriver::LeaveStandby() {

}

const NetworkServer *NetworkVRDriver::getNetworkServer() {
    return p_socketServer;
}

NetworkDevice *NetworkVRDriver::RegisterDevice(vr::ETrackedDeviceClass deviceClass, const std::string &serial) {
    auto device = FindDeviceBySerial(serial);

    if (FindDeviceBySerial(serial) == nullptr) {
        device = new NetworkDevice(*this, deviceClass, serial);
        vr::VRServerDriverHost()->TrackedDeviceAdded(serial.c_str(),
                                                     deviceClass,
                                                     device);
        devices.push_back(device);

        return device;
    }

    return device;
}

NetworkDevice *NetworkVRDriver::FindDeviceByIndex(vr::TrackedDeviceIndex_t index) {
    for (const auto &device: devices) {
        if (device->GetDeviceId() != 0 && device->GetDeviceId() == index) {
            return device;
        }
    }

    return nullptr;
}

NetworkDevice *NetworkVRDriver::FindDeviceBySerial(const std::string &serial) {
    for (const auto &device: devices) {
        if (device->GetSerial() == serial) {
            return device;
        }
    }

    return nullptr;
}
