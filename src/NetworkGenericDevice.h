#pragma once
#include "INetworkTrackedDevice.h" 

class NetworkGenericDevice : public INetworkTrackedDevice {

public: 
	NetworkGenericDevice(const char* serial);
	~NetworkGenericDevice() = default;

	void SetDeviceGlobalTranslation(double(&tvec)[3]) override;
	void SetDeviceGlobalQuaternion(vr::HmdQuaternion_t& quat) override;
	std::string GetSerial();
	vr::TrackedDeviceIndex_t GetDeviceId();

	void updateDeviceTransform(bool visible, double(&rvec)[3], double(&tvec)[3]);
	vr::EVRInitError Activate(uint32_t unObjectId);
	void Deactivate();
	void EnterStandby();
	void* GetComponent(const char* pchComponentNameAndVersion);
	void DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize);

	// ------------------------------------
	// Tracking Methods
	// ------------------------------------
	vr::DriverPose_t GetPose();
	void RunFrame();

private:
	std::string serial_;
	vr::TrackedDeviceIndex_t deviceid_;
	double	(*global_translation_)[3];
	vr::HmdQuaternion_t* global_quaternion_;
};