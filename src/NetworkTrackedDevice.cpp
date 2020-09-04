#include "NetworkTrackedDevice.h"
#include <math.h>
#include "linalg.h"

NetworkTrackedDevice::NetworkTrackedDevice(const char* serial) {
	this->serial = (char*) serial;
}

NetworkTrackedDevice::~NetworkTrackedDevice() {

}

vr::DriverPose_t NetworkTrackedDevice::getDefaultPose()
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

void NetworkTrackedDevice::updateTrackerWith(bool visible, double (&rvec)[3], double (&tvec)[3])
{
	vr::DriverPose_t pose = getDefaultPose();
	pose.result = visible ? vr::ETrackingResult::TrackingResult_Running_OK : vr::ETrackingResult::TrackingResult_Running_OutOfRange;
	pose.poseIsValid = visible;

	pose.qWorldFromDriverRotation = GetDriver()->globalQuaternion;
	memcpy(pose.vecWorldFromDriverTranslation, GetDriver()->globalTranslation, sizeof(double[3]));
	if (visible) {
		memcpy(pose.vecPosition, tvec, sizeof(double[3]));
		memcpy(&pose.qRotation, &NetworkVRDriver::rvecToQuat(rvec), sizeof(vr::DriverPoseQuaternion_t));
	}

	vr::VRServerDriverHost()->TrackedDevicePoseUpdated(this->deviceId, pose, sizeof(vr::DriverPose_t));
	
}

vr::EVRInitError NetworkTrackedDevice::Activate(uint32_t unObjectId)
{
	this->deviceId = unObjectId;
	vr::PropertyContainerHandle_t propertyContainer = vr::VRProperties()->TrackedDeviceToPropertyContainer(unObjectId);
	return vr::VRInitError_None;
}

void NetworkTrackedDevice::Deactivate()
{
}

void NetworkTrackedDevice::EnterStandby()
{
	
}

void* NetworkTrackedDevice::GetComponent(const char* pchComponentNameAndVersion)
{
	return nullptr;
}

void NetworkTrackedDevice::DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize)
{
}

vr::DriverPose_t NetworkTrackedDevice::GetPose()
{
	vr::DriverPose_t pose = getDefaultPose();
	pose.poseIsValid = false;
	pose.result = vr::ETrackingResult::TrackingResult_Uninitialized;
	return pose;
}

void NetworkTrackedDevice::RunFrame() {
	vr::VRServerDriverHost()->TrackedDevicePoseUpdated(this->deviceId, this->GetPose(), sizeof(vr::DriverPose_t));
}
