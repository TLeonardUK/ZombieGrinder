// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef DEDICATED_SERVER_BUILD

#include "Engine/Audio/FMod/FMod_AudioRenderer.h"
#include "Engine/Audio/FMod/FMod_Sound.h"
#include "Engine/Engine/GameEngine.h"

#include "Engine/IO/StreamFactory.h"
#include "Engine/IO/Stream.h"

#include "Generic/Stats/Stats.h"

#include "Generic/Math/Math.h"

#include "Engine/Resources/ResourceFactory.h"

#include "Engine/Engine/EngineOptions.h"

//#include <windows.h>

//#ifndef MASTER_BUILD
//#define ENABLE_FMOD_PROFILER
//#define FMOD_AUDIO_LOGGING
//#endif

//#define ENABLE_FMOD_PROFILER
//#define FMOD_AUDIO_LOGGING

DEFINE_FRAME_STATISTIC("Audio/Active Channels", int, g_audio_active_channels, true);
DEFINE_FRAME_STATISTIC("Audio/CPU Usage", float, g_audio_cpu_usage, true);

FMOD_RESULT F_CALLBACK FMod_AudioRenderer::fmod_open(const char* name, unsigned int* filesize, void** handle, void* userdata)
{
	Stream* stream = StreamFactory::Open(name, StreamMode::Read);
	if (stream == NULL)
	{
		return FMOD_ERR_FILE_NOTFOUND;
	}

	*filesize = stream->Length(); 
	*handle   = stream;

	return FMOD_OK;
}

FMOD_RESULT F_CALLBACK FMod_AudioRenderer::fmod_close(void* handle, void* userdata)
{
	Stream* stream = static_cast<Stream*>(handle);
	if (stream == NULL)
	{
		return FMOD_ERR_INVALID_PARAM;
	}

	stream->Close();
	delete stream;

	return FMOD_OK;
}

FMOD_RESULT F_CALLBACK FMod_AudioRenderer::fmod_seek(void* handle, unsigned int pos, void* userdata)
{
	Stream* stream = static_cast<Stream*>(handle);
	if (stream == NULL)
	{
		return FMOD_ERR_INVALID_PARAM;
	}

	stream->Seek(pos);
	return FMOD_OK;
}

FMOD_RESULT F_CALLBACK FMod_AudioRenderer::fmod_read(void* handle, void* buffer, unsigned int sizebytes, unsigned int* bytesread, void* userdata)
{
	Stream* stream = static_cast<Stream*>(handle);
	if (stream == NULL)
	{
		return FMOD_ERR_INVALID_PARAM;
	}

	unsigned int can_read = Min(stream->Length() - stream->Position(), sizebytes);
	*bytesread = can_read;

	if (can_read > 0)
	{
		stream->ReadBuffer((char*)buffer, 0, can_read);
		return FMOD_OK;
	}

	return can_read < sizebytes ? FMOD_ERR_FILE_EOF : FMOD_OK;
}

void* F_CALLBACK FMod_AudioRenderer::fmod_alloc(unsigned int size, FMOD_MEMORY_TYPE type, const char *sourcestr)
{
	FMod_AudioRenderer* renderer = dynamic_cast<FMod_AudioRenderer*>(AudioRenderer::Get());
	return renderer->Get_Allocator()->Alloc(size);
}

void* F_CALLBACK FMod_AudioRenderer::fmod_realloc(void *ptr, unsigned int size, FMOD_MEMORY_TYPE type, const char *sourcestr)
{
	FMod_AudioRenderer* renderer = dynamic_cast<FMod_AudioRenderer*>(AudioRenderer::Get());
	return renderer->Get_Allocator()->Realloc(ptr, size);
}

void  F_CALLBACK FMod_AudioRenderer::fmod_free(void *ptr, FMOD_MEMORY_TYPE type, const char *sourcestr)
{
	FMod_AudioRenderer* renderer = dynamic_cast<FMod_AudioRenderer*>(AudioRenderer::Get());
	return renderer->Get_Allocator()->Free(ptr);
}

FMod_AudioRenderer::FMod_AudioRenderer()
{
	int max_channels = *EngineOptions::audio_max_channels;
	FMOD_RESULT result;

	m_old_position = Vector3(0, 0, 0);

#ifdef FMOD_AUDIO_LOGGING
	result = FMOD_Debug_Initialize(
		FMOD_DEBUG_LEVEL_ERROR|FMOD_DEBUG_LEVEL_WARNING|FMOD_DEBUG_LEVEL_LOG,
		FMOD_DEBUG_MODE_TTY,
		NULL,
		NULL
		);
	DBG_ASSERT_STR(result == FMOD_OK, "FMOD_Debug_SetLevel Failed - Result: 0x%08x", result);
#endif

	result = FMOD_Memory_Initialize(NULL, 0, fmod_alloc, fmod_realloc, fmod_free, FMOD_MEMORY_ALL); 
	//result = FMOD_Memory_Initialize(platform_malloc(1024 * 1024 * 100), 1024 * 1024 * 100, NULL, NULL, NULL, FMOD_MEMORY_ALL); 
	DBG_ASSERT_STR(result == FMOD_OK, "FMOD_Memory_Initialize Failed - Result: 0x%08x (%s)", result, FMOD_ErrorString(result));

	DBG_LOG("Creating fmod audio system ...");
	{
		result = FMOD_System_Create(&m_system);
		DBG_ASSERT_STR(result == FMOD_OK, "FMOD_System_Create Failed - Result: 0x%08x (%s)", result, FMOD_ErrorString(result));

		FMOD_ADVANCEDSETTINGS settings;
		memset(&settings, 0, sizeof(FMOD_ADVANCEDSETTINGS));
		settings.cbSize = sizeof(FMOD_ADVANCEDSETTINGS);
		settings.maxVorbisCodecs = max_channels;
		settings.defaultDecodeBufferSize = 1000;

		result = FMOD_System_SetAdvancedSettings(m_system, &settings);
		DBG_ASSERT_STR(result == FMOD_OK, "FMOD_System_SetAdvancedSettings Failed - Result: 0x%08x (%s)", result, FMOD_ErrorString(result));
		DBG_LOG("FMOD_System_SetAdvancedSettings was a success.");

		result = FMOD_System_SetSoftwareChannels(m_system, max_channels);
		DBG_ASSERT_STR(result == FMOD_OK, "FMOD_System_SetSoftwareChannels Failed - Result: 0x%08x (%s)", result, FMOD_ErrorString(result));
		DBG_LOG("FMOD_System_SetSoftwareChannels was a success.");

		result = FMOD_System_SetSoftwareFormat(m_system, 44000, FMOD_SPEAKERMODE_STEREO, 0);
		DBG_ASSERT_STR(result == FMOD_OK, "FMOD_System_SetSoftwareFormat Failed - Result: 0x%08x (%s)", result, FMOD_ErrorString(result));
	}
	
	DBG_LOG("Initializing fmod system ...");
	{
#ifdef PLATFORM_WIN32
		// WASAPI seems to have issues continually feeding data on windows, and causes stalls. We force the driver to something else to solve this
		// for the time being.
	//	result = FMOD_System_SetOutput(m_system, FMOD_OUTPUTTYPE_WINMM);
	//	DBG_ASSERT_STR(result == FMOD_OK, "FMOD_System_SetOutput Failed - Result: 0x%08x (%s)", result, FMOD_ErrorString(result));
	//	DBG_LOG("FMOD_System_SetOutput(FMOD_OUTPUTTYPE_ASIO) was a success.");
#endif

		DBG_LOG("Calling FMOD_System_Init (Max Channels=%i)", max_channels);
#ifdef ENABLE_FMOD_PROFILER
		result = FMOD_System_Init(m_system, max_channels, FMOD_INIT_PROFILE_ENABLE|FMOD_INIT_PROFILE_METER_ALL, NULL);
#else
		result = FMOD_System_Init(m_system, max_channels, FMOD_INIT_NORMAL, NULL);
#endif
		DBG_ASSERT_STR(result == FMOD_OK, "FMOD_System_Init Failed - Result: 0x%08x (%s)", result, FMOD_ErrorString(result));
		DBG_LOG("FMOD_System_Init was a success.");

		unsigned int version = 0;
		result = FMOD_System_GetVersion(m_system, &version);
		DBG_ASSERT_STR(result == FMOD_OK, "FMOD_System_GetVersion Failed - Result: 0x%08x (%s)", result, FMOD_ErrorString(result));
		
		int hardware_channels = 0;
		result = FMOD_System_GetSoftwareChannels(m_system, &hardware_channels);
		DBG_ASSERT_STR(result == FMOD_OK, "FMOD_System_GetSoftwareChannels Failed - Result: 0x%08x (%s)", result, FMOD_ErrorString(result));

		int driver_id = 0;
		result = FMOD_System_GetDriver(m_system, &driver_id);
		DBG_ASSERT_STR(result == FMOD_OK, "FMOD_System_GetDriver Failed - Result: 0x%08x (%s)", result, FMOD_ErrorString(result));

		char driver_name[1024];
		result = FMOD_System_GetDriverInfo(m_system, driver_id, driver_name, 1024, NULL, NULL, NULL, NULL);
		DBG_ASSERT_STR(result == FMOD_OK, "FMOD_System_GetDriverInfo Failed - Result: 0x%08x (%s)", result, FMOD_ErrorString(result));

		int sample_rate = 0;
		result = FMOD_System_GetSoftwareFormat(m_system, &sample_rate, NULL, NULL);
		DBG_ASSERT_STR(result == FMOD_OK, "FMOD_System_GetSoftwareFormat Failed - Result: 0x%08x (%s)", result, FMOD_ErrorString(result));

		DBG_LOG("  FMOD version       : %i.%i.%i (0x%08x)", (version >> 16) & 0xFFFF, (version >> 8) & 0xFF, version & 0xFF, version);
		DBG_LOG("  Built FMOD version : %i.%i.%i (0x%08x)", (FMOD_VERSION >> 16) & 0xFFFF, (FMOD_VERSION >> 8) & 0xFF, FMOD_VERSION & 0xFF, FMOD_VERSION);
		DBG_LOG("  Max channels       : %i", max_channels);
		DBG_LOG("  Software channels  : %i", hardware_channels);
		DBG_LOG("  Driver name        : %s", driver_name);
		DBG_LOG("  Sample rate        : %i", sample_rate);
	}

	DBG_LOG("Creating fmod file system ...");
	{
		result = FMOD_System_SetFileSystem(m_system, fmod_open, fmod_close, fmod_read, fmod_seek, NULL, NULL, 2048);
		DBG_ASSERT_STR(result == FMOD_OK, "FMOD_System_SetFileSystem Failed - Result: 0x%08x (%s)", result, FMOD_ErrorString(result));
	}

	// Set reverb properties to hallway (why, I don't know ...)
	FMOD_System_Set3DSettings(m_system, 1.0f, 1.0f, 1.0f);

	//FMOD_REVERB_PROPERTIES reverb = FMOD_PRESET_GENERIC;
	//FMOD_System_SetReverbProperties(m_system, 0, &reverb);
}

FMod_AudioRenderer::~FMod_AudioRenderer()
{
	if (m_system != NULL)
	{
		DBG_LOG("Releasing fmod system ...");
		{
			FMOD_RESULT result = FMOD_System_Release(m_system);
			DBG_ASSERT_STR(result == FMOD_OK, "Result: 0x%08x", result);

			m_system = NULL;
		}
	}
}

FMOD_SYSTEM* FMod_AudioRenderer::Get_FMod_System()
{
	return m_system;
}

struct fmod_unique_audio
{
	std::string name;
	int count;
	float position;
};

void FMod_AudioRenderer::Dump_Channels()
{
	DBG_LOG("======= FMOD Dump =======");


	std::vector<fmod_unique_audio> audio;

	for (int i = 0; i < 4096; i++)
	{
		FMOD_CHANNEL* channel = NULL;
		FMOD_System_GetChannel(m_system, i, &channel);

		if (channel != NULL)
		{
			FMOD_BOOL playing = false;
			FMOD_Channel_IsPlaying(channel, &playing);
			if (playing)
			{
				void* userdata = NULL;
				FMOD_Channel_GetUserData(channel, &userdata);

				if (userdata != NULL)
				{
					Sound* sound = reinterpret_cast<Sound*>(userdata);

					bool bFound = false;

					for (std::vector<fmod_unique_audio>::iterator iter = audio.begin(); iter != audio.end(); iter++)
					{
						fmod_unique_audio& a = *iter;
						if (a.name == sound->Get_Name())
						{
							a.count++;
							bFound = true;
							break;
						}
					}

					if (!bFound)
					{
						unsigned int pos = 0;
						FMOD_Channel_GetPosition(channel, &pos, FMOD_TIMEUNIT_MS);

						fmod_unique_audio a;
						a.name = sound->Get_Name();
						a.count = 1;
						a.position = (float)pos;
						audio.push_back(a);
					}
				}
			}
		}
	}

	for (std::vector<fmod_unique_audio>::iterator iter = audio.begin(); iter != audio.end(); iter++)
	{
		fmod_unique_audio& a = *iter;
		DBG_LOG("[%i] Name=%s Position=%f", a.count, a.name.c_str(), a.position);
	}
}
	
void FMod_AudioRenderer::Tick(const FrameTime& time)
{
	//double timer = Platform::Get()->Get_Ticks();
	FMOD_System_Update(m_system);
	//double elapsed = Platform::Get()->Get_Ticks() - timer;

	/*if (elapsed > 10.0f)
	{
		Dump_Channels();

		char buffer[2048];
		sprintf(buffer, "FMOD_System_Update Took %f\n", elapsed);
		OutputDebugStringA(buffer);
	}*/

	int channels = 0;
	float dsp_tile = 0.0f;
	float stream_time = 0.0f;
	float geometry_time = 0.0f; 
	float update_time = 0.0f; 
	float total_time = 0.0f;

	FMOD_System_GetChannelsPlaying(m_system, &channels);
	FMOD_System_GetCPUUsage(m_system, &dsp_tile, &stream_time, &geometry_time, &update_time, &total_time);
	
	g_audio_active_channels.Set(channels);
	g_audio_cpu_usage.Set(total_time);

	AudioRenderer::Tick(time);
}

FMOD_RESULT F_CALLBACK FMod_AudioRenderer::fmod_streaming_callback(FMOD_SOUND *sound, void *data, unsigned int datalen)
{
	void* userdata;
	if (FMOD_Sound_GetUserData(sound, &userdata) != FMOD_OK)
	{
		return FMOD_ERR_FILE_ENDOFDATA;
	}

	FMod_Sound* fs = static_cast<FMod_Sound*>(userdata);
	fs->m_callback(data, datalen, fs->m_callback_metadata);

	return FMOD_OK;
}

void FMod_AudioRenderer::Set_Listener_Count(int count)
{
	FMOD_System_Set3DNumListeners(m_system, count);
}

void FMod_AudioRenderer::Set_Listener_Position(int index, Vector3 position)
{
	if (index < 0)
	{
		index = 0;
	}

	// 1 meter = ~16px in ZG.
	float delta_t = GameEngine::Get()->Get_Time()->Get_Delta_Seconds();
	Vector3 velocity_meters = ((position - m_old_position) / Vector3(16.0f, 16.0f, 16.0f)).Get_Abs();
	Vector3 meters_per_second = (velocity_meters / delta_t);

	FMOD_VECTOR pos = { position.X, position.Y, 0.0f };
	FMOD_VECTOR vel = { meters_per_second.X, meters_per_second.Y, 0.0f };
	FMOD_VECTOR forward = { 0.0f, 1.0f, 0.0f };
	FMOD_VECTOR up = { 1.0f, 0.0f, 0.0f };
	FMOD_System_Set3DListenerAttributes(m_system, index, &pos, &vel, &forward, &up);

	m_old_position = position;
}

Sound* FMod_AudioRenderer::Create_Streaming_Sound(int sample_rate, int channels, SoundFormat::Type type, AudioRequestPCMCallback callback, void* meta_data)
{
	FMOD_MODE mode = FMOD_2D | FMOD_OPENUSER | FMOD_LOOP_NORMAL | FMOD_CREATESTREAM;
	
    FMOD_CREATESOUNDEXINFO createsoundexinfo;
	
	int sample_size = 0;
	FMOD_SOUND_FORMAT format;
	switch (type)
	{
	case SoundFormat::PCM16:
		{
			sample_size = sizeof(signed short);
			format = FMOD_SOUND_FORMAT_PCM16;
			break;
		}
	default:
		{
			return NULL;
		}
	}

	FMod_Sound* result_sound = new FMod_Sound(this, callback, meta_data);

    memset(&createsoundexinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
    createsoundexinfo.cbsize            = sizeof(FMOD_CREATESOUNDEXINFO);             
    createsoundexinfo.decodebuffersize  = (unsigned int)(sample_rate * 0.05f); // 20th of a second.
    createsoundexinfo.length            = sample_rate * channels * sample_size * 5;		// 5 second looping buffer.
    createsoundexinfo.numchannels       = channels;                                  
    createsoundexinfo.defaultfrequency  = sample_rate; 
    createsoundexinfo.format            = format;                  
	createsoundexinfo.pcmreadcallback   = fmod_streaming_callback;  
    createsoundexinfo.pcmsetposcallback = NULL;
	createsoundexinfo.userdata			= result_sound;

	// Create the sound
	FMOD_SOUND* sound = NULL;
	FMOD_RESULT result = FMOD_System_CreateSound(m_system, NULL, mode, &createsoundexinfo, &sound);
	if (result != FMOD_OK)
	{
		DBG_LOG("FMod failed to create streaming sound sound due to error 0x%08x", result);
		SAFE_DELETE(result_sound);
		return NULL;
	}

	result_sound->Set_Sound(sound);
	return result_sound;
}

#endif