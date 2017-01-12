// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_NET_NETFILETRANSFER_
#define _ENGINE_NET_NETFILETRANSFER_

#include "Generic/Patterns/Singleton.h"

#include "Engine/Engine/FrameTime.h"

#include "Engine/Network/Packets/EnginePackets.h"
#include "Engine/Network/Packets/PacketDispatcher.h"

#include "Engine/Online/OnlineMatching.h"

#include <string>
#include <vector>

// Notice: This system could be prone to DoS attacks. Make sure to only use in areas
//		   that are controlled!

class NetConnection;

class NetFileTransfer
{
	MEMORY_ALLOCATOR(NetFileTransfer, "Network");

private:
	NetConnection*	m_connection;
	int				m_slot_hash;
	bool			m_recieving;
	bool			m_is_finished;
	bool			m_is_finished_acked;
	double			m_chunk_timer;
	bool			m_ready;

	DataBuffer		m_data;
	DataBuffer		m_compressed_data;
	int				m_data_offset;

protected:
	enum 
	{
		chunk_size		= 1024,
		send_speed_bps	= 64 * 1024
	};

public:
	NetFileTransfer(int slot_hash, NetConnection* connection, bool recieving);
	NetFileTransfer(std::string slot_name, NetConnection* connection, bool recieving);
	~NetFileTransfer();

	bool Is_Recieving();
	bool Is_Finished();

	DataBuffer& Get_Data();

	INLINE int Get_Slot_Hash()
	{
		return m_slot_hash;
	}

	void Send(std::string path);
	void Send(DataBuffer& data);

	void Recieve_Start(NetPacket_C2S_FileTransferStart* packet);
	void Recieve_Chunk(NetPacket_C2S_FileTransferChunk* packet);
	void Recieve_Finish(NetPacket_C2S_FileTransferFinish* packet);
	void Recieve_Finish_Ack(NetPacket_C2S_FileTransferFinishAck* packet);

	void Poll();

};

#endif

