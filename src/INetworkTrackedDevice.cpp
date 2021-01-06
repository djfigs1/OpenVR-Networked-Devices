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

	vr::HmdQuaternion_t qt = GetDriver()->globalQuaternion;
	pose.qWorldFromDriverRotation = qt;
	memcpy(pose.vecWorldFromDriverTranslation, GetDriver()->globalTranslation, sizeof(double[3]));
	if (visible) {
		memcpy(pose.vecPosition, tvec, sizeof(double[3]));
		vr::DriverPoseQuaternion_t q = NetworkVRDriver::rvecToQuat(rvec);
		memcpy(&pose.qRotation, &q, sizeof(vr::DriverPoseQuaternion_t));
	}

	vr::VRServerDriverHost()->TrackedDevicePoseUpdated(this->deviceId, pose, sizeof(vr::DriverPose_t));
	
}

vr::EVRInitError NetworkTrackedDevice::Activate(uint32_t unObjectId)
{
	this->deviceId = unObjectId;
	vr::PropertyContainerHandle_t container = vr::VRProperties()->TrackedDeviceToPropertyContainer(unObjectId);
	std::string ready = this->isReferenceDevice ? "{networkvr}/icons/reference/ready.png" : "{networkvr}/icons/tracked_device/ready.png";
	std::string not_ready = this->isReferenceDevice ? "{networkvr}/icons/reference/ready.png" : "{networkvr}/icons/tracked_device/ready.png";

	vr::VRProperties()->SetStringProperty(container, vr::Prop_NamedIconPathDeviceReady_String, ready.c_str());

	vr::VRProperties()->SetStringProperty(container, vr::Prop_NamedIconPathDeviceOff_String, not_ready.c_str());
	vr::VRProperties()->SetStringProperty(container, vr::Prop_NamedIconPathDeviceSearching_String, not_ready.c_str());
	vr::VRProperties()->SetStringProperty(container, vr::Prop_NamedIconPathDeviceSearchingAlert_String, not_ready.c_str());
	vr::VRProperties()->SetStringProperty(container, vr::Prop_NamedIconPathDeviceReadyAlert_String, not_ready.c_str());
	vr::VRProperties()->SetStringProperty(container, vr::Prop_NamedIconPathDeviceNotReady_String, not_ready.c_str());
	vr::VRProperties()->SetStringProperty(container, vr::Prop_NamedIconPathDeviceStandby_String, not_ready.c_str());
	vr::VRProperties()->SetStringProperty(container, vr::Prop_NamedIconPathDeviceAlertLow_String, not_ready.c_str());

	//vr::VRProperties()->SetBoolProperty(container, vr::Prop_DeviceIsCharging_Bool, false);
	//vr::VRProperties()->SetFloatProperty(container, vr::Prop_DeviceBatteryPercentage_Float, 0.5f);
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
