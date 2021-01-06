#include "NetworkGenericDevice.h"
#include <math.h>
#include "linalg.h"

NetworkGenericDevice::NetworkGenericDevice(const char* serial) {
	this->serial_ = serial;
}

vr::TrackedDeviceIndex_t NetworkGenericDevice::GetDeviceId() {
	return this->deviceid_;
}

std::string NetworkGenericDevice::GetSerial() {
	return this->serial_;
}

void NetworkGenericDevice::SetDeviceGlobalTranslation(double(&tvec)[3]) {
	this->global_translation_ = &tvec;
}

void NetworkGenericDevice::SetDeviceGlobalQuaternion(vr::HmdQuaternion_t& quat) {
	this->global_quaternion_ = &quat;
}

void NetworkGenericDevice::updateDeviceTransform(bool visible, double(&rvec)[3], double(&tvec)[3])
{
	vr::DriverPose_t pose = getDefaultPose();
	pose.result = visible ? vr::ETrackingResult::TrackingResult_Running_OK : vr::ETrackingResult::TrackingResult_Running_OutOfRange;
	pose.poseIsValid = visible;

	vr::HmdQuaternion_t qt = *this->global_quaternion_;
	pose.qWorldFromDriverRotation = qt;
	memcpy(pose.vecWorldFromDriverTranslation, this->global_quaternion_, sizeof(double[3]));
	if (visible) {
		memcpy(pose.vecPosition, tvec, sizeof(double[3]));
		vr::DriverPoseQuaternion_t q = INetworkTrackedDevice::rvecToQuat(rvec);
		memcpy(&pose.qRotation, &q, sizeof(vr::DriverPoseQuaternion_t));
	}

	vr::VRServerDriverHost()->TrackedDevicePoseUpdated(this->deviceid_, pose, sizeof(vr::DriverPose_t));

}

vr::EVRInitError NetworkGenericDevice::Activate(uint32_t unObjectId)
{
	this->deviceid_ = unObjectId;
	vr::PropertyContainerHandle_t container = vr::VRProperties()->TrackedDeviceToPropertyContainer(unObjectId);
	std::string ready = "{networkvr}/icons/tracked_device/ready.png";
	std::string not_ready = "{networkvr}/icons/tracked_device/not_ready.png";

	vr::VRProperties()->SetStringProperty(container, vr::Prop_NamedIconPathDeviceReady_String, ready.c_str());

	vr::VRProperties()->SetStringProperty(container, vr::Prop_NamedIconPathDeviceOff_String, not_ready.c_str());
	vr::VRProperties()->SetStringProperty(container, vr::Prop_NamedIconPathDeviceSearching_String, not_ready.c_str());
	vr::VRProperties()->SetStringProperty(container, vr::Prop_NamedIconPathDeviceSearchingAlert_String, not_ready.c_str());
	vr::VRProperties()->SetStringProperty(container, vr::Prop_NamedIconPathDeviceReadyAlert_String, not_ready.c_str());
	vr::VRProperties()->SetStringProperty(container, vr::Prop_NamedIconPathDeviceNotReady_String, not_ready.c_str());
	vr::VRProperties()->SetStringProperty(container, vr::Prop_NamedIconPathDeviceStandby_String, not_ready.c_str());
	vr::VRProperties()->SetStringProperty(container, vr::Prop_NamedIconPathDeviceAlertLow_String, not_ready.c_str());
	return vr::VRInitError_None;
}

void NetworkGenericDevice::Deactivate()
{
}

void NetworkGenericDevice::EnterStandby()
{

}

void* NetworkGenericDevice::GetComponent(const char* pchComponentNameAndVersion)
{
	return nullptr;
}

void NetworkGenericDevice::DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize)
{
}

vr::DriverPose_t NetworkGenericDevice::GetPose()
{
	vr::DriverPose_t pose = getDefaultPose();
	pose.poseIsValid = false;
	pose.result = vr::ETrackingResult::TrackingResult_Uninitialized;
	return pose;
}

void NetworkGenericDevice::RunFrame() {
	vr::VRServerDriverHost()->TrackedDevicePoseUpdated(this->deviceid_, this->GetPose(), sizeof(vr::DriverPose_t));
}
