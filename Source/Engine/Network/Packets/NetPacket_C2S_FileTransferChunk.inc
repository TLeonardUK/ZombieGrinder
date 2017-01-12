// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

START_PACKET(NetPacket_C2S_FileTransferChunk, false)
	PACKET_FIELD(int, slot_hash);
	PACKET_FIELD(DataBuffer, data);
END_PACKET()