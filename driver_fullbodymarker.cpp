#include "openvr_driver.h"
#define HMD_DLL_EXPORT extern "C" __declspec( dllexport )

using namespace vr;



HMD_DLL_EXPORT void * HmdDriverFactory(const char * pInterfaceName, int * pReturnCode)
{
	if (0 == strcmp(IServerTrackedDeviceProvider_Version, pInterfaceName))
	{
		return nullptr;
	}
}