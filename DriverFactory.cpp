#include "DriverFactory.h"

static std::shared_ptr<TrackerProvider> driver;

void* HmdDriverFactory(const char* interface_name, int* return_code) {
	if (std::strcmp(interface_name, vr::IServerTrackedDeviceProvider_Version) == 0) {
		if (!driver) {
			driver = std::make_shared<TrackerProvider>();
		}

		return GetDriver();
	}

	if (return_code) {
		*return_code = vr::VRInitError_Init_InterfaceNotFound;
	}

	return nullptr;
}

TrackerProvider* GetDriver()
{
	return driver.get();
}