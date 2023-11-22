#pragma once
#include <thread>
#include <string.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <openvr_driver.h>
#include "NetworkVRDriver.h"

class SocketServer
{
public:
	SocketServer(class NetworkVRDriver* provider);
	~SocketServer();

	static const int RECV_BUFFER_SIZE = 1024;
	char receiveBuffer[RECV_BUFFER_SIZE] = { 0 };
	bool runServer = true;
	bool clientConnected = false;
	class NetworkVRDriver* provider;
	
	static int initialize_winsock();
	void socket_thread();
	void start();
	void stop();

	void handleSocketMessage(int packetLength,  struct sockaddr_in* client_addr, SOCKET socket);
	void onHandshake(struct sockaddr_in* client_addr, SOCKET socket);
	void onAdvertise(struct sockaddr_in* client_addr, SOCKET socket);
	void onCalibrate(struct sockaddr_in* client_addr, SOCKET socket);
	void onUpdate(struct sockaddr_in* client_addr, SOCKET socket);
	void onWorldTranslate(struct sockaddr_in* client_addr, SOCKET socket);
	void onSetProperties(struct sockaddr_in* client_addr, SOCKET socket);
	void onPing(struct sockaddr_in* client_addr, SOCKET socket);

	enum SOCKET_COMMAND_TYPE {
		HANDSHAKE = 0x00,
		ADVERTISE = 0x01,
		CALIBRATE = 0x02,
		UPDATE = 0x03,
		WORLD_TRANSLATE = 0x04,
		SET_PROPERTIES = 0x05,
		PING = 0x06,
	};

private:
	int currentBufferPosition = 0;
	int currentBufferSize = 0;
	bool canReadSizeFromBuffer(size_t size);
	char readCharFromBuffer();
	double readDoubleFromBuffer();
	int readIntegerFromBuffer();
	void readDoubleArrayFromBuffer(const size_t size, double* dest);
	std::string readStringFromBuffer();
	std::thread t_socketserver;
};