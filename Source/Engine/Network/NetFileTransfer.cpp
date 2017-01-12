// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Network/NetFileTransfer.h"
#include "Engine/Network/NetConnection.h"
#include "Engine/Network/Packets/EnginePackets.h"
#include "Generic/Helper/StringHelper.h"

#include "Engine/Platform/Platform.h"

#include "Engine/IO/StreamFactory.h"

NetFileTransfer::NetFileTransfer(std::string storage_name, NetConnection* connection, bool recieving)
	: m_connection(connection)
	, m_recieving(recieving)
	, m_is_finished(false)
	, m_ready(false)
	, m_data_offset(0)
	, m_is_finished_acked(false)
{
	m_slot_hash = StringHelper::Hash(storage_name.c_str());
	m_connection->Register_File_Transfer(this);
}

NetFileTransfer::NetFileTransfer(int slot_hash, NetConnection* connection, bool recieving)
	: m_connection(connection)
	, m_slot_hash(slot_hash)
	, m_recieving(recieving)
	, m_is_finished(false)
	, m_ready(false)
	, m_data_offset(0)
	, m_is_finished_acked(false)
{
	m_connection->Register_File_Transfer(this);
}

NetFileTransfer::~NetFileTransfer()
{
	m_connection->Unregister_File_Transfer(this);
}

bool NetFileTransfer::Is_Recieving()
{
	return m_recieving;
}

bool NetFileTransfer::Is_Finished()
{
	if (m_recieving == true)
		return m_is_finished;
	else
		return m_is_finished && m_is_finished_acked;
}

DataBuffer& NetFileTransfer::Get_Data()
{
	return m_data;
}

void NetFileTransfer::Send(std::string path)
{
	DataBuffer buffer;

	// Read entire file into memory.
	Stream* stream = StreamFactory::Open(path.c_str(), StreamMode::Read);
	DBG_ASSERT(stream != NULL);
	buffer.Reserve(stream->Bytes_Remaining());
	stream->ReadBuffer(buffer.Buffer(), 0, stream->Bytes_Remaining());
	SAFE_DELETE(stream);

	// Send!
	Send(buffer);

	DBG_LOG("Sending file '%s' to storage slot '0x%08x' on connection 0x%08x, compressed data size is %i kb.", path.c_str(), m_slot_hash, m_connection, m_compressed_data.Size() / 1024);
}

void NetFileTransfer::Send(DataBuffer& buffer)
{
	m_recieving = false;
	m_chunk_timer = Platform::Get()->Get_Ticks();
	m_data_offset = 0;
	m_data = buffer;

	// Compress dat shit.
	m_data.Compress(m_compressed_data);
	m_data_offset = 0;

	NetPacket_C2S_FileTransferStart packet;
	packet.slot_hash = m_slot_hash;
	m_connection->Send(&packet, NET_CONNECTION_CHANNEL_FILE_TRANSFER, true);

	m_ready = true;

	DBG_LOG("Sending buffer to storage slot '0x%08x' on connection 0x%08x, compressed data size is %i kb.", m_slot_hash, m_connection, m_compressed_data.Size() / 1024);
}

void NetFileTransfer::Recieve_Start(NetPacket_C2S_FileTransferStart* packet)
{
	m_recieving = true;
	m_data_offset = 0;
	m_ready = true;
}

void NetFileTransfer::Recieve_Chunk(NetPacket_C2S_FileTransferChunk* packet)
{
	m_compressed_data.Reserve(m_compressed_data.Size() + packet->data.Size());
	memcpy(m_compressed_data.Buffer() + m_data_offset, packet->data.Buffer(), packet->data.Size());
	m_data_offset += packet->data.Size();
}

void NetFileTransfer::Recieve_Finish(NetPacket_C2S_FileTransferFinish* packet)
{
	m_is_finished = true;
	m_compressed_data.Decompress(m_data);

	// Send ack.
	NetPacket_C2S_FileTransferFinishAck p;
	p.slot_hash = m_slot_hash;
	packet->Get_Recieved_From()->Send(&p, NET_CONNECTION_CHANNEL_FILE_TRANSFER, true);

	DBG_LOG("File transfer in slot 0x%08x of connection 0x%08x has finished with size %i KB.", m_slot_hash, this, m_data.Size() / 1024);
}

void NetFileTransfer::Recieve_Finish_Ack(NetPacket_C2S_FileTransferFinishAck* packet)
{
	m_is_finished_acked = true;
	DBG_LOG("File transfer in slot 0x%08x of connection 0x%08x was acknowledged as finished.", m_slot_hash, this);
}

void NetFileTransfer::Poll()
{
	// Transfer isn't ready yet! :(
	if (!m_ready)
	{
		return;
	}

	// If sending, see if we want to send next chunk.
	if (m_is_finished == false && m_recieving == false)
	{
		const double time = Platform::Get()->Get_Ticks();
		const double chunk_send_interval = 1000.0f / (send_speed_bps / chunk_size);
		const double elapsed = time - m_chunk_timer;

		// TODO: Some kind of back-off to prevent saturation!

		if (elapsed >= chunk_send_interval)
		{
			const int bytes_remaining = m_compressed_data.Size() - m_data_offset;
			const int this_chunk_size = Min(chunk_size, bytes_remaining);

			if (this_chunk_size <= 0)
			{
				DBG_LOG("Finished sending file to storage slot '0x%08x' on connection '0x%08x'", m_slot_hash, this);
				m_is_finished = true;

				NetPacket_C2S_FileTransferFinish packet;
				packet.slot_hash = m_slot_hash;

				m_connection->Send(&packet, NET_CONNECTION_CHANNEL_FILE_TRANSFER, true);
			}
			else
			{
				NetPacket_C2S_FileTransferChunk packet;
				packet.slot_hash = m_slot_hash;
				packet.data.Reserve(this_chunk_size);

				memcpy(packet.data.Buffer(), m_compressed_data.Buffer() + m_data_offset, this_chunk_size);
				m_data_offset += this_chunk_size;

				m_connection->Send(&packet, NET_CONNECTION_CHANNEL_FILE_TRANSFER, true);
			}

			m_chunk_timer = time;
		}
	}
}