#pragma once
#include <thread>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <openvr_driver.h>
#include "VRDriver.h"

class SocketServer
{
public:
	SocketServer(class TrackerProvider* provider);
	~SocketServer();

	static const int RECV_BUFFER_SIZE = 1024;
	char m_recvBuffer[RECV_BUFFER_SIZE] = { 0 };
	bool m_runServer = true;
	class TrackerProvider* provider;
	
	static int initialize_winsock();
	void socket_thread();
	void handleSocketMessage(int packetLength, struct sockaddr* client_addr, SOCKET socket);
	void start();
	void stop();

	enum SOCKET_COMMAND_TYPE {
		ADVERTISE = 0x00,
		CALIBRATE = 0x01,
		UPDATE = 0x02
	};

private:
	std::thread t_socketserver;
};