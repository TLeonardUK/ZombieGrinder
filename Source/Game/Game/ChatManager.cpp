// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Game/ChatManager.h"

#include "Engine/Network/NetManager.h"
#include "Engine/Network/NetServer.h"
#include "Engine/Network/NetUser.h"

#include "Engine/Online/OnlineGlobalChat.h"

#include "Engine/Renderer/Text/MarkupFontRenderer.h"

#include "Game/Runner/Game.h"
#include "Game/Network/GameNetUser.h"
#include "Game/Runner/GameMode.h"

#include "Engine/Platform/Platform.h"

#include "Engine/Engine/GameEngine.h"
#include "Engine/UI/UIManager.h"
#include "Engine/Localise/Locale.h"

#include "Game/Runner/GameOptions.h"

ChatManager::ChatManager()
	: m_callback_server_chat_message_recieved(this, &ChatManager::Callback_Recieve_Server_Chat, false, NULL)
	, m_callback_client_chat_message_recieved(this, &ChatManager::Callback_Recieve_Client_Chat, false, NULL)
	, m_messages_recieved(0)
	, m_global_total_recieved(0)
{
	m_history_size = *GameOptions::network_chat_max_history;
}

void ChatManager::Serialize_Demo(BinaryStream* stream, DemoVersion::Type version, bool bSaving, float frameDelta)
{
	if (!bSaving)
	{
		m_history.clear();
	}

	unsigned int count = m_history.size();
	stream->Serialize<unsigned int>(bSaving, count);

	if (!bSaving)
	{
		m_history.resize(count);
	}

	for (unsigned int i = 0; i < count; i++)
	{
		ChatMessage& message = m_history[i];

		double elapsed = Platform::Get()->Get_Ticks() - message.Recieve_Time;
		stream->Serialize<double>(bSaving, elapsed);
		stream->Serialize<std::string>(bSaving, message.Packet.Message);
		stream->Serialize<char>(bSaving, reinterpret_cast<char&>(message.Packet.Type));

		unsigned short to_id = message.Packet.To_User_ID;
		unsigned short from_id = message.Packet.From_User_ID;

		if (bSaving)
		{
			to_id = DemoManager::Get()->Net_ID_To_Player_Index(to_id);
			from_id = DemoManager::Get()->Net_ID_To_Player_Index(from_id);
		}

		stream->Serialize<unsigned short>(bSaving, to_id);
		stream->Serialize<unsigned short>(bSaving, from_id);

		if (!bSaving)
		{
			message.Recieve_Time = Platform::Get()->Get_Ticks() - elapsed;
			message.Display_String = Chat_To_String(&message.Packet);
		}
	}

	if (!bSaving)
	{
		m_messages_recieved = count;
		m_global_total_recieved = count;
	}
}

void ChatManager::Tick(const FrameTime& time)
{
	// Pipe in chat messages.
	if (*GameOptions::pipe_global_chat && OnlineGlobalChat::Try_Get() != NULL)
	{
		std::vector<GlobalChatMessage> messages;
		int total_recieved = OnlineGlobalChat::Get()->Get_Messages(messages);

		if (m_global_total_recieved != total_recieved)
		{
			for (std::vector<GlobalChatMessage>::iterator iter = messages.begin(); iter != messages.end(); iter++)
			{
				GlobalChatMessage& msg = *iter;
				if (msg.Counter >= m_global_total_recieved)
				{
					if (msg.Type == GlobalChatMessageType::Chat_Message)
					{
						std::string message = MarkupFontRenderer::Escape(msg.Message);
						message = SF("#chat_message_game_global", msg.Source.c_str(), message.c_str());

						Store_Chat(message.c_str());
					}
				}
			}

			m_global_total_recieved = total_recieved;
		}
	}
}

void ChatManager::Parse(std::string message)
{
	if (message.size() > 1 && message.at(0) == '/')
	{
		bool parsed = false;

		std::vector<std::string> segments;
		StringHelper::Split(message.substr(1).c_str(), ' ', segments);

		if (segments.size() > 0)
		{
			std::string command = StringHelper::Lowercase(segments.at(0).c_str());
			segments.erase(segments.begin());

			// /pm destination message
			if (command == "pm" && segments.size() >= 2)
			{
				std::string desination = segments.at(0);
				std::string final_msg = StringHelper::Join(segments.begin() + 1, segments.end());
				NetUser* user = NetManager::Get()->Get_User_By_Username(desination);

				if (user != NULL)
				{
					Send_Client(ChatMessageType::Private, final_msg, user->Get_Net_ID());
				}
				else
				{
					NetPacket_C2S_ChatMessage packet;
					packet.Message		= S("#chat_message_invalid_destination");
					packet.Type			= ChatMessageType::Server;
					Store_Chat(&packet);
				}
				parsed = true;
			}

			// /action message
			else if (command == "action" && segments.size() >= 1)
			{
				std::string final_msg = StringHelper::Join(segments.begin(), segments.end());
				
				Send_Client(ChatMessageType::Action, final_msg);
				
				parsed = true;
			}

			// /global message
			else if (command == "global" && segments.size() >= 1)
			{
				std::string final_msg = StringHelper::Join(segments.begin(), segments.end());

				GlobalChatMessage msg;
				msg.Message = final_msg;
				msg.Source = "";
				msg.Type = GlobalChatMessageType::Chat_Message;

				OnlineGlobalChat::Get()->Send_Message(msg);

				parsed = true;
			}
		}

		if (parsed == false)
		{	
			NetPacket_C2S_ChatMessage packet;
			packet.Message		= S("#chat_message_invalid_command");
			packet.Type			= ChatMessageType::Server;
			Store_Chat(&packet);
		}
	}
	else
	{
		Send_Client(ChatMessageType::Global, message, -1);
	}
}

std::string ChatManager::User_To_String(NetUser* user, bool uncolored)
{
	std::string from_username = "unknown_user";

	if (user == NULL)
	{
		return from_username;
	}

	GameMode* mode = Game::Get()->Get_Game_Mode();
	GameNetUser* from_user = static_cast<GameNetUser*>(user);
	
	if (from_user != NULL)
	{
		Color team_color = Color(0, 0, 0, 0);

		GameModeTeam* team = mode->Get_Team_By_Index(from_user->Get_State()->Team_Index);
		if (team != NULL && uncolored == false)
		{
			team_color = team->Primary_Color;
		}

#ifdef OPT_PREMIUM_ACCOUNTS
		// Gold name in chat for premium users!
		if (from_user->Get_Online_User()->Get_Premium())
		{
			team_color = Color(255, 194, 14, 255);
		}
#endif
	
		if (!uncolored && team_color.A > 0)
		{
			from_username = StringHelper::Format("[c=%i,%i,%i]", team_color.R, team_color.G, team_color.B) + MarkupFontRenderer::Escape(from_user->Get_Username()) + "[/c]";
		}
		else
		{
			from_username = from_user->Get_Username();
		}
	}

	return from_username;
}

std::string ChatManager::ID_To_Username(int id)
{
	NetManager* manager = NetManager::Get();

	if (DemoManager::Get()->Is_Playing())
	{
		DemoPlayer player;
		if (DemoManager::Get()->Get_Player_By_Player_Index(id, player))
		{
			return StringHelper::Format("[c=230,0,0,255]%s[/c]", MarkupFontRenderer::Escape(player.Username).c_str());
		}
	}
	else
	{
		NetUser* from_user = manager->Get_User_By_Net_ID(id);
		return User_To_String(from_user);
	}

	return "Unknown";
}

std::string ChatManager::Chat_To_String(NetPacket_C2S_ChatMessage* packet)
{
	NetManager* manager = NetManager::Get();

	switch (packet->Type)
	{
	case ChatMessageType::Private:  
		{
			std::string from_username = ID_To_Username(packet->From_User_ID);
			std::string to_username = ID_To_Username(packet->To_User_ID);

			std::string message = MarkupFontRenderer::Escape(packet->Message);

			return SF("#chat_message_private", to_username.c_str(), from_username.c_str(), message.c_str());
		}
	case ChatMessageType::Global:   
		{
			std::string from_username = ID_To_Username(packet->From_User_ID);

			std::string message = MarkupFontRenderer::Escape(packet->Message);

			return SF("#chat_message_global", from_username.c_str(), message.c_str());
		}
	case ChatMessageType::Game_Global:   
		{
			std::string from_username = ID_To_Username(packet->From_User_ID);

			std::string message = MarkupFontRenderer::Escape(packet->Message);

			return SF("#chat_message_game_global", from_username.c_str(), message.c_str());
		}
	case ChatMessageType::Action:   
		{
			std::string from_username = ID_To_Username(packet->From_User_ID);

			std::string message = MarkupFontRenderer::Escape(packet->Message);

			return SF("#chat_message_action", from_username.c_str(), message.c_str());
		}
	case ChatMessageType::Server:	
		{
			std::string message = packet->Message;
			return SF("#chat_message_server", message.c_str());
		}
	default:
		{
			return packet->Message;
		}
	}
}
	
void ChatManager::Store_Chat(NetPacket_C2S_ChatMessage* packet)
{
	ChatMessage message;
	message.Packet			= *packet;
	message.Display_String	= Chat_To_String(packet);
	message.Recieve_Time	= Platform::Get()->Get_Ticks();

	m_history.push_back(message);
	while (m_history.size() > m_history_size)
	{
		m_history.erase(m_history.begin());
	}

	m_messages_recieved++;

	GameEngine::Get()->Get_UIManager()->Play_UI_Sound(UISoundType::Pop);
}

void ChatManager::Store_Chat(std::string chat)
{
	ChatMessage message;
	message.Display_String	= chat;
	message.Recieve_Time	= Platform::Get()->Get_Ticks();

	m_history.push_back(message);
	while (m_history.size() > m_history_size)
	{
		m_history.erase(m_history.begin());
	}

	m_messages_recieved++;

	GameEngine::Get()->Get_UIManager()->Play_UI_Sound(UISoundType::Pop);
}

void ChatManager::Send_Client(ChatMessageType::Type type, std::string message, int to_id)
{
	NetManager* manager = NetManager::Get();

	NetPacket_C2S_ChatMessage packet;
	packet.Message = message;
	packet.Type = type;
	packet.To_User_ID = to_id;
	packet.From_User_ID = manager->Get_Primary_Local_Net_User()->Get_Net_ID();

	manager->Client()->Get_Connection()->Send(&packet, NET_CONNECTION_CHANNEL_SYSTEM_RELIABLE, true);	
}

void ChatManager::Send_Server(ChatMessageType::Type type, std::string message, int to_id, int from_id)
{
	NetManager* manager = NetManager::Get();

	NetPacket_C2S_ChatMessage packet;
	packet.Message = message;
	packet.Type = type;
	packet.To_User_ID = to_id;
	packet.From_User_ID = from_id;

	if (to_id == -1)
	{
		manager->Server()->Get_Connection()->Broadcast(&packet, NET_CONNECTION_CHANNEL_SYSTEM_RELIABLE, true);	
	}
	else
	{
		NetUser* to_user = manager->Get_User_By_Net_ID(to_id);
		if (to_user != NULL)
		{
			to_user->Get_Connection()->Broadcast(&packet, NET_CONNECTION_CHANNEL_SYSTEM_RELIABLE, true);	
		}
	}

	if (NetManager::Get()->Is_Dedicated_Server())
	{
		Store_Chat(&packet);
	}
}

bool ChatManager::Callback_Recieve_Server_Chat(NetPacket_C2S_ChatMessage* packet)
{
	NetManager* manager = NetManager::Get();

	NetPacket_C2S_ChatMessage output = *packet;

	switch (output.Type)
	{
	case ChatMessageType::Private:
	case ChatMessageType::Global:
	case ChatMessageType::Action:
	case ChatMessageType::Server:
		break;
	default:
		DBG_LOG("Invalid chat type recieved, possible hacking attempt?");
		return true;
	}

	// TODO: Validate the from net-id of this chat.

	// Broadcast message to all users.
	if (output.Type == ChatMessageType::Private)
	{
		NetUser* from_user = manager->Get_User_By_Net_ID(output.From_User_ID);
		NetUser* to_user = manager->Get_User_By_Net_ID(output.To_User_ID);

		if (from_user != NULL)
			from_user->Get_Connection()->Send(&output, NET_CONNECTION_CHANNEL_SYSTEM_RELIABLE, true);
		if (to_user != NULL)
			to_user->Get_Connection()->Send(&output, NET_CONNECTION_CHANNEL_SYSTEM_RELIABLE, true);
	}
	else
	{
		manager->Server()->Get_Connection()->Broadcast(&output, NET_CONNECTION_CHANNEL_SYSTEM_RELIABLE, true);	
	}

	if (NetManager::Get()->Is_Dedicated_Server())
	{
		Store_Chat(packet);
	}

	return true;
}

bool ChatManager::Callback_Recieve_Client_Chat(NetPacket_C2S_ChatMessage* packet)
{
	std::string result = Chat_To_String(packet);
	std::string type = "UNKNOWN";

	switch (packet->Type)
	{
	case ChatMessageType::Private:  type = "PRIVATE"; break;
	case ChatMessageType::Global:   type = "GLOBAL";  break;
	case ChatMessageType::Action:   type = "ACTION";  break;
	case ChatMessageType::Server:	type = "SERVER";  break;
	}

	DBG_LOG("[CHAT] [FROM=%i,TO=%i] [%s] %s", packet->From_User_ID, packet->To_User_ID, type.c_str(), result.c_str()); 

	Store_Chat(packet);

	return true;
}

std::vector<ChatMessage>& ChatManager::Get_History()
{
	return m_history;
}

int ChatManager::Get_Messages_Recieved()
{
	return m_messages_recieved;
}