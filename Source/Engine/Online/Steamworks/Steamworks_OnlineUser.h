// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_STEAMWORKS_ONLINE_USER_
#define _ENGINE_STEAMWORKS_ONLINE_USER_

#include "Generic/Patterns/Singleton.h"

#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/Textures/Texture.h"
#include "Engine/Renderer/Textures/Pixelmap.h"

#include "Engine/Online/OnlineUser.h"

#include "Engine/GeoIP/GeoIPManager.h"

#include "Generic/Threads/Mutex.h"
#include "Generic/Threads/MutexLock.h"
#include "Generic/Types/ByteStack.h"
#include "Generic/Types/TemporalBufferedByteStack.h"

#include "public/steam/steam_api.h"

#include <string>
#include <vector>

class Steamworks_OnlinePlatform;
class Sound;
class SoundChannel;

class Steamworks_OnlineUser : public OnlineUser
{
	MEMORY_ALLOCATOR(Steamworks_OnlineUser, "Network");

private:
	CSteamID		m_steamid;
	std::string		m_username;

	int				m_local_player_index;
	InputSource		m_input_source;
	bool			m_is_local;
	bool			m_is_guest;
	int				m_profile_index;

	GeoIPResult		m_geoip_result;
	bool			m_got_geoip_result;

	int				m_guest_count;

#ifdef OPT_PREMIUM_ACCOUNTS
	bool			m_premium;
#endif

	bool			m_pending_avatar_download;
	int				m_pending_avatar_image_id;
	unsigned char*	m_pending_avatar_data;
	Pixelmap*			m_avatar_Pixelmap;
	Texture*		m_avatar_texture;

	int				m_voice_data_sample_rate;
	float			m_voice_data_volume;
	double			m_last_voice_data_recieve_time;

	Mutex*							m_voice_data_queue_mutex;
	TemporalBufferedByteStack		m_voice_data_queue;

	bool			m_adding_voice_data;

	Sound*			m_voice_sound;
	SoundChannel*	m_voice_channel;

public:
	enum
	{
		talking_indication_interval			= 500,			// For how long after recieving a voice data packet we are considered to be talking.
		talking_sound_dispose_interval		= 1000 * 60,	// How long of no speaking until we dispose of the sound.
		max_voice_data_buffer				= 32 * 1024,	// How large the data buffer that contains voice data is, if this is to small and we don't consume fast enough we will end up dropping voice data.
		voice_buffer_artificial_latency		= 0,
		voice_active_threshold				= 50
	};

protected:
	friend class Steamworks_OnlinePlatform;
	friend class Steamworks_OnlineClient;
	friend class Steamworks_OnlineServer;

	void Update_Persona();

	static void Voice_Data_Callback(void* data, int size, void* meta_data);

public:
	Steamworks_OnlineUser(CSteamID id, int input_index, bool is_local);
	~Steamworks_OnlineUser();

	void Set_SteamID(CSteamID id);
	void Set_Is_Local(bool val);
	
	u64 Get_User_ID();

	void Tick(const FrameTime& time);

	CSteamID Get_SteamID();
	std::string Get_Username();

#ifdef OPT_PREMIUM_ACCOUNTS
	bool Get_Premium();
	void Set_Premium(bool value);
#endif
	int Get_Local_Player_Index();	
	void Set_Local_Player_Index(int index);
	int Get_Profile_Index();	
	void Set_Profile_Index(int index);
	InputSource Get_Input_Source();
	void Set_Input_Source(InputSource source);
	bool Is_Local();
	bool Is_Guest();
	bool Is_Developer();

	GeoIPResult Get_GeoIP_Result();

	void Set_Short_Country_Name(const char* name);
	void Set_Guest_Count(int count);

	int Get_Guest_Count();
	
	Texture* Get_Avatar_Texture();
	
	void Add_Voice_Data(const char* data, int data_size, int sample_rate, float output_volume);
	bool Is_Talking();
	void Mark_As_Talking();

};

#endif

