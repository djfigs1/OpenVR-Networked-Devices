#pragma once
#include <thread>
#include <vector>
#include <string>
#include <openvr_driver.h>
#include "network.h"
#include "NetworkVRDriver.h"
#include "NetworkClient.h"

#define RECV_BUFFER_SIZE 65535

class NetworkVRDriver;
class NetworkClient;

class NetworkServer
{
public:
	explicit NetworkServer(NetworkVRDriver& driver_ref);
	~NetworkServer();

	bool runServer = true;


	static int initialize_winsock();
    NetworkClient *findClientBySerial(const std::string& serial);
    NetworkClient *findClientByAddress(const struct sockaddr_in* address);

    void sendClientMessage(const ServerMessage &message, NetworkClient &client) const;
	void socket_thread();
	void start();
	void stop();
    void updateClients();

private:
    void handleClientMessage(int packetLength, struct sockaddr_in* client);
    NetworkClient * handleHello(const HelloRequest &request, struct sockaddr_in *client_addr);
    void handleRegisterDevice(const RegisterDevicesRequest &request, NetworkClient *client);
    void handleUpdateDevice(const UpdateDeviceRequest& request);
    void handleAddComponent(const AddComponentsRequest& request);


    NetworkVRDriver& driver;
    std::vector<NetworkClient*> clients;
    int sd;
    char socket_buffer[RECV_BUFFER_SIZE] = { 0 };
	std::thread recv_thread;
};