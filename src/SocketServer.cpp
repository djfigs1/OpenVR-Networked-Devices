#include "SocketServer.h"
#include <cmath>
#include <stdexcept>
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
	this->runServer = true;
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

	while (this->runServer)
	{
		ZeroMemory(this->receiveBuffer, sizeof(this->receiveBuffer));
		packetLength = recvfrom(s, (char*)this->receiveBuffer, sizeof(this->receiveBuffer), 0, (struct sockaddr*) & client, &len);
		this->handleSocketMessage(packetLength, &client, s);
	}
}

void SocketServer::handleSocketMessage(int packetLength, struct sockaddr_in* client, SOCKET socket) {
	this->currentBufferPosition = 0;
	this->currentBufferSize = packetLength;
	if (packetLength > 0) {
		char command_type = this->readCharFromBuffer();
		switch (command_type) {
		case SOCKET_COMMAND_TYPE::HANDSHAKE:
			this->onHandshake(client, socket);
			break;

		case SOCKET_COMMAND_TYPE::ADVERTISE:
			this->onAdvertise(client, socket);
			break;

		case SOCKET_COMMAND_TYPE::CALIBRATE:
			this->onCalibrate(client, socket);
			break;

		case SOCKET_COMMAND_TYPE::UPDATE:
			this->onUpdate(client, socket);
			break;

		case SOCKET_COMMAND_TYPE::WORLD_TRANSLATE:
			this->onWorldTranslate(client, socket);
			break;

		default:
			return;
		}
	}	
}

void SocketServer::onHandshake(struct sockaddr_in* client_addr, SOCKET socket)
{
	std::string log_message = "Handshake from ";
	char address[16];
	inet_ntop(AF_INET, client_addr, address, sizeof(address));
	log_message += address;
	vr::VRDriverLog()->Log(log_message.c_str());
	this->provider->AddReference();
}

void SocketServer::onAdvertise(struct sockaddr_in* client_addr, SOCKET socket)
{
	if (!this->clientConnected) {
		this->clientConnected = true;
		this->provider->clientDidHandshake();
	}

	try {
		while (this->canReadSizeFromBuffer(1))
		{
			char tracker_id = this->readCharFromBuffer();
			std::string tracker_serial = this->readStringFromBuffer();
			this->provider->AddTracker(tracker_id, tracker_serial.c_str());
		}

		// reply with 0x00 0x01 (send, success)
		char send_buffer[2];
		send_buffer[0] = 0x00;
		send_buffer[1] = true;

		sendto(socket, send_buffer, sizeof(send_buffer), 0, (sockaddr*)client_addr, sizeof(sockaddr_in));
	}
	catch (const std::exception &e) {
		vr::VRDriverLog()->Log("buffer read failed with err");
	}
}

void SocketServer::onCalibrate(struct sockaddr_in* client_addr, SOCKET socket)
{

}

void SocketServer::onUpdate(struct sockaddr_in* client_addr, SOCKET socket)
{
	try {
		while (this->canReadSizeFromBuffer(1))
		{
			char tracker_char = this->readCharFromBuffer();
			char tracker_id = std::abs(tracker_char);
			bool tracker_visible = copysign(1.0, tracker_char) > 0;
			double rvec[3], tvec[3];

			// if the tracker isn't visible, there aren't vecs to parse
			if (tracker_visible)
			{
				// rvec
				this->readDoubleArrayFromBuffer(3, rvec);

				// tvec
				this->readDoubleArrayFromBuffer(3, tvec);
			}

			NetworkGenericDevice* tracker = this->provider->trackers_map[tracker_id];
			if (tracker != nullptr)
			{
				tracker->updateDeviceTransform(tracker_visible, rvec, tvec);
			}
		}
	} catch (const std::exception &e) {
		vr::VRDriverLog()->Log("buffer read failed with err");
	}
}

void SocketServer::onWorldTranslate(struct sockaddr_in* client_addr, SOCKET socket)
{
	if (this->canReadSizeFromBuffer(2 * sizeof(double[3])))
	{
		double rvec[3], tvec[3];
		this->readDoubleArrayFromBuffer(3, rvec);
		this->readDoubleArrayFromBuffer(3, tvec);
		vr::DriverPoseQuaternion_t quat = INetworkTrackedDevice::rvecToQuat(rvec);
		this->provider->globalQuaternion = { quat.w, quat.x, quat.y, quat.z };
		memcpy(this->provider->globalTranslation, &tvec, sizeof(double[3]));
	}
}

void SocketServer::onSetProperties(sockaddr_in* client_addr, SOCKET socket)
{
	while (this->canReadSizeFromBuffer(1)) {
		char tracker_id = this->readCharFromBuffer();
		NetworkGenericDevice* tracker = this->provider->trackers_map[tracker_id];
		if (tracker != nullptr)
		{
			int property_type = this->readIntegerFromBuffer();
		}
	}
}

/// <summary>
/// Simply sends an immediate reply to the client
/// </summary>
/// <param name="client_addr"></param>
/// <param name="socket"></param>
void SocketServer::onPing(sockaddr_in* client_addr, SOCKET socket)
{
	char reply_payload[1] = { 0x01 };
	sendto(socket, reply_payload, sizeof(reply_payload), 0, (sockaddr*)client_addr, sizeof(sockaddr_in));
}

void SocketServer::stop() {
	if (this->t_socketserver.joinable())
	{
		this->runServer = false;
		this->t_socketserver.join();
	}

}

int SocketServer::initialize_winsock()
{
	WSADATA wsaData;
	return WSAStartup(MAKEWORD(2, 2), &wsaData);
}

bool SocketServer::canReadSizeFromBuffer(size_t size) {
	return this->currentBufferPosition + size <= this->currentBufferSize;
}

char SocketServer::readCharFromBuffer() {
	if (this->canReadSizeFromBuffer(sizeof(char))) {
		char val = this->receiveBuffer[this->currentBufferPosition];
		this->currentBufferPosition += sizeof(char);
		return val;
	}
	throw std::runtime_error("buffer is too small to read from");
}

double SocketServer::readDoubleFromBuffer() {
	if (this->canReadSizeFromBuffer(sizeof(double))) {
		double val = *((double*)&this->receiveBuffer[this->currentBufferPosition]);
		// Increment the current buffer position by the size of a double
		this->currentBufferPosition += sizeof(double);
		return val;
	}
	throw std::runtime_error("buffer is too small to read from");
}

int SocketServer::readIntegerFromBuffer() {
	if (this->canReadSizeFromBuffer(sizeof(int))) {
		int val = *((int*) &this->receiveBuffer[this->currentBufferPosition]);
		// Increment the current buffer position by the size of a double
		this->currentBufferPosition += sizeof(int);
		return val;
	}
	throw std::runtime_error("buffer is too small to read from");
}

void SocketServer::readDoubleArrayFromBuffer(const size_t size, double* dest) {
	// Size testing doesn't have to occur becuase it will happen within the double read
	for (int i = 0; i < size; i++) {
		dest[i] = this->readDoubleFromBuffer();
	}
}

std::string SocketServer::readStringFromBuffer() {
	std::string return_str;
	while (this->canReadSizeFromBuffer(sizeof(char))) {
		char str_character = this->readCharFromBuffer();
		if (str_character != 0x00) { 
			return_str += str_character;
		} else {
			break;
		}
	}
	return return_str;
}

