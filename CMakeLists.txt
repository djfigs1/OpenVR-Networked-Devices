cmake_minimum_required(VERSION 3.26.4)
project(OpenVRNetworkedDevices)

set(DRIVER_NAME "driver_ovr_networked_devices")
set(CMAKE_SHARED_LIBRARY_PREFIX "")
set(PROTOBUF_OUTPUT_DIR "${PROJECT_SOURCE_DIR}/proto")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# openvr
find_package(OpenVR REQUIRED)

add_library(${DRIVER_NAME} SHARED src/driver_factory.cpp
        src/NetworkVRDriver.cpp
        src/NetworkServer.cpp
        src/network.h
        src/network.cpp
        src/NetworkClient.h
        src/NetworkClient.cpp
        src/NetworkDevice.cpp
        src/NetworkDeviceComponent.cpp
        src/NetworkDeviceComponent.h)

# protobuf
include(FindProtobuf)
find_package(Protobuf REQUIRED)
add_library(OVRNDProto STATIC ./ovrnd.proto)
target_include_directories(OVRNDProto PUBLIC ${Protobuf_INCLUDE_DIRS})
protobuf_generate(
        TARGET OVRNDProto
        PROTOC_OUT_DIR ${PROTOBUF_OUTPUT_DIR})

add_dependencies(${DRIVER_NAME} OVRNDProto)
target_include_directories(${DRIVER_NAME} PUBLIC ${PROTOBUF_OUTPUT_DIR})
target_link_libraries(${DRIVER_NAME} PUBLIC OVRNDProto)
target_link_libraries(${DRIVER_NAME} PUBLIC OpenVR::OpenVR)
target_link_libraries(${DRIVER_NAME} PUBLIC ${Protobuf_LIBRARIES})
if(WIN32)
    target_link_libraries(${DRIVER_NAME} PRIVATE ws2_32)
endif()