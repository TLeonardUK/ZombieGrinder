// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Online/Steamworks/Steamworks_OnlinePlatform.h"
#include "Engine/Online/Steamworks/Steamworks_OnlineMatching.h"
#include "Engine/Online/Steamworks/Steamworks_OnlineUser.h"
#include "Engine/Online/Steamworks/Steamworks_OnlineClient.h"
#include "Engine/Online/Steamworks/Steamworks_OnlineVoiceChat.h"

#include "Engine/Network/NetConnection.h"

#include "Engine/Platform/Platform.h"

#include "Engine/Input/Input.h"

#include "Engine/Network/NetManager.h"
#include "Engine/Network/NetClient.h"
#include "Engine/Network/NetServer.h"
#include "Engine/Network/Packets/EnginePackets.h"

#include "Engine/Engine/EngineOptions.h"

#include "Engine/Version.h"

Steamworks_OnlineVoiceChat::Steamworks_OnlineVoiceChat()
	: m_voice_chat_recording(false)
	, m_voice_data_index(0)
	, m_callback_recieve_voice_data(this, &Steamworks_OnlineVoiceChat::Callback_RecievedVoiceData, false, NULL)
	, m_record_falloff(0.0f)
	, m_disabled(false)
{
	m_voice_buffer_length = 800;
	m_voice_buffer = new char[m_voice_buffer_length];
} 

Steamworks_OnlineVoiceChat::~Steamworks_OnlineVoiceChat()
{
	SAFE_DELETE(m_voice_buffer);
}

bool Steamworks_OnlineVoiceChat::Initialize()
{
	DBG_LOG("Setting up voice chat.");

	return true;
}

bool Steamworks_OnlineVoiceChat::Callback_RecievedVoiceData(NetPacket_C2S_VoiceData* packet)
{
	NetServer* net_server = NetManager::Get()->Server();
	NetClient* net_client = NetManager::Get()->Client();
	NetManager* net_manager = NetManager::Get();

	std::vector<NetConnection*> connections = net_manager->Get_Connections();

	// If server rebroadcast to users.
	if (net_server != NULL)
	{
		for (std::vector<NetConnection*>::iterator iter = connections.begin(); iter != connections.end(); iter++)
		{
			NetConnection* connection = *iter;
			if (connection->Is_Child() &&
				connection->Get_Online_User() && 
				!connection->Get_Online_User()->Is_Local() &&
				connection->Get_Online_User()->Get_Local_Player_Index() == 0)
			{
				connection->Send(packet, NET_CONNECTION_CHANNEL_VOICE_UNRELIABLE, false);
			}
		}
	}
	
	// If client then playback.
	if (net_client != NULL)
	{
		OnlineUser* user = Steamworks_OnlinePlatform::Get()->Get_User_By_SteamID(CSteamID(packet->User_ID));
		if (user != NULL)
		{
			Recieve_Voice_Data(user, packet->Data.Buffer(), packet->Data.Size(), packet->Output_Volume);
		}
	}

	return true;
}

void Steamworks_OnlineVoiceChat::Set_Disabled(bool bDisabled)
{
	m_disabled = bDisabled;
}

bool Steamworks_OnlineVoiceChat::Get_Disabled()
{
	return m_disabled;
}

void Steamworks_OnlineVoiceChat::Tick(const FrameTime& time)
{
	Input* input			= Input::Get();
	NetClient* net_client	= NetManager::Get()->Client();
	float output_volume		= *EngineOptions::audio_voice_send_volume;

	bool should_record = true;
	
	// In client?
	if (net_client == NULL)
	{
		return;
	}

	// Not in game?
	else if (OnlineMatching::Get()->Get_State() != OnlineMatching_State::In_Lobby)
	{
		should_record = false;
	}

	// Voice chat disabled?
	else if (*EngineOptions::audio_voice_enabled == false)
	{
		should_record = false;
	}

	// Push to talk not active?
	else if (*EngineOptions::audio_push_to_talk_enabled == true && !input->Is_Down(OutputBindings::PushToTalk))
	{
		should_record = false;
	}

	// Enable temp disabling.
	else if (m_disabled)
	{
		should_record = false;
	}

	// Lets start recordin'
	if (should_record == true)
	{
		if (m_voice_chat_recording == false)
		{
			DBG_LOG("Started recording users voice.");
			SteamUser()->StartVoiceRecording();
			m_voice_chat_recording = true;

			m_record_falloff = 0.0f;
		}
	}
	else
	{	
		if (m_voice_chat_recording == true)
		{
			DBG_LOG("Stopped recording users voice.");
			SteamUser()->StopVoiceRecording();
			m_voice_chat_recording = false;

			m_record_falloff = Platform::Get()->Get_Ticks() + record_falloff_time;
		}
	}

	if (m_voice_chat_recording || m_record_falloff > 0.0f)
	{
		if (Platform::Get()->Get_Ticks() >= m_record_falloff)
		{
			m_record_falloff = 0.0f;
		}

		// Grab recorded data.
		uint32 compressed;
		uint32 uncompressed;

		int sample_rate = SteamUser()->GetVoiceOptimalSampleRate();

		int retval = SteamUser()->GetAvailableVoice(&compressed, &uncompressed, sample_rate);
		if (retval == k_EVoiceResultOK)
		{
			if (SteamUser()->GetVoice(true, m_voice_buffer, m_voice_buffer_length, &compressed, false, NULL, NULL, NULL, 0) == k_EVoiceResultOK)
			{
				NetPacket_C2S_VoiceData packet;
				packet.User_ID			= SteamUser()->GetSteamID().ConvertToUint64(); 
				packet.Output_Volume	= output_volume;
				packet.Data.Set(m_voice_buffer, compressed);

				if (compressed >= Steamworks_OnlineUser::voice_active_threshold)
				{
					OnlineUser* user = OnlinePlatform::Get()->Get_Initial_User();
					user->Mark_As_Talking();
				}

				// Send to server?
				if (net_client->Get_State() == NetClient_State::Connected)
				{
					NetManager::Get()->Client()->Get_Connection()->Send(&packet, NET_CONNECTION_CHANNEL_VOICE_UNRELIABLE, false);
				}

				// Send through lobby?
				else
				{
					OnlineMatching::Get()->Send_Lobby_Packet(&packet);
				}
			}
		}
	}
}

void Steamworks_OnlineVoiceChat::Recieve_Voice_Data(OnlineUser* user, const char* data, int data_size, float output_volume) 
{
	// We don't care about our own voice data.
	if (user->Is_Local())
	{
		return;
	}

	// Decompress data.
	static const int max_size = 1024 * 16;
	static char uncompressed[max_size]; // We're making these static to avoid blowing the stack.
	static uint32 uncompressed_size = 0;

	int sample_rate = SteamUser()->GetVoiceOptimalSampleRate();

	EVoiceResult result = SteamUser()->DecompressVoice(data, data_size, uncompressed, max_size, &uncompressed_size, sample_rate);
	if (result == k_EVoiceResultOK)
	{
		user->Add_Voice_Data(uncompressed, uncompressed_size, sample_rate, output_volume);
	}
	else
	{
		DBG_LOG("Failed to decompress voice data with error code %i.", result);
	}
}
