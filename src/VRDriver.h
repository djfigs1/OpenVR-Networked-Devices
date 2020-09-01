#pragma once
#include <openvr_driver.h>
#include <memory>
#include <map>
#include "NetworkTracker.h"
#include "NetworkTrackerReference.h"
#include "SocketServer.h"
#define LOG(msg) vr::VRDriverLog()->Log(msg);

class TrackerProvider :
	public vr::IServerTrackedDeviceProvider
{
public:
	class SocketServer* p_socketServer;
	std::map<char, Tracker*> trackers_map;
	std::vector<TrackingReference> references;
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

