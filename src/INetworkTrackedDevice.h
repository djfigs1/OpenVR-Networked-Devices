#pragma once
#include <openvr_driver.h>
#include <string.h>
#include "linalg.h"

class INetworkTrackedDevice : public vr::ITrackedDeviceServerDriver {

public:

	virtual std::string GetSerial() = 0;
	virtual vr::TrackedDeviceIndex_t GetDeviceId() = 0;

	virtual void SetDeviceGlobalTranslation(double(&tvec)[3]) = 0;
	virtual void SetDeviceGlobalQuaternion(vr::HmdQuaternion_t &quat) = 0;

	virtual void updateDeviceTransform(bool visible, double(&rvec)[3], double(&tvec)[3]) = 0;
	virtual vr::EVRInitError Activate(uint32_t unObjectId) = 0;
	virtual void Deactivate() = 0;
	virtual void EnterStandby() = 0;
	virtual void* GetComponent(const char* pchComponentNameAndVersion) = 0;
	virtual void DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize) = 0;

	// ------------------------------------
	// Tracking Methods
	// ------------------------------------
	virtual vr::DriverPose_t GetPose() = 0;
	virtual void RunFrame() = 0;

	~INetworkTrackedDevice() = default;

	static vr::DriverPose_t getDefaultPose() {
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

	static vr::DriverPoseQuaternion_t rvecToQuat(double(&rvec)[3]) {
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
};