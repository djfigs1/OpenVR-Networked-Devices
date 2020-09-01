#include "NetworkTrackerReference.h"

vr::EVRInitError TrackingReference::Activate(uint32_t unObjectId)
{
	vr::VRServerDriverHost()->TrackedDevicePoseUpdated(unObjectId, this->GetPose(), sizeof(vr::DriverPose_t));
	vr::PropertyContainerHandle_t propertyContainer = vr::VRProperties()->TrackedDeviceToPropertyContainer(unObjectId);
	vr::VRProperties()->SetUint64Property(propertyContainer, vr::Prop_CurrentUniverseId_Uint64, 10);


	return vr::VRInitError_None;
}

void TrackingReference::Deactivate()
{
}

void TrackingReference::EnterStandby()
{
}

void* TrackingReference::GetComponent(const char* pchComponentNameAndVersion)
{
	return nullptr;
}

void TrackingReference::DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize)
{
}

vr::DriverPose_t TrackingReference::GetPose()
{
	vr::DriverPose_t out_pose = { 0 };
	out_pose.deviceIsConnected = true;
	out_pose.poseIsValid = true;
	out_pose.result = vr::ETrackingResult::TrackingResult_Running_OK;
	out_pose.willDriftInYaw = false;
	out_pose.shouldApplyHeadModel = false;
	out_pose.vecPosition[0] = out_pose.vecPosition[1] = out_pose.vecPosition[2] = 0;
	out_pose.qDriverFromHeadRotation.w = out_pose.qWorldFromDriverRotation.w = out_pose.qRotation.w = 1.0;
	return out_pose;
}
