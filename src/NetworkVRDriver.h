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
	double globalTranslation[3];
	vr::HmdQuaternion_t globalQuaternion;
	std::map<char, class NetworkTrackedDevice*> trackers_map;
	static vr::DriverPoseQuaternion_t rvecToQuat(double (&rvec)[3]);
	void ClearTrackers();
	void AddTracker(char id, const char* tracker_name);
	bool RemoveTracker(char id);

	// Implementation of ITrackedDeviceServerDriver
	vr::EVRInitError Init(vr::IVRDriverContext* pDriverContext);
	void Cleanup();
	const char* const* GetInterfaceVersions();
	void RunFrame();
	bool ShouldBlockStandbyMode();
	void EnterStandby();
	void LeaveStandby();
};

