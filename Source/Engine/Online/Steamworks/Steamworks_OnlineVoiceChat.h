// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_STEAMWORKS_ONLINE_VOICE_CHAT_
#define _ENGINE_STEAMWORKS_ONLINE_VOICE_CHAT_

#include "Engine/Online/OnlineVoiceChat.h"
#include "Engine/Online/Steamworks/Steamworks_OnlinePlatform.h"

#include "Engine/Network/Packets/EnginePackets.h"
#include "Engine/Network/Packets/PacketDispatcher.h"
#include "Engine/Network/NetClient.h"

class Steamworks_OnlineVoiceChat: public OnlineVoiceChat
{
	MEMORY_ALLOCATOR(Steamworks_OnlineVoiceChat, "Network");

private:
	int m_voice_data_index;
	bool m_voice_chat_recording;
	char* m_voice_buffer;
	int m_voice_buffer_length;
	double m_record_falloff;
	bool m_disabled;

	enum
	{
		record_falloff_time = 1500
	};

protected:
	
	// Packet callbacks.	
	PACKET_RECIEVE_CALLBACK(Steamworks_OnlineVoiceChat,	Callback_RecievedVoiceData,		NetPacket_C2S_VoiceData,	m_callback_recieve_voice_data);

public:
	Steamworks_OnlineVoiceChat();
	~Steamworks_OnlineVoiceChat();
	
	bool Initialize();

	void Set_Disabled(bool bDisabled);
	bool Get_Disabled();

	void Tick(const FrameTime& time);
	void Recieve_Voice_Data(OnlineUser* user, const char* data, int data_size, float output_volume);

};

#endif

