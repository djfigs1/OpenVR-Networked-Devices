#pragma once

#include <cstdlib>
#include <memory>
#include "VRDriver.h"
#include <openvr_driver.h>

extern "C" __declspec(dllexport) void* HmdDriverFactory(const char* interface_name, int* return_code);

TrackerProvider* GetDriver();