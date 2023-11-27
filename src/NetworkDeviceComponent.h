#pragma once
//
// Created by djfig on 11/26/2023.
//
#include <string>
#include "openvr_driver.h"
#include "NetworkDevice.h"

#ifndef OPENVRNETWORKEDDEVICES_NETWORKDEVICECOMPONENT_H
#define OPENVRNETWORKEDDEVICES_NETWORKDEVICECOMPONENT_H


enum NetworkDeviceComponentType {
    Boolean,
    Scalar,
    Haptic,
    Skeleton
};

class NetworkDeviceComponent {
public:
    NetworkDeviceComponent(const std::string &name, const std::string &component, NetworkDeviceComponentType type, vr::VRInputComponentHandle_t handle);

    std::string name;
    std::string component;
    NetworkDeviceComponentType type;
    vr::VRInputComponentHandle_t handle;

    const std::string &getName() const;

    const std::string &getComponent() const;

    NetworkDeviceComponentType getType() const;

    vr::VRInputComponentHandle_t getHandle() const;
};

#endif //OPENVRNETWORKEDDEVICES_NETWORKDEVICECOMPONENT_H
