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

vr::DriverPoseQuaternion_t NetworkVRDriver::rvecToQuat(double(&rvec)[3]) {
	vr::DriverPoseQuaternion_t quat = { 0 };
	// Rotation
	linalg::vec<double, 3> rot_vec = { rvec[0], rvec[1], rvec[2] };
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
	auto pose = NetworkTrackedDevice::getDefaultPose();
	pose.qRotation = this->globalQuaternion;
	memcpy(&pose.vecPosition, &this->globalTranslation, sizeof(double[3]));
	if (this->reference != nullptr) {
		vr::VRServerDriverHost()->TrackedDevicePoseUpdated(reference->deviceId, pose, sizeof(vr::DriverPose_t));
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
		this->reference = new NetworkTrackedDevice("Network Reference");
		this->reference->isReferenceDevice = true;
		vr::VRServerDriverHost()->TrackedDeviceAdded(reference->serial, vr::ETrackedDeviceClass::TrackedDeviceClass_TrackingReference, reference);
	}
}

void NetworkVRDriver::AddTracker(char id, const char* tracker_name)
{
	if (this->trackers_map[id] != nullptr) {
		// don't recreate tracker device
		return;
	}

	this->trackers_map[id] = new NetworkTrackedDevice(tracker_name);
	vr::VRServerDriverHost()->TrackedDeviceAdded(tracker_name, vr::ETrackedDeviceClass::TrackedDeviceClass_GenericTracker, this->trackers_map[id]);
}