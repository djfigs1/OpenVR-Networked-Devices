#pragma once
#include <openvr_driver.h>
#include <memory>
#include <string>
#include "NetworkDevice.h"
#include "NetworkServer.h"

class NetworkVRDriver : public vr::IServerTrackedDeviceProvider {
public:
    NetworkVRDriver();

    ~NetworkVRDriver();

    double globalTranslation[3];
    vr::HmdQuaternion_t globalQuaternion;

    NetworkDevice *FindDeviceByIndex(vr::TrackedDeviceIndex_t index);
    NetworkDevice *FindDeviceBySerial(const std::string &serial);

    NetworkDevice *
    RegisterDevice(vr::ETrackedDeviceClass deviceClass, const std::string &serial);

    static void LogMessage(const std::string &message);

    const class NetworkServer *getNetworkServer();

    // Implementation of ITrackedDeviceServerDriver
    vr::EVRInitError Init(vr::IVRDriverContext *pDriverContext);

    void Cleanup();

    const char *const *GetInterfaceVersions();

    void RunFrame();

    bool ShouldBlockStandbyMode();

    void EnterStandby();

    void LeaveStandby();



private:
    NetworkServer *p_socketServer;
    std::vector<NetworkDevice*> devices;
};

