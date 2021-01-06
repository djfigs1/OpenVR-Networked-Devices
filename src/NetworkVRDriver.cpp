#include "NetworkVRDriver.h"
#include "linalg.h"
#include <thread>
#include <string>
#include <memory>
#include <iostream>


NetworkVRDriver::NetworkVRDriver()
{
	this->globalQuaternion.w = 1.0;
	this->globalQuaternion.x = this->globalQuaternion.y = this->globalQuaternion.z = 0;
	ZeroMemory(this->globalTranslation, sizeof(double[3]));
}

NetworkVRDriver::~NetworkVRDriver()
{

}

vr::EVRInitError NetworkVRDriver::Init(vr::IVRDriverContext* pDriverContext) {
	VR_INIT_SERVER_DRIVER_CONTEXT(pDriverContext);
	LOG("NetworkVR has been initialized!");


	this->p_socketServer = new SocketServer(this);
	int winsock_result = SocketServer::initialize_winsock();
	if (winsock_result == 0) {
		LOG("WinSock has been initialized successfully!");
		this->p_socketServer->start();
	}

	//hmd_thread = std::thread(checkHMDLocation);

	return vr::VRInitError_None;
}

void NetworkVRDriver::Cleanup() {
	LOG("NetworkVR cleanup!");
	delete this->p_socketServer;
}

const char* const* NetworkVRDriver::GetInterfaceVersions()
{
	return vr::k_InterfaceVersions;
}

void NetworkVRDriver::RunFrame()
{
	auto pose = INetworkTrackedDevice::getDefaultPose();
	pose.qRotation = this->globalQuaternion;
	memcpy(&pose.vecPosition, &this->globalTranslation, sizeof(double[3]));
	if (this->reference != nullptr) {
		vr::VRServerDriverHost()->TrackedDevicePoseUpdated(reference->GetDeviceId(), pose, sizeof(vr::DriverPose_t));
	}
}

bool NetworkVRDriver::ShouldBlockStandbyMode()
{
	return false;
}

void NetworkVRDriver::EnterStandby()
{

}

void NetworkVRDriver::LeaveStandby()
{

}

void NetworkVRDriver::clientDidHandshake()
{
	
}

void NetworkVRDriver::AddReference()
{
	if (this->reference == nullptr) {
		this->reference = new NetworkReferenceDevice("Network Reference");
		this->reference->SetDeviceGlobalQuaternion(this->globalQuaternion);
		this->reference->SetDeviceGlobalTranslation(this->globalTranslation);
		vr::VRServerDriverHost()->TrackedDeviceAdded(reference->GetSerial().c_str(), vr::ETrackedDeviceClass::TrackedDeviceClass_TrackingReference, reference);
	}
}

void NetworkVRDriver::AddTracker(char id, const char* tracker_name)
{
	if (this->trackers_map[id] != nullptr) {
		// don't recreate tracker device
		return;
	}

	auto tracker = new NetworkGenericDevice(tracker_name);
	this->trackers_map[id] = tracker;
	tracker->SetDeviceGlobalQuaternion(this->globalQuaternion);
	tracker->SetDeviceGlobalTranslation(this->globalTranslation);

	vr::VRServerDriverHost()->TrackedDeviceAdded(tracker_name, vr::ETrackedDeviceClass::TrackedDeviceClass_GenericTracker, tracker);
}