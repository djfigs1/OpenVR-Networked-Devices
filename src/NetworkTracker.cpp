#include "NetworkTracker.h"
#include <math.h>
#include "linalg.h"

Tracker::Tracker(const char* serial, TrackerProvider* provider) {
	this->serial = (char*) serial;
	this->provider = provider;
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

void Tracker::updateTrackerWith(bool visible, double (&rvec)[3], double (&tvec)[3])
{
	vr::DriverPose_t pose = getDefaultPose();
	pose.result = visible ? vr::ETrackingResult::TrackingResult_Running_OK : vr::ETrackingResult::TrackingResult_Running_OutOfRange;
	pose.poseIsValid = visible;

	pose.qWorldFromDriverRotation = this->provider->globalQuaternion;
	memcpy(pose.vecWorldFromDriverTranslation, this->provider->globalTranslation, sizeof(double[3]));
	if (visible) {
		memcpy(pose.vecPosition, tvec, sizeof(double[3]));
		memcpy(&pose.qRotation, &TrackerProvider::rvecToQuat(rvec), sizeof(vr::DriverPoseQuaternion_t));
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
