#pragma once
#include <thread>
#include <string.h>
#define NOMINMAX
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
	char m_recvBuffer[RECV_BUFFER_SIZE] = { 0 };
	bool m_runServer = true;
	bool m_clientConnected = false;
	class NetworkVRDriver* provider;
	
	static int initialize_winsock();
	void socket_thread();
	void start();
	void stop();

	void handleSocketMessage(int packetLength, struct sockaddr_in* client_addr, SOCKET socket);
	void onHandshake(int &current_bit, int packet_length, struct sockaddr_in* client_addr, SOCKET socket);
	void onAdvertise(int& current_bit, int packet_length, struct sockaddr_in* client_addr, SOCKET socket);
	void onCalibrate(int& current_bit, int packet_length, struct sockaddr_in* client_addr, SOCKET socket);
	void onUpdate(int& current_bit, int packet_length, struct sockaddr_in* client_addr, SOCKET socket);
	void onWorldTranslate(int& current_bit, int packet_length, struct sockaddr_in* client_addr, SOCKET socket);

	enum SOCKET_COMMAND_TYPE {
		HANDSHAKE = 0x00,
		ADVERTISE = 0x01,
		CALIBRATE = 0x02,
		UPDATE = 0x03,
		WORLD_TRANSLATE = 0x04,
		SET_PROPERTIES = 0x05,
	};

private:
	std::thread t_socketserver;
};