cmake_minimum_required(VERSION 3.26.4)
project(OpenVRNetworkedDevices)

set(DRIVER_NAME "driver_ovr_networked_devices")
set(CMAKE_SHARED_LIBRARY_PREFIX "")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# openvr
find_package(OpenVR REQUIRED)

# protobuf
include(FindProtobuf)
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIR})

add_library(${DRIVER_NAME} SHARED src/driver_factory.cpp
        src/NetworkController.cpp
        src/NetworkGenericDevice.cpp
        src/NetworkReferenceDevice.cpp
        src/NetworkVRDriver.cpp
        src/SocketServer.cpp
        src/proto/ovrnd.pb.cc)

target_link_libraries(${DRIVER_NAME} PUBLIC OpenVR::OpenVR)
target_link_libraries(${DRIVER_NAME} PUBLIC ${PROTOBUF_LIBRARY})
if(WIN32)
    target_link_libraries(${DRIVER_NAME} PRIVATE ws2_32)
endif()