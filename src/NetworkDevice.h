#pragma once

#include <string>
#include <utility>
#include <functional>
#include <vector>
#include <mutex>
#include "openvr_driver.h"
#include "linalg.h"
#include "NetworkDeviceComponent.h"

class NetworkVRDriver;

class NetworkClient;

class NetworkDeviceComponent;

enum NetworkDeviceComponentType;

class NetworkDevice : public vr::ITrackedDeviceServerDriver {

public:
    explicit NetworkDevice(NetworkVRDriver &driver_ref, vr::ETrackedDeviceClass deviceClass, std::string serial_ref);

    std::string GetSerial();

    vr::TrackedDeviceIndex_t GetDeviceId() const;

    vr::EVRInitError Activate(uint32_t unObjectId) override;

    virtual vr::ETrackedDeviceClass GetDeviceClass();

    void Deactivate() override;

    void EnterStandby() override;

    vr::PropertyContainerHandle_t getPropertyContainerHandle() const;

    void *GetComponent(const char *pchComponentNameAndVersion) override;

    void DebugRequest(const char *pchRequest, char *pchResponseBuffer, uint32_t unResponseBufferSize) override;

    void setStringProperty(vr::ETrackedDeviceProperty prop, const std::string &value);

    void setFloatProperty(vr::ETrackedDeviceProperty prop, float fNewValue);

    void setBoolProperty(vr::ETrackedDeviceProperty prop, bool bNewValue);

    NetworkDeviceComponent *
    findComponentByNameAndComponent(const std::string &name, const std::string &component);

    NetworkDeviceComponent *
    findComponentByIndexAndType(vr::VRInputComponentHandle_t handle, NetworkDeviceComponentType componentType);

    void addBooleanComponent(const std::string &name, const std::string &component);

    void addScalarComponent(const std::string &name, const std::string &component, vr::EVRScalarType type, vr::EVRScalarUnits units);

    void addHapticComponent(const std::string &name, const std::string &component);

    static std::string formatNameAndComponent(const std::string &name, const std::string &component);

    // ------------------------------------
    // Tracking Methods
    // ------------------------------------
    virtual vr::DriverPose_t GetPose();

    virtual void RunFrame();

    void AddClient(NetworkClient &client);

    void WithEachClient(std::function<void(NetworkClient *)> func);

    ~NetworkDevice() = default;

    static vr::DriverPose_t getDefaultPose() {
        vr::DriverPose_t pose = {0};
        vr::HmdQuaternion_t rotation = {1, 0, 0, 0};

        pose.deviceIsConnected = true;
        pose.poseIsValid = true;
        pose.result = vr::ETrackingResult::TrackingResult_Running_OK;
        pose.qWorldFromDriverRotation = rotation;
        pose.qDriverFromHeadRotation = rotation;
        pose.qRotation = rotation;
        return pose;
    }

    static vr::DriverPoseQuaternion_t rvecToQuat(double(&rvec)[3]) {
        vr::DriverPoseQuaternion_t quat = {0};
        // Rotation
        linalg::vec<double, 3> rot_vec = {rvec[0], rvec[1], rvec[2]};
        double theta = linalg::length(rot_vec);
        linalg::vec<double, 3> axis = linalg::normalize(rot_vec);
        quat.w = cos(theta / 2);
        if (!isnan(axis.x)) // if rvec is 0 0 0 don't override qrotation
        {
            quat.x = axis.x * sin(theta / 2);
            quat.y = axis.y * sin(theta / 2);
            quat.z = axis.z * sin(theta / 2);
        }
        return quat;
    }

protected:
    NetworkVRDriver &driver;
    std::string serial;
    vr::TrackedDeviceIndex_t device_id;
    std::vector<NetworkClient *> clients;
    std::vector<NetworkDeviceComponent *> components;
    vr::ETrackedDeviceClass deviceClass;
    std::mutex *clients_mutex;
};
