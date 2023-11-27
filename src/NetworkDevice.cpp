//
// Created by djfig on 11/26/2023.
//
#include "NetworkDevice.h"
#include "NetworkVRDriver.h"

std::string NetworkDevice::GetSerial() {
    return serial;
};

vr::TrackedDeviceIndex_t NetworkDevice::GetDeviceId() const {
    return device_id;
};

vr::EVRInitError NetworkDevice::Activate(uint32_t unObjectId) {
    device_id = unObjectId;

    if (deviceClass == vr::TrackedDeviceClass_GenericTracker) {
        // For some reason I cannot set the icon through vrresources, so here is this instead.
        vr::PropertyContainerHandle_t container = vr::VRProperties()->TrackedDeviceToPropertyContainer(unObjectId);
        std::string ready = "{ovr_networked_devices}/icons/device.png";

        vr::VRProperties()->SetStringProperty(container, vr::Prop_NamedIconPathDeviceReady_String, ready.c_str());
        vr::VRProperties()->SetStringProperty(container, vr::Prop_NamedIconPathDeviceOff_String, ready.c_str());
        vr::VRProperties()->SetStringProperty(container, vr::Prop_NamedIconPathDeviceSearching_String, ready.c_str());
        vr::VRProperties()->SetStringProperty(container, vr::Prop_NamedIconPathDeviceSearchingAlert_String,
                                              ready.c_str());
        vr::VRProperties()->SetStringProperty(container, vr::Prop_NamedIconPathDeviceReadyAlert_String, ready.c_str());
        vr::VRProperties()->SetStringProperty(container, vr::Prop_NamedIconPathDeviceNotReady_String, ready.c_str());
        vr::VRProperties()->SetStringProperty(container, vr::Prop_NamedIconPathDeviceStandby_String, ready.c_str());
        vr::VRProperties()->SetStringProperty(container, vr::Prop_NamedIconPathDeviceAlertLow_String, ready.c_str());
    }

    WithEachClient([this](auto client) {
        client->notifyDeviceActivation(*this);
    });

    return vr::VRInitError_None;
}

void NetworkDevice::setStringProperty(vr::ETrackedDeviceProperty prop, const std::string &value) {
    auto container = getPropertyContainerHandle();
    vr::VRProperties()->SetStringProperty(container, prop, value.c_str());
}

void NetworkDevice::setFloatProperty(vr::ETrackedDeviceProperty prop, float fNewValue) {
    auto container = getPropertyContainerHandle();
    vr::VRProperties()->SetFloatProperty(container, prop, fNewValue);
}

void NetworkDevice::setBoolProperty(vr::ETrackedDeviceProperty prop, bool bNewValue) {
    auto container = getPropertyContainerHandle();
    vr::VRProperties()->SetBoolProperty(container, prop, bNewValue);
}

void NetworkDevice::Deactivate() {

}

void NetworkDevice::EnterStandby() {

}

void *NetworkDevice::GetComponent(const char *pchComponentNameAndVersion) {
    return nullptr;
}

void NetworkDevice::DebugRequest(const char *pchRequest, char *pchResponseBuffer, uint32_t unResponseBufferSize) {

}

void NetworkDevice::RunFrame() {

}

vr::DriverPose_t NetworkDevice::GetPose() {
    return getDefaultPose();
}

NetworkDevice::NetworkDevice(NetworkVRDriver &driver_ref, vr::ETrackedDeviceClass deviceClass, std::string serial_ref)
        : driver(driver_ref),
          serial(std::move(serial_ref)) {
    this->deviceClass = deviceClass;
    device_id = 0;
    clients_mutex = new std::mutex();
}

vr::ETrackedDeviceClass NetworkDevice::GetDeviceClass() {
    return vr::TrackedDeviceClass_Max;
}

void NetworkDevice::AddClient(NetworkClient &client) {
    clients_mutex->lock();
    clients.push_back(&client);

    // If we already have a device id, notify of our activation.
    if (device_id != 0) {
        driver.LogMessage("Existing id found!");
        client.notifyDeviceActivation(*this);
    }

    clients_mutex->unlock();
}

void NetworkDevice::WithEachClient(std::function<void(NetworkClient *)> func) {
    clients_mutex->lock();
    for (auto client: clients) {
        func(client);
    }
    clients_mutex->unlock();
}

vr::PropertyContainerHandle_t NetworkDevice::getPropertyContainerHandle() const {
    vr::PropertyContainerHandle_t container = vr::VRProperties()->TrackedDeviceToPropertyContainer(
            this->GetDeviceId());

    return container;
}

void NetworkDevice::addBooleanComponent(const std::string &name, const std::string &component) {
    if (findComponentByNameAndComponent(name, component) != nullptr) return;

    std::string inputName = formatNameAndComponent(name, component);
    vr::VRInputComponentHandle_t handle;
    vr::VRDriverInput()->CreateBooleanComponent(getPropertyContainerHandle(), inputName.c_str(), &handle);
    auto newComponent = new NetworkDeviceComponent(name, component, Boolean, handle);
    components.push_back(newComponent);
}

void NetworkDevice::addScalarComponent(const std::string &name, const std::string &component, vr::EVRScalarType type,
                                       vr::EVRScalarUnits units) {
    if (findComponentByNameAndComponent(name, component) != nullptr) return;
    std::string inputName = formatNameAndComponent(name, component);
    vr::VRInputComponentHandle_t handle;
    vr::VRDriverInput()->CreateScalarComponent(getPropertyContainerHandle(), inputName.c_str(), &handle, type, units);
    auto newComponent = new NetworkDeviceComponent(name, component, Scalar, handle);
    components.push_back(newComponent);
}

void NetworkDevice::addHapticComponent(const std::string &name, const std::string &component) {
    if (findComponentByNameAndComponent(name, component) != nullptr) return;
    std::string inputName = formatNameAndComponent(name, component);
    vr::VRInputComponentHandle_t handle;
    vr::VRDriverInput()->CreateHapticComponent(getPropertyContainerHandle(), inputName.c_str(), &handle);
    auto newComponent = new NetworkDeviceComponent(name, component, Haptic, handle);
    components.push_back(newComponent);

}

NetworkDeviceComponent *NetworkDevice::findComponentByIndexAndType(vr::VRInputComponentHandle_t handle,
                                                                   NetworkDeviceComponentType componentType) {
    for (const auto component: components) {
        if (component->getHandle() == handle) {
            return component->getType() == componentType ? component : nullptr;
        }
    }

    return nullptr;
}

NetworkDeviceComponent *
NetworkDevice::findComponentByNameAndComponent(const std::string &name, const std::string &component_str) {
    for (const auto component: components) {
        if (component->getName() == name && component->getComponent() == component_str) {
            return component;
        }
    }

    return nullptr;
}

std::string NetworkDevice::formatNameAndComponent(const std::string &name, const std::string &component) {
    return "/input/" + name + "/" + component;
}
