#include "NetworkTracker.h"
#include <math.h>
#include <chrono>
#include "linalg.h"

Tracker::Tracker(const char* serial) {
	this->serial = (char*) serial;
}

Tracker::~Tracker() {

}

vr::DriverPose_t Tracker::getDefaultPose()
{
	vr::DriverPose_t pose = { 0 };
	vr::HmdQuaternion_t quat;
	quat.w = 1;
	quat.x = 0;
	quat.y = 0;
	quat.z = 0;

	pose.deviceIsConnected = true;
	pose.poseIsValid = true;
	pose.result = vr::ETrackingResult::TrackingResult_Running_OK;
	pose.qWorldFromDriverRotation = quat;
	pose.qDriverFromHeadRotation = quat;
	pose.qRotation = quat;
	return pose;
}

void Tracker::updateTrackerWith(bool visible, double* rvec, double* tvec)
{
	vr::DriverPose_t pose = getDefaultPose();
	pose.poseIsValid = visible;
	pose.result = visible ? vr::ETrackingResult::TrackingResult_Running_OK : vr::ETrackingResult::TrackingResult_Running_OutOfRange;
	if (visible) {
		// Position
		memcpy(pose.vecPosition, tvec, sizeof(double[3]));
		
		// Rotation
		linalg::vec<double, 3> rot_vec = { rvec[0], rvec[1], rvec[2] };
		double theta = linalg::length(rot_vec);
		linalg::vec<double, 3> axis = linalg::normalize(rot_vec);
		pose.qRotation.w = cos(theta / 2);
		if (!isnan(axis.x)) // if rvec is 0 0 0 don't override qrotation
		{
			pose.qRotation.x = axis.x;
			pose.qRotation.y = axis.y;
			pose.qRotation.z = axis.z;
		}
		

	}
	vr::VRServerDriverHost()->TrackedDevicePoseUpdated(this->deviceId, pose, sizeof(vr::DriverPose_t));
	
}

vr::EVRInitError Tracker::Activate(uint32_t unObjectId)
{
	this->deviceId = unObjectId;
	vr::PropertyContainerHandle_t propertyContainer = vr::VRProperties()->TrackedDeviceToPropertyContainer(unObjectId);
	return vr::VRInitError_None;
}

void Tracker::Deactivate()
{
}

void Tracker::EnterStandby()
{
	
}

void* Tracker::GetComponent(const char* pchComponentNameAndVersion)
{
	return nullptr;
}

void Tracker::DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize)
{
}

vr::DriverPose_t Tracker::GetPose()
{
	vr::DriverPose_t pose = getDefaultPose();
	pose.poseIsValid = false;
	pose.result = vr::ETrackingResult::TrackingResult_Uninitialized;
	return pose;
}

void Tracker::RunFrame() {
	vr::VRServerDriverHost()->TrackedDevicePoseUpdated(this->deviceId, this->GetPose(), sizeof(vr::DriverPose_t));
}