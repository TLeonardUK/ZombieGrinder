// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_CHAT_MANAGER_
#define _GAME_CHAT_MANAGER_

#include "Engine/Network/NetClient.h"

#include "Engine/Demo/DemoManager.h"

#include "Game/Network/Packets/GamePackets.h"

struct ChatMessage
{
public:
	NetPacket_C2S_ChatMessage Packet;
	std::string Display_String;
	double Recieve_Time;
};

class ChatManager : public Singleton<ChatManager>
{
	MEMORY_ALLOCATOR(ChatManager, "Game");

private:
	unsigned int m_history_size;
	std::vector<ChatMessage> m_history;
	int m_messages_recieved;
	int m_global_total_recieved;

private:
	PACKET_RECIEVE_SERVER_CALLBACK(ChatManager, Callback_Recieve_Server_Chat, NetPacket_C2S_ChatMessage, m_callback_server_chat_message_recieved);
	PACKET_RECIEVE_CLIENT_CALLBACK(ChatManager, Callback_Recieve_Client_Chat, NetPacket_C2S_ChatMessage, m_callback_client_chat_message_recieved);

	void Store_Chat(NetPacket_C2S_ChatMessage* packet);
	void Store_Chat(std::string chat);

public:
	ChatManager();

	void Serialize_Demo(BinaryStream* stream, DemoVersion::Type version, bool bSaving, float frameDelta);

	std::string ID_To_Username(int id);

	void Tick(const FrameTime& time);

	std::string Chat_To_String(NetPacket_C2S_ChatMessage* packet);
	std::string User_To_String(NetUser* user, bool uncolored = false);

	std::vector<ChatMessage>& Get_History();
	int Get_Messages_Recieved();
	
	void Parse(std::string message);

	void Send_Client(ChatMessageType::Type type, std::string message, int to_id = -1);
	void Send_Server(ChatMessageType::Type type, std::string message, int to_id = -1, int from_id = -1);

};

#endif

