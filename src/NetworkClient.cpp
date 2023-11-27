//
// Created by djfig on 11/26/2023.
//
#include "NetworkClient.h"

NetworkClient::NetworkClient(NetworkVRDriver &driver_ref, const std::string &serial, struct sockaddr_in *address): driver(driver_ref) {
    this->serial = serial;
    update_mutex = new std::mutex();
    setAddress(address);
    reference = driver.RegisterDevice(vr::TrackedDeviceClass_TrackingReference, serial);
}

void NetworkClient::sendMessage(const ServerMessage &msg) {
    driver.getNetworkServer()->sendClientMessage(msg, *this);
}

const struct sockaddr_in* NetworkClient::getAddressPtr() {
    return &address;
}

void NetworkClient::setAddress(sockaddr_in *addr) {
    std::memcpy(&address, addr, sizeof(sockaddr_in));
}

const std::string &NetworkClient::getSerial() {
    return serial;
}

void NetworkClient::flushUpdates() {
    update_mutex->lock();

    if (activationResponse != nullptr)  {
        auto response = message.add_responses();
        response->set_allocated_device_activation(activationResponse);
    }

    if (eventsResponse != nullptr) {
        auto response = message.add_responses();
        response->set_allocated_events(eventsResponse);
    }

    if (message.responses_size() > 0)
    {
        sendMessage(message);
        message.Clear();
        activationResponse = nullptr;
        eventsResponse = nullptr;
    }

    update_mutex->unlock();
}

void NetworkClient::notifyDeviceActivation(NetworkDevice &device) {
    update_mutex->lock();

    if (activationResponse == nullptr) {
        activationResponse = new DeviceActivationResponse();
    }

    auto device_data = activationResponse->add_devices();
    device_data->set_device_class(static_cast<Device_DeviceClass>(device.GetDeviceClass()));
    device_data->set_device_id(device.GetDeviceId());
    device_data->set_serial(device.GetSerial());

    update_mutex->unlock();
}

void NetworkClient::NotifyEvent(vr::VREvent_t &vrEvent) {
    update_mutex->lock();

    if (eventsResponse == nullptr) {
        eventsResponse = new EventsResponse();
    }

    auto event = eventsResponse->add_events();
    event->set_device_id(vrEvent.trackedDeviceIndex);
    event->set_age_seconds(vrEvent.eventAgeSeconds);
    event->set_event_type(vrEvent.eventType);

    if (vrEvent.eventType == vr::EVREventType::VREvent_Input_HapticVibration)
    {
        auto hapticData = new EventsResponse_Event_HapticVibrationEventData();
        auto hapticEventData = vrEvent.data.hapticVibration;
        hapticData->set_frequency(hapticEventData.fFrequency);
        hapticData->set_amplitude(hapticEventData.fAmplitude);
        hapticData->set_duration(hapticEventData.fDurationSeconds);
        hapticData->set_component_handle(hapticEventData.componentHandle);
        hapticData->set_container_handle(hapticEventData.containerHandle);
    }

    update_mutex->unlock();
}
