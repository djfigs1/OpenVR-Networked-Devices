//
// Created by djfig on 11/26/2023.
//

#include "NetworkDeviceComponent.h"

NetworkDeviceComponent::NetworkDeviceComponent(const std::string &name, const std::string &component,
                                               NetworkDeviceComponentType type, vr::VRInputComponentHandle_t handle) {
    this->name = name;
    this->component = component;
    this->type = type;
    this->handle = handle;
}

const std::string &NetworkDeviceComponent::getName() const {
    return name;
}

const std::string &NetworkDeviceComponent::getComponent() const {
    return component;
}

NetworkDeviceComponentType NetworkDeviceComponent::getType() const {
    return type;
}

vr::VRInputComponentHandle_t NetworkDeviceComponent::getHandle() const {
    return handle;
}