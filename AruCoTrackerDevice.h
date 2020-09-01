#pragma once

#include <openvr_driver.h>

using namespace vr;

class AruCoTrackerDevice : public vr::ITrackedDeviceServerDriver {

	DriverPose_t GetPose() = 0;
};