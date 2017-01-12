// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

START_PACKET_ENUM(ReplicationEventType)
	PACKET_ENUM_ELEMENT(Spawn)
	PACKET_ENUM_ELEMENT(Despawn)
	PACKET_ENUM_ELEMENT(RPC)
END_PACKET_ENUM()

START_PACKET(ReplicationEventSpawnData, false)
	PACKET_FIELD(int, Class_ID);
	PACKET_FIELD(int, Owner_ID);
END_PACKET()

START_PACKET(ReplicationEventRPCTarget, false)
	PACKET_FIELD(int, Index);
END_PACKET()

START_PACKET(ReplicationEventRPCData, false)
	PACKET_FIELD(int,								Symbol_Hash);
	PACKET_VECTOR_FIELD(ReplicationEventRPCTarget,	Targets);
	PACKET_FIELD(DataBuffer,						Parameters);
END_PACKET()

START_PACKET(ReplicationEvent, false)
	PACKET_FIELD(int, Unique_ID);
	PACKET_FIELD(double, Tick);
	PACKET_ENUM_FIELD(ReplicationEventType, Type);
	PACKET_FIELD(DataBuffer, Data);
END_PACKET()

// Always reliable.
START_PACKET(NetPacket_C2S_ReplicationEvents, false)	
	PACKET_FIELD		(bool,				Partial);
	PACKET_FIELD		(DataBuffer,		Compressed_Blob);
//	PACKET_VECTOR_FIELD	(ReplicationEvent,	Events);
END_PACKET()

// Can be dropped.
START_PACKET(NetPacket_C2S_ReplicationStates, false)	
	PACKET_FIELD		(double, Tick);
	PACKET_FIELD		(bool, Partial);
	PACKET_FIELD		(DataBuffer, Compressed_Header_Data);
	PACKET_FIELD		(DataBuffer, Compressed_State_Data);
END_PACKET()