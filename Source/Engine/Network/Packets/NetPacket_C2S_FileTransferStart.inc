// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

START_PACKET(NetPacket_C2S_FileTransferStart, false)
	PACKET_FIELD(int, slot_hash);
END_PACKET()