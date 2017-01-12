// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

START_PACKET_ENUM(ChatMessageType)

	PACKET_ENUM_ELEMENT(Private)			// Private player->player messages.
	PACKET_ENUM_ELEMENT(Global)				// Global player->all other players messages.
	PACKET_ENUM_ELEMENT(Action)				// Global player->all other players messages. "* Derp did something"
	PACKET_ENUM_ELEMENT(Server)				// Server announcements "Derp has joined the game".
	PACKET_ENUM_ELEMENT(Game_Global)		// IRC/Clan Chat

END_PACKET_ENUM()

START_PACKET(NetPacket_C2S_ChatMessage, false)

	PACKET_FIELD(u16, From_User_ID);
	PACKET_FIELD(u16, To_User_ID);

	PACKET_ENUM_FIELD(ChatMessageType, Type);

	PACKET_FIELD(std::string, Message);

END_PACKET()