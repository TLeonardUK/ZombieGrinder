// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_MONITOR_SERVER_
#define _GAME_MONITOR_SERVER_

#include "Engine/Engine/FrameTime.h"
#include "Generic/Types/DataBuffer.h"

#include "Game/Monitor/Packets/BaseMonitorPacket.h"
#include "Game/Monitor/Packets/GameStateMonitorPacket.h"

#include <string>
#include <vector>

#ifdef PLATFORM_WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <netinet/tcp.h>
#include <unistd.h>
#define SOCKET int
#define INVALID_SOCKET -1
#endif

class MonitorServer;

struct MonitorServerConnection
{
	MEMORY_ALLOCATOR(MonitorServerConnection, "Game");

private:
	friend class MonitorServer;

	SOCKET m_socket;
	int m_id;

	std::string m_remote_ip;
	int m_remote_port;

	DataBuffer m_payload;
	int m_payload_length;
	int m_payload_read;
	bool m_reading_payload;

	GameStateMonitorPacket m_game_state;

public:
	MonitorServerConnection();
	~MonitorServerConnection();

	void Close();
	void PollPeriodic();
	bool Poll();

	bool IsConnected();

	void Send(BaseMonitorPacket* Packet);
	void Recieve(const char* json);

};

class MonitorServer
{
	MEMORY_ALLOCATOR(MonitorServer, "Game");

private:
	SOCKET m_socket;
	int m_id_counter;

	std::vector<MonitorServerConnection> m_connections;

public:
	MonitorServer();
	~MonitorServer();

	void Init();
	void Tick(const FrameTime& time);

};

#endif

