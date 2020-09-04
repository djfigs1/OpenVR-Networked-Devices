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
	
	if (bind(s, (struct sockaddr*) &server, sizeof(server)) == SOCKET_ERROR)
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
		packetLength = recvfrom(s, (char*)this->m_recvBuffer, sizeof(this->m_recvBuffer), 0, (struct sockaddr*)&client, &len);
		this->handleSocketMessage(packetLength, (struct sockaddr*) &client, s);

		//memcpy(tracker->position, buffer, sizeof(double[3]));
		//tracker->isConnected = true;
		//tracker->RunFrame();
	}
}

void SocketServer::handleSocketMessage(int packetLength, struct sockaddr* client, SOCKET socket) {
	char command_type = this->m_recvBuffer[0];
	int current_bit = 1;
	switch (command_type) {
	case SOCKET_COMMAND_TYPE::ADVERTISE:
			{
				while (current_bit < packetLength)
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

				sendto(socket, send_buffer, sizeof(send_buffer), 0, client, sizeof(sockaddr_in));
			}
			break;

		case SOCKET_COMMAND_TYPE::CALIBRATE:
			// IRVEC, ITVEC, ORVEC, OTVEC
			{
				if (packetLength - current_bit == 4 * 24)
				{
					double i_rvec[3];
					double i_tvec[3];
					double o_rvec[3];
					double o_tvec[3];

					memcpy(&i_rvec, &this->m_recvBuffer[1], sizeof(double[3]));
					memcpy(&i_tvec, &this->m_recvBuffer[25], sizeof(double[3]));
					memcpy(&o_rvec, &this->m_recvBuffer[49], sizeof(double[3]));
					memcpy(&o_tvec, &this->m_recvBuffer[73], sizeof(double[3]));
				
					linalg::vec<double, 3> ila_tvec = { i_tvec[0], i_tvec[1], i_tvec[2] };
					linalg::vec<double, 3> ola_tvec = { o_tvec[0], o_tvec[1], o_tvec[2] };
					linalg::vec<double, 3> r_tvec = ola_tvec - ila_tvec;
					this->provider->globalTranslation[0] = r_tvec[0];
					this->provider->globalTranslation[1] = r_tvec[2];
					this->provider->globalTranslation[2] = r_tvec[2];

					vr::DriverPoseQuaternion_t iq = NetworkVRDriver::rvecToQuat(i_rvec);
					vr::DriverPoseQuaternion_t oq = NetworkVRDriver::rvecToQuat(o_rvec);
					linalg::vec<double, 4> ilaq = { iq.w, iq.x, iq.y, iq.z };
					linalg::vec<double, 4> olaq = { oq.w, oq.x, oq.y, oq.z };
					linalg::vec<double, 4> dlaq = olaq * linalg::qinv(ilaq);
					this->provider->globalQuaternion.w = dlaq.w;
					this->provider->globalQuaternion.x = dlaq.x;
					this->provider->globalQuaternion.y = dlaq.y;
					this->provider->globalQuaternion.z = dlaq.z;

				}
			}
			break;

		case SOCKET_COMMAND_TYPE::UPDATE:
			while (current_bit < packetLength)
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
			break;

		default:
			return;
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

