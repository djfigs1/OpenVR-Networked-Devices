cmake_minimum_required(VERSION 3.26.4)
project(OpenVRNetworkedDevices)

set(DRIVER_NAME "driver_ovr_networked_devices")
set(CMAKE_SHARED_LIBRARY_PREFIX "")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(OpenVR REQUIRED)

add_library(${DRIVER_NAME} SHARED src/driver_factory.cpp
        src/NetworkController.cpp
        src/NetworkGenericDevice.cpp
        src/NetworkReferenceDevice.cpp
        src/NetworkVRDriver.cpp
        src/SocketServer.cpp)

target_link_libraries(${DRIVER_NAME} PUBLIC OpenVR::OpenVR)
if(WIN32)
    target_link_libraries(${DRIVER_NAME} PUBLIC wsock32 ws2_32)
endif()