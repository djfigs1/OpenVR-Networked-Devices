#include "VRDriver.h"
#include "linalg.h"
#include <thread>
#include <string>
#include <memory>
#include <iostream>

/*
std::thread hmd_thread;

void checkHMDLocation() {
	while (true) {
		vr::TrackedDevicePose_t hmd;
		vr::VRServerDriverHost()->GetRawTrackedDevicePoses(0, &hmd, 1U);
		if (hmd.bPoseIsValid) {
			float x = hmd.mDeviceToAbsoluteTracking.m[0][3];
			float y = hmd.mDeviceToAbsoluteTracking.m[1][3];
			float z = hmd.mDeviceToAbsoluteTracking.m[2][3];
			char buffer[50];
			snprintf(buffer, 50, "HMD X: %f, Y: %f, Z: %f", x, y, z);
			LOG(buffer);
		}
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
	

}*/

TrackerProvider::TrackerProvider()
{
	this->globalQuaternion.w = 1.0;
	this->globalQuaternion.x = this->globalQuaternion.y = this->globalQuaternion.z = 0;
	ZeroMemory(this->globalTranslation, sizeof(double[3]));
}

TrackerProvider::~TrackerProvider()
{

}

vr::EVRInitError TrackerProvider::Init(vr::IVRDriverContext* pDriverContext) {
	VR_INIT_SERVER_DRIVER_CONTEXT(pDriverContext);
	LOG("arucovr init!");

	this->p_socketServer = new SocketServer(this);
	int winsock_result = SocketServer::initialize_winsock();
	if (winsock_result == 0) {
		LOG("winsock intialized successfully!");
		this->p_socketServer->start();
	}
	
	//hmd_thread = std::thread(checkHMDLocation);

	return vr::VRInitError_None;
}

vr::DriverPoseQuaternion_t TrackerProvider::rvecToQuat(double(&rvec)[3]) {
	vr::DriverPoseQuaternion_t quat;
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

void TrackerProvider::Cleanup() {
	LOG("arucovr cleanup!");
	delete this->p_socketServer;
}

const char* const* TrackerProvider::GetInterfaceVersions()
{
	return vr::k_InterfaceVersions;
}

void TrackerProvider::RunFrame()
{

}

bool TrackerProvider::ShouldBlockStandbyMode()
{
	return false;
}

void TrackerProvider::EnterStandby()
{

}

void TrackerProvider::LeaveStandby()
{

}

void TrackerProvider::ClearTrackers()
{
}

void TrackerProvider::AddTracker(char id, const char* tracker_name)
{
	if (this->trackers_map[id] != nullptr) {
		// don't recreate tracker device
		return;
	}

	this->trackers_map[id] = new Tracker(tracker_name, this);
	vr::VRServerDriverHost()->TrackedDeviceAdded(tracker_name, vr::ETrackedDeviceClass::TrackedDeviceClass_GenericTracker, this->trackers_map[id]);
}

bool TrackerProvider::RemoveTracker(char id)
{
	return false;
}
