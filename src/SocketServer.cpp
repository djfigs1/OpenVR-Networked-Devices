#include "SocketServer.h"
#include <cmath>
#include <math.h>
#include <string>
#include "linalg.h"


SocketServer::SocketServer(NetworkVRDriver* provider)
{
	this->provider = provider;
}


SocketServer::~SocketServer()
{

}

void SocketServer::start()
{
	this->m_runServer = true;
	this->t_socketserver = std::thread(&SocketServer::socket_thread, this);
}



void SocketServer::socket_thread()
{
	SOCKET s = INVALID_SOCKET;
	struct sockaddr_in server, client;

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s == INVALID_SOCKET)
	{
		vr::VRDriverLog()->Log("Could not create socket");
		return;
	}

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(8082);

	if (bind(s, (struct sockaddr*) & server, sizeof(server)) == SOCKET_ERROR)
	{
		int error = WSAGetLastError();

		vr::VRDriverLog()->Log("bind failed with err");

		return;
	}


	int len, packetLength;
	len = sizeof(client);

	while (this->m_runServer)
	{
		ZeroMemory(this->m_recvBuffer, sizeof(this->m_recvBuffer));
		packetLength = recvfrom(s, (char*)this->m_recvBuffer, sizeof(this->m_recvBuffer), 0, (struct sockaddr*) & client, &len);
		this->handleSocketMessage(packetLength, &client, s);
	}
}

void SocketServer::handleSocketMessage(int packetLength, struct sockaddr_in* client, SOCKET socket) {
	char command_type = this->m_recvBuffer[0];
	int current_bit = 1;
	switch (command_type) {
		case SOCKET_COMMAND_TYPE::HANDSHAKE:
			this->onHandshake(current_bit, packetLength, client, socket);
			break;

		case SOCKET_COMMAND_TYPE::ADVERTISE:
			this->onAdvertise(current_bit, packetLength, client, socket);
			break;

		case SOCKET_COMMAND_TYPE::CALIBRATE:
			this->onCalibrate(current_bit, packetLength, client, socket);
			break;

		case SOCKET_COMMAND_TYPE::UPDATE:
			this->onUpdate(current_bit, packetLength, client, socket);
			break;

		case SOCKET_COMMAND_TYPE::WORLD_TRANSLATE:
			this->onWorldTranslate(current_bit, packetLength, client, socket);
			break;

		default:
			return;
	}
}

void SocketServer::onHandshake(int& current_bit, int packet_length, struct sockaddr_in* client_addr, SOCKET socket)
{
	std::string log_message = "Handshake from ";
	char address[16];
	inet_ntop(AF_INET, client_addr, address, sizeof(address));
	log_message += address;
	vr::VRDriverLog()->Log(log_message.c_str());
	this->provider->AddReference();
}

void SocketServer::onAdvertise(int& current_bit, int packet_length, struct sockaddr_in* client_addr, SOCKET socket)
{
	if (!this->m_clientConnected) {
		this->m_clientConnected = true;
		this->provider->clientDidHandshake();
	}

	while (current_bit < packet_length)
	{
		char tracker_id = this->m_recvBuffer[current_bit];
		current_bit += 1;
		std::string tracker_serial = "";
		while (m_recvBuffer[current_bit] != 0x00) { // 0x00 marks terminator for string
			tracker_serial += m_recvBuffer[current_bit];
			current_bit += 1;
		}
		current_bit += 1;
		this->provider->AddTracker(tracker_id, tracker_serial.c_str());
	}

	// reply with 0x00 0x01 (send, success)
	char send_buffer[2];
	send_buffer[0] = 0x00;
	send_buffer[1] = true;

	sendto(socket, send_buffer, sizeof(send_buffer), 0, (sockaddr*) client_addr, sizeof(sockaddr_in));
}

void SocketServer::onCalibrate(int& current_bit, int packet_length, struct sockaddr_in* client_addr, SOCKET socket)
{
}

void SocketServer::onUpdate(int& current_bit, int packet_length, struct sockaddr_in* client_addr, SOCKET socket)
{
	while (current_bit < packet_length)
	{
		char tracker_char = this->m_recvBuffer[current_bit];
		char tracker_id = std::abs(tracker_char);
		bool tracker_visible = copysign(1.0, tracker_char) > 0;
		current_bit += 1;
		double rvec[3], tvec[3];

		// if the tracker isn't visible, there aren't vecs to parse
		if (tracker_visible)
		{
			// rvec
			memcpy(&rvec, &this->m_recvBuffer[current_bit], sizeof(double[3]));
			current_bit += sizeof(double[3]);

			// tvec
			memcpy(&tvec, &this->m_recvBuffer[current_bit], sizeof(double[3]));
			current_bit += sizeof(double[3]);
		}

		NetworkTrackedDevice* tracker = this->provider->trackers_map[tracker_id];
		if (tracker != nullptr)
		{
			tracker->updateTrackerWith(tracker_visible, rvec, tvec);
		}
	}
}

void SocketServer::onWorldTranslate(int& current_bit, int packet_length, struct sockaddr_in* client_addr, SOCKET socket)
{
	if (packet_length == 1 + 2 * sizeof(double[3]))
	{
		double rvec[3], tvec[3];
		memcpy(&rvec, &this->m_recvBuffer[current_bit], sizeof(double[3]));
		current_bit += sizeof(double[3]);
		memcpy(&tvec, &this->m_recvBuffer[current_bit], sizeof(double[3]));
		vr::DriverPoseQuaternion_t quat = NetworkVRDriver::rvecToQuat(rvec);
		this->provider->globalQuaternion = { quat.w, quat.x, quat.y, quat.z };
		memcpy(this->provider->globalTranslation, &tvec, sizeof(double[3]));
	}
}

void SocketServer::stop() {
	if (this->t_socketserver.joinable())
	{
		this->m_runServer = false;
		this->t_socketserver.join();
	}

}

int SocketServer::initialize_winsock()
{
	WSADATA wsaData;
	return WSAStartup(MAKEWORD(2, 2), &wsaData);
}

