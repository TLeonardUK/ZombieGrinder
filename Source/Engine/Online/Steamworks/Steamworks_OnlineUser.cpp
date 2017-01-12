// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Online/Steamworks/Steamworks_OnlineUser.h"
#include "Engine/Online/OnlinePlatform.h"

#include "Engine/Audio/AudioRenderer.h"
#include "Engine/Audio/Sounds/Sound.h"

#include "Engine/Network/NetManager.h"
#include "Engine/Network/NetClient.h"

#include "Engine/Platform/Platform.h"

#include "Engine/Engine/EngineOptions.h"

#include "Generic/Helper/StringHelper.h"

Steamworks_OnlineUser::Steamworks_OnlineUser(CSteamID id, int local_index, bool is_local)
	: m_steamid(id)
	, m_local_player_index(local_index)
	, m_is_local(is_local)
	, m_got_geoip_result(false)
	, m_guest_count(0)
	, m_pending_avatar_download(false)
	, m_avatar_texture(NULL)
	, m_pending_avatar_image_id(0)
	, m_pending_avatar_data(NULL)
	, m_avatar_Pixelmap(NULL)
	, m_last_voice_data_recieve_time(0.0f)
	, m_voice_channel(NULL)
	, m_voice_data_queue(max_voice_data_buffer, voice_buffer_artificial_latency)
	, m_voice_sound(NULL)
	, m_input_source(InputSourceDevice::NONE, -1)
	, m_profile_index(-1)
#ifdef OPT_PREMIUM_ACCOUNTS
	, m_premium(false)
#endif
{
	m_geoip_result.CountryID = 0;
	m_geoip_result.CountryName = "--";
	m_geoip_result.CountryShortName = "N/A";

	m_voice_data_queue_mutex = Mutex::Create();

	Update_Persona();
}

Steamworks_OnlineUser::~Steamworks_OnlineUser()
{
	SAFE_DELETE(m_voice_data_queue_mutex);
	//SAFE_DELETE(m_avatar_Pixelmap); // Note: Don't dispose of Pixelmap, it belongs to texture now.
	SAFE_DELETE(m_avatar_texture);
	SAFE_DELETE(m_pending_avatar_data);
	SAFE_DELETE(m_voice_channel);
	SAFE_DELETE(m_voice_sound);
}

void Steamworks_OnlineUser::Set_SteamID(CSteamID id)
{
	m_steamid = id;
	Update_Persona();
}

void Steamworks_OnlineUser::Set_Is_Local(bool val)
{
	m_is_local = val;
}

void Steamworks_OnlineUser::Tick(const FrameTime& time)
{
	// Only get information on client.
	if (OnlinePlatform::Get()->Client() == NULL)
	{
		return;
	}

	// If we haven't spoken for a while, delete the sound.
	if (m_voice_sound != NULL && (Platform::Get()->Get_Ticks() - m_last_voice_data_recieve_time) > talking_sound_dispose_interval)
	{
		SAFE_DELETE(m_voice_channel);
		SAFE_DELETE(m_voice_sound);
	}

	// Update the GeoIP result.
	if (m_got_geoip_result == false && m_steamid.BIndividualAccount() && GeoIPManager::Try_Get() != NULL)
	{
		if (m_is_local == true)
		{
			// Grab based on local IP.
			m_geoip_result = GeoIPManager::Get()->Lookup_Short_Country(SteamUtils()->GetIPCountry());
			m_got_geoip_result = true;
		}
		else
		{
			// We need to wait for lobby data.
		}

		if (m_got_geoip_result == true)
		{
			DBG_LOG("[Steamworks User] %s was determined to be from '%s'.", m_username.c_str(), m_geoip_result.CountryName);		
		}
	}

	// Update the download of the users avatar.
	if (m_avatar_texture == NULL && m_steamid.BIndividualAccount() && Renderer::Try_Get() != NULL)
	{
		if (m_pending_avatar_download == false)
		{
			m_pending_avatar_image_id = SteamFriends()->GetMediumFriendAvatar(m_steamid);
			m_pending_avatar_download = true;
		}
		else
		{
			if (m_pending_avatar_image_id > 0)
			{
				uint32 width, height;

				if (SteamUtils()->GetImageSize(m_pending_avatar_image_id, &width, &height))
				{
					int buffer_size = 4 * width * height * sizeof(unsigned char);

					if (m_pending_avatar_data == NULL)
					{
						m_pending_avatar_data = new unsigned char[buffer_size];
					}

					if (SteamUtils()->GetImageRGBA(m_pending_avatar_image_id, (uint8*)m_pending_avatar_data, buffer_size))
					{
						m_avatar_Pixelmap = new Pixelmap(m_pending_avatar_data, (int)width, (int)height, (int)width, PixelmapFormat::R8G8B8A8, false);
						m_avatar_Pixelmap->Flip_Vertical();
						m_avatar_texture = Renderer::Get()->Create_Texture(m_avatar_Pixelmap, TextureFlags::NONE);
						
						DBG_LOG("[Steamworks User] Downloaded avatar for '%s'.", m_username.c_str());		
					}
				}
			}
		}
	}
}

void Steamworks_OnlineUser::Update_Persona()
{
	// Only get username on client.
	if (OnlinePlatform::Get()->Client() == NULL)
	{
		return;
	}
	if (NetManager::Try_Get() == NULL)
	{
		return;
	}

	std::string old_username = m_username;

	m_username = SteamFriends()->GetFriendPersonaName(m_steamid);
	m_is_guest = (m_is_local == true && m_local_player_index > 0);

	// If a guest then append their index to the end.
	if (m_is_local == true && m_local_player_index > 0)
	{
		m_username = StringHelper::Format("%s (%i)", m_username.c_str(), m_local_player_index);
	}

	// Inform the server than our name has changed!
	if (old_username != m_username)
	{
		NetManager::Get()->Client()->On_Name_Changed(this, old_username, m_username);
	}
}

u64 Steamworks_OnlineUser::Get_User_ID()
{
	return m_steamid.ConvertToUint64();
}

CSteamID Steamworks_OnlineUser::Get_SteamID()
{
	return m_steamid;
}

std::string Steamworks_OnlineUser::Get_Username()
{
	return m_username;
}

#ifdef OPT_PREMIUM_ACCOUNTS
bool Steamworks_OnlineUser::Get_Premium()
{
	return m_premium;
}

void Steamworks_OnlineUser::Set_Premium(bool value)
{
	m_premium = value;
}
#endif

bool Steamworks_OnlineUser::Is_Developer()
{
	u64 ids[] = {
		76561197977204045ULL ,	// Tim
		76561197998740683ULL 	// Jordan
	};

	u64 real = m_steamid.ConvertToUint64();
	bool is_dev = false;

	for (int i = 0; i < ARRAY_LENGTH(ids); i++)
	{
		if (ids[i] == real)
		{
			is_dev = true;
			break;
		}
	}

	return is_dev;
}

void Steamworks_OnlineUser::Set_Local_Player_Index(int index)
{
	m_local_player_index = index;

	Update_Persona();
}

int Steamworks_OnlineUser::Get_Local_Player_Index()
{
	return m_local_player_index;
}

void Steamworks_OnlineUser::Set_Profile_Index(int index)
{
	m_profile_index = index;
}

int Steamworks_OnlineUser::Get_Profile_Index()
{
	return m_profile_index;
}

InputSource Steamworks_OnlineUser::Get_Input_Source()
{
	return m_input_source;
}

void Steamworks_OnlineUser::Set_Input_Source(InputSource source)
{
	m_input_source = source;
}

bool Steamworks_OnlineUser::Is_Local()
{
	return m_is_local;
}

bool Steamworks_OnlineUser::Is_Guest()
{
	return m_is_guest;
}

GeoIPResult Steamworks_OnlineUser::Get_GeoIP_Result()
{
	return m_geoip_result;
}

void Steamworks_OnlineUser::Set_Short_Country_Name(const char* name)
{
	m_geoip_result = GeoIPManager::Get()->Lookup_Short_Country(name);
	m_got_geoip_result = true;
	
	DBG_LOG("[Steamworks User] %s was determined to be from '%s'.", m_username.c_str(), m_geoip_result.CountryName);			
}

void Steamworks_OnlineUser::Set_Guest_Count(int count)
{
	m_guest_count = count;
}

int Steamworks_OnlineUser::Get_Guest_Count()
{
	return m_guest_count;
}	

Texture* Steamworks_OnlineUser::Get_Avatar_Texture()
{
	return m_avatar_texture;
}

void Steamworks_OnlineUser::Voice_Data_Callback(void* data, int size, void* meta_data)
{
	Steamworks_OnlineUser* user = static_cast<Steamworks_OnlineUser*>(meta_data);

	double time = Platform::Get()->Get_Ticks();
	int max_data = 0;

	{
		MutexLock lock(user->m_voice_data_queue_mutex);

		max_data = Min(user->m_voice_data_queue.Available(time), size);
		user->m_voice_data_queue.Read((char*)data, max_data);
	}

	// Zero out the rest of the buffer if we have no more voice data available.
	if (max_data < size)
	{
		if (max_data > 0)
		{
			DBG_LOG("Not enough voice data to fill playback buffer (asked for %i, have %i). Data starvation may cause audio artifacts.", size, max_data);
		}
		memset((char*)data + max_data, 0, size - max_data);
	}

	// We do this rather than in Add_Voice_Data so it syncs up with audio better.
	if (max_data > voice_active_threshold)
	{
		user->m_last_voice_data_recieve_time = time;
	}
}

void Steamworks_OnlineUser::Add_Voice_Data(const char* data, int data_size, int sample_rate, float output_volume)
{
	if (m_voice_sound == NULL)
	{
		m_voice_sound = AudioRenderer::Get()->Create_Streaming_Sound(sample_rate, 1, SoundFormat::PCM16, Voice_Data_Callback, this);
		m_voice_channel = m_voice_sound->Allocate_Channel();
		m_voice_sound->Play(m_voice_channel);
	}
	
	double time = Platform::Get()->Get_Ticks();

	m_voice_channel->Set_Volume(output_volume);

	// We should delay voice data here for a few seconds of latency to ensure uninterrupted throughput.
	{
		MutexLock lock(m_voice_data_queue_mutex);
		m_voice_data_queue.Write(data, data_size, time);
	}

	m_voice_data_sample_rate		= sample_rate;
	m_voice_data_volume				= (*EngineOptions::audio_voice_recv_volume) * output_volume;
}

bool Steamworks_OnlineUser::Is_Talking()
{
	double elapsed = (Platform::Get()->Get_Ticks() - m_last_voice_data_recieve_time);
	return (m_last_voice_data_recieve_time != 0.0f && elapsed < talking_indication_interval);
}

void Steamworks_OnlineUser::Mark_As_Talking()
{	
	double time = Platform::Get()->Get_Ticks();
	m_last_voice_data_recieve_time = time;
}
