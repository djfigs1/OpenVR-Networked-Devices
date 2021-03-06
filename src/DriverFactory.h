#pragma once

#include <cstdlib>
#include <memory>
#include "NetworkVRDriver.h"
#include <openvr_driver.h>

extern "C" __declspec(dllexport) void* HmdDriverFactory(const char* interface_name, int* return_code);

class NetworkVRDriver* GetDriver();