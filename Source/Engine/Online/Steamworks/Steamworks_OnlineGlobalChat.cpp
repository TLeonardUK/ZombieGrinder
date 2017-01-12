// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Online/Steamworks/Steamworks_OnlinePlatform.h"
#include "Engine/Online/Steamworks/Steamworks_OnlineMatching.h"
#include "Engine/Online/Steamworks/Steamworks_OnlineUser.h"
#include "Engine/Online/Steamworks/Steamworks_OnlineClient.h"
#include "Engine/Online/Steamworks/Steamworks_OnlineGlobalChat.h"

#include "Engine/Network/NetConnection.h"

#include "Engine/Platform/Platform.h"

#include "Engine/Input/Input.h"

#include "Engine/Localise/Locale.h"

#include "Engine/Network/NetManager.h"
#include "Engine/Network/NetClient.h"
#include "Engine/Network/NetServer.h"
#include "Engine/Network/Packets/EnginePackets.h"

#include "Engine/Engine/EngineOptions.h"

#include "Engine/Version.h"

Steamworks_OnlineGlobalChat::Steamworks_OnlineGlobalChat()
	: m_on_user_left_callback(this, &Steamworks_OnlineGlobalChat::Callback_On_User_Left)
	, m_on_recieved_message_callback(this, &Steamworks_OnlineGlobalChat::Callback_On_Recieved_Message)
	, m_state(Steamworks_OnlineGlobalChatState::Offline)
	, m_message_recieve_count(0)
{
} 

Steamworks_OnlineGlobalChat::~Steamworks_OnlineGlobalChat()
{
}

bool Steamworks_OnlineGlobalChat::Initialize()
{
	DBG_LOG("Setting up global chat.");
	return true;
}

void Steamworks_OnlineGlobalChat::Tick(const FrameTime& time)
{
	switch (m_state)
	{
	case Steamworks_OnlineGlobalChatState::Offline:
		{
			if (SteamUser()->BLoggedOn() && SteamFriends()->GetPersonaState() != k_EPersonaStateOffline)
			{
				uint64 val = (*EngineOptions::global_chat_group_id);
				m_lobby_id.SetFromUint64(val);
	
				DBG_LOG("[Global Chat] Attempted to join chat room '%llu'.", val);

				m_join_request = SteamFriends()->JoinClanChatRoom(m_lobby_id);
				m_join_request_result.Set(m_join_request, this, &Steamworks_OnlineGlobalChat::On_Join_Finished);

				m_state = Steamworks_OnlineGlobalChatState::Connecting;
			}
			break;
		}
	case Steamworks_OnlineGlobalChatState::Connecting:
		{
			// Finish callback changes this.
			break;
		}
	case Steamworks_OnlineGlobalChatState::Online:
		{
			// Check if we have gone offline?
			break;
		}
	case Steamworks_OnlineGlobalChatState::Banned:
		{
			// Don't try to join chat again, we were unable to the first time round!
			break;
		}
	}
}

void Steamworks_OnlineGlobalChat::On_Join_Finished(JoinClanChatRoomCompletionResult_t* params, bool bIOFailure)	
{
	// Went offline?
	if (bIOFailure)
	{
		DBG_LOG("[Global Chat] Failed to join chat room, IO failure.");
		m_state = Steamworks_OnlineGlobalChatState::Offline;
	}
	else
	{
		if (params->m_eChatRoomEnterResponse == k_EChatRoomEnterResponseSuccess)
		{
			DBG_LOG("[Global Chat] Successfully joined chat room.");
			m_state = Steamworks_OnlineGlobalChatState::Online;

		}
		else
		{
			DBG_LOG("[Global Chat] Failed to join chat room due to error '%i'.", params->m_eChatRoomEnterResponse);
			m_state = Steamworks_OnlineGlobalChatState::Banned;
		}
	}
}

void Steamworks_OnlineGlobalChat::Callback_On_User_Left(GameConnectedChatLeave_t* params)
{
	if (params->m_steamIDUser == SteamUser()->GetSteamID() && 
		params->m_steamIDClanChat == m_lobby_id)
	{
		if (params->m_bDropped)
		{
			DBG_LOG("[Global Chat] Left check due to connection drop.");
			m_state = Steamworks_OnlineGlobalChatState::Offline;

			GlobalChatMessage message;
			message.Source = S("#global_chat_server");
			message.Message = S("#global_chat_lost_connection");
			message.Type = GlobalChatMessageType::Chat_Message;
			Store_Message(message);
		}
		else if (params->m_bKicked)
		{
			DBG_LOG("[Global Chat] Left check due to admin kick.");
			m_state = Steamworks_OnlineGlobalChatState::Banned;

			GlobalChatMessage message;
			message.Source = S("#global_chat_server");
			message.Message = S("#global_chat_kicked");
			message.Type = GlobalChatMessageType::Chat_Message;
			Store_Message(message);
		}
		else
		{
			DBG_LOG("[Global Chat] Left lobby due to unknown reason.");
			m_state = Steamworks_OnlineGlobalChatState::Offline;

			GlobalChatMessage message;
			message.Source = S("#global_chat_server");
			message.Message = S("#global_chat_lost_connection");
			message.Type = GlobalChatMessageType::Chat_Message;
			Store_Message(message);
		}
	}
}

void Steamworks_OnlineGlobalChat::Callback_On_Recieved_Message(GameConnectedClanChatMsg_t* params)
{
	if (params->m_steamIDClanChat == m_lobby_id)
	{
		CSteamID msg_source;
		char msg_text[2048];
		EChatEntryType msg_type;

		int ret = SteamFriends()->GetClanChatMessage(params->m_steamIDClanChat, params->m_iMessageID, msg_text, 2048, &msg_type, &msg_source);
		if (msg_type == k_EChatEntryTypeChatMsg)
		{
			DBG_LOG("[Global Chat] Recieved Message: ret=%i id=%i type=%i source='%s' text='%s'", ret, params->m_iMessageID, msg_type, SteamFriends()->GetFriendPersonaName(msg_source), msg_text);

			GlobalChatMessage message;
			message.Source = SteamFriends()->GetFriendPersonaName(msg_source);
			message.Message = msg_text;
			message.Type = GlobalChatMessageType::Chat_Message;
			Store_Message(message);
		}
	}
}

int Steamworks_OnlineGlobalChat::Get_User_Count()
{
	if (m_state == Steamworks_OnlineGlobalChatState::Online)
	{
		return SteamFriends()->GetClanChatMemberCount(m_lobby_id);
	}
	return 0;
}

int Steamworks_OnlineGlobalChat::Get_Messages(std::vector<GlobalChatMessage>& messages)
{
	messages = m_messages;
	return m_message_recieve_count;
}

void Steamworks_OnlineGlobalChat::Send_Message(GlobalChatMessage Message)
{
	if (m_state == Steamworks_OnlineGlobalChatState::Online)
	{
		int ret = SteamFriends()->SendClanChatMessage(m_lobby_id, Message.Message.c_str());
		DBG_LOG("[Global Chat] Sent Message: ret=%i type=%i message='%s'", ret, Message.Type, Message.Message.c_str());
	}
	else if (m_state == Steamworks_OnlineGlobalChatState::Banned)
	{
		DBG_LOG("[Global Chat] Attempte to send message, but not allowed access.");

		GlobalChatMessage message;
		message.Source = S("#global_chat_server");
		message.Message = S("#global_chat_not_access");
		message.Type = GlobalChatMessageType::Chat_Message;
		Store_Message(message);
	}
	else 
	{
		DBG_LOG("[Global Chat] Attempte to send message, but not connected.");

		GlobalChatMessage message;
		message.Source = S("#global_chat_server");
		message.Message = S("#global_chat_not_connected");
		message.Type = GlobalChatMessageType::Chat_Message;
		Store_Message(message);
	}
}

void Steamworks_OnlineGlobalChat::Store_Message(GlobalChatMessage Message)
{
	if (m_messages.size() == max_messages)
	{
		m_messages.erase(m_messages.begin());
	}
	Message.Counter = m_message_recieve_count;
	m_messages.push_back(Message);
	m_message_recieve_count++;
}