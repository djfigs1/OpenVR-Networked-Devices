#pragma once
#include <openvr_driver.h>
#include "NetworkVRDriver.h"
#include "DriverFactory.h"

class NetworkTrackedDevice : public vr::ITrackedDeviceServerDriver {

public:
	NetworkTrackedDevice(const char* serial);
	~NetworkTrackedDevice();
	char* serial;
	bool isReferenceDevice = false;

	static vr::DriverPose_t getDefaultPose();
	void updateTrackerWith(bool visible, double(&rvec)[3], double(&tvec)[3]);

	vr::EVRInitError Activate(uint32_t unObjectId);
	void Deactivate();
	void EnterStandby();
	void* GetComponent(const char* pchComponentNameAndVersion);
	void DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize);

	// ------------------------------------
	// Tracking Methods
	// ------------------------------------
	vr::DriverPose_t GetPose();
	vr::TrackedDeviceIndex_t deviceId = 0;
	void RunFrame();
};