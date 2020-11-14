#pragma once
#include <openvr_driver.h>
#include <memory>
#include <map>

#include "NetworkTrackedDevice.h"
#include "SocketServer.h"

#define LOG(msg) vr::VRDriverLog()->Log(msg)

class NetworkVRDriver : public vr::IServerTrackedDeviceProvider
{
public:
	NetworkVRDriver();
	~NetworkVRDriver();

	class SocketServer* p_socketServer;
	std::map<char, class NetworkTrackedDevice*> trackers_map;
	
	
	double globalTranslation[3];
	vr::HmdQuaternion_t globalQuaternion;
	class NetworkTrackedDevice* reference = nullptr;
	
	static vr::DriverPoseQuaternion_t rvecToQuat(double (&rvec)[3]);
	void clientDidHandshake();
	void AddReference();
	void AddTracker(char id, const char* tracker_name);

	// Implementation of ITrackedDeviceServerDriver
	vr::EVRInitError Init(vr::IVRDriverContext* pDriverContext);
	void Cleanup();
	const char* const* GetInterfaceVersions();
	void RunFrame();
	bool ShouldBlockStandbyMode();
	void EnterStandby();
	void LeaveStandby();

};

