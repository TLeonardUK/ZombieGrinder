// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Video/Theora/Theora_VideoRenderer.h"
#include "Engine/Video/VideoRenderer.h"
#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/Textures/Pixelmap.h"

#include "Generic/Threads/Thread.h"
#include "Generic/Threads/Semaphore.h"

#include "Engine/Video/Video.h"

#include "Engine/IO/Stream.h"
#include "Engine/IO/StreamFactory.h"

#include "Engine/Resources/ResourceFactory.h"

#include "Engine/Audio/AudioRenderer.h"
#include "Engine/Audio/Sounds/Sound.h"
#include "Engine/Audio/Sounds/SoundChannel.h"

#include "theoraplayer/TheoraAudioInterface.h"
#include "theoraplayer/TheoraVideoManager.h"
#include "theoraplayer/TheoraVideoClip.h"
#include "theoraplayer/TheoraVideoFrame.h"
#include "theoraplayer/TheoraDataSource.h"

#include "Engine/Localise/Locale.h"

#include "Generic/Types/TemporalBufferedByteStack.h"

class TheoraResourceDataSource : public TheoraDataSource
{
private:
	Stream* m_stream;

public:
	TheoraResourceDataSource(Stream* stream)
		: m_stream(stream)
	{
	}

	virtual ~TheoraResourceDataSource()
	{
		SAFE_DELETE(m_stream);
	}

	virtual int read(void* output,int nBytes)
	{
		int toReadBytes = Min(nBytes, (int)m_stream->Bytes_Remaining());
		if (toReadBytes > 0)
		{
			m_stream->ReadBuffer((char*)output, 0, toReadBytes);
		}
		return toReadBytes;
	}

	virtual std::string repr()
	{
		return "Resource-Stream";
	}

	virtual void seek(uint64_t byte_index)
	{
		m_stream->Seek((unsigned int)byte_index);
	}

	virtual uint64_t size()
	{
		return m_stream->Length();
	}

	virtual uint64_t tell()
	{
		return m_stream->Position();
	}
};

class Theora_AudioInterface : public TheoraAudioInterface
{
private:
	Sound* m_sound;
	SoundChannel* m_channel;

	Mutex*							m_queue_mutex;
	TemporalBufferedByteStack		m_queue;

	enum
	{	
		max_data_buffer = 32 * 1024,
		buffer_artificial_latency = 0,
	};

public:
	Theora_AudioInterface(TheoraVideoClip* owner, int nChannels, int freq)
		: TheoraAudioInterface(owner, nChannels, freq)
		, m_queue(max_data_buffer, buffer_artificial_latency)
	{
		m_queue_mutex = Mutex::Create();
		m_sound = AudioRenderer::Get()->Create_Streaming_Sound(freq, nChannels, SoundFormat::PCM16, &Theora_AudioInterface::StaticFeedCallback, this);
		m_channel = m_sound->Allocate_Channel(true);
		m_sound->Play(m_channel, false);
	}

	virtual ~Theora_AudioInterface()
	{
		SAFE_DELETE(m_channel);
		SAFE_DELETE(m_sound);
		SAFE_DELETE(m_queue_mutex);
	}

	static short float2short(float f)
	{
		if (f >  1) f = 1;
		else if (f < -1) f = -1;
		return (short)(f * 32767);
	}

	virtual void insertData(float* data, int nSamples)
	{
		MutexLock lock(m_queue_mutex);

		double time = Platform::Get()->Get_Ticks();
		short* buffer = (short*)m_queue.GetBuffer(nSamples * sizeof(short), time);// reinterpret_cast<char*>(data), nSamples * sizeof(float) * mNumChannels, time);
		for (int i = 0; i < nSamples; i++)
		{
			*buffer = float2short(data[i]);;
			buffer++;
		}
	}

	void FeedCallback(void* data, int size)
	{
		double time = Platform::Get()->Get_Ticks();
		int max_data = 0;

		{
			MutexLock lock(m_queue_mutex);

			max_data = Min(m_queue.Available(time), size);
			m_queue.Read((char*)data, max_data);
		}

		// Zero out the rest of the buffer if we have no more voice data available.
		if (max_data < size)
		{
			if (max_data > 0)
			{
				DBG_LOG("Not enough audio data to fill playback buffer (asked for %i, have %i). Data starvation may cause audio artifacts.", size, max_data);
			}
			memset((char*)data + max_data, 0, size - max_data);
		}
	}

	static void StaticFeedCallback(void* data, int size, void* meta_data)
	{
		reinterpret_cast<Theora_AudioInterface*>(meta_data)->FeedCallback(data, size);
	}

};

class Theora_AudioInterfaceFactory : public TheoraAudioInterfaceFactory
{
public:
	virtual Theora_AudioInterface* createInstance(TheoraVideoClip* owner, int nChannels, int freq)
	{
		return new Theora_AudioInterface(owner, nChannels, freq);
	}
};

Theora_VideoRenderer::Theora_VideoRenderer()
	: m_state(VideoRendererState::Idle)
	, m_video(NULL)
	, m_clip(NULL)
	, m_data_source(NULL)
	, m_texture(NULL)
	, m_pixelmap(NULL)
{
	m_manager = new TheoraVideoManager();

	m_audio_interface_factory = new Theora_AudioInterfaceFactory();
	m_manager->setAudioInterfaceFactory(m_audio_interface_factory);
}

Theora_VideoRenderer::~Theora_VideoRenderer()
{
	if (m_clip)
	{
		m_manager->destroyVideoClip(m_clip);
		m_clip = NULL;
	}

	// Note: don't destroy data stream, destroyVideoClip does it.
	m_data_source = NULL;

	SAFE_DELETE(m_manager);
}

void Theora_VideoRenderer::Load(Video* video)
{
	DBG_ASSERT(m_state != VideoRendererState::Running);

	m_video = video;
	m_state = VideoRendererState::Loading;

	if (m_clip)
	{
		m_manager->destroyVideoClip(m_clip);
		m_clip = NULL;
	}

	// Note: don't destroy data stream, destroyVideoClip does it.
	m_data_source = NULL;

	if (m_texture)
	{
		SAFE_DELETE(m_texture);
	}

	if (m_pixelmap)
	{
		SAFE_DELETE(m_pixelmap);
	}

	Stream* stream = ResourceFactory::Get()->Open(video->Get_URL().c_str(), StreamMode::Read);
	if (!stream)
	{
		DBG_LOG("[Video] Failed to open stream to: %s", video->Get_URL().c_str());
		m_state = VideoRendererState::Error;
		return;
	}

	Stream* subtitle_stream = ResourceFactory::Get()->Open(video->Get_Subtitle_URL().c_str(), StreamMode::Read);
	if (!subtitle_stream)
	{
		DBG_LOG("[Video] Failed to open subtitle stream to: %s", video->Get_URL().c_str());
	}
	else
	{
		Load_Subtitles(subtitle_stream);
	}

	DBG_LOG("[Video] Loading video: %s", video->Get_URL().c_str());
	DBG_LOG("[Video] Loading video subtitles: %s", video->Get_Subtitle_URL().c_str());

	m_data_source = new TheoraResourceDataSource(stream);

	m_clip = m_manager->createVideoClip(m_data_source, TH_RGB);
	if (!m_clip)
	{
		DBG_LOG("[Video] Failed to create video clip for: %s", video->Get_URL().c_str());
		m_state = VideoRendererState::Error;
		return;
	}
}

float Theora_VideoRenderer::Parse_Subtitle_Time(std::string time)
{
	std::vector<std::string> time_split;
	StringHelper::Split(StringHelper::Trim(time.c_str()).c_str(), ':', time_split);

	if (time_split.size() == 3)
	{
		float hours = (float)atoi(time_split[0].c_str());
		float minutes = (float)atoi(time_split[1].c_str());
		float seconds = 0.0f;
		float milliseconds = 0.0f;

		std::vector<std::string> second_split;
		StringHelper::Split(StringHelper::Trim(time_split[2].c_str()).c_str(), ',', second_split);
		if (second_split.size() == 2)
		{
			seconds = (float)atoi(second_split[0].c_str());
			milliseconds = (float)atoi(second_split[1].c_str());
		}

		float total_ms = milliseconds;
		total_ms += seconds * 1000.0f;
		total_ms += minutes * 60.0f * 1000.0f;
		total_ms += hours * 60.0f * 60.0f * 1000.0f;

		return total_ms / 1000.0f;
	}

	return 0.0f;
}

bool Theora_Subtitle_Sort(const Theora_Subtitle& i, const Theora_Subtitle& j)
{ 
	return (i.Order_Number < j.Order_Number); 
}

void Theora_VideoRenderer::Load_Subtitles(Stream* stream)
{
	while (!stream->IsEOF())
	{
		std::string order_number = stream->ReadLine();
		if (order_number != "" && !stream->IsEOF())
		{
			std::string duration = stream->ReadLine();
			std::string description = "";
			while (!stream->IsEOF())
			{
				std::string sub_line = stream->ReadLine();
				if (sub_line == "")
				{
					break;
				}
				if (description != "")
				{
					description += "\n";
				}
				description += sub_line;
			}

			Theora_Subtitle subtitle;
			subtitle.Order_Number = atoi(order_number.c_str());
			subtitle.Text = description;
			subtitle.Start_Time = 0.0f;
			subtitle.End_Time = 0.0f;

			std::vector<std::string> duration_split;
			StringHelper::Split(StringHelper::Trim(duration.c_str()).c_str(), ' ', duration_split);
			if (duration_split.size() == 3)
			{
				subtitle.Start_Time = Parse_Subtitle_Time(duration_split[0]);
				subtitle.End_Time = Parse_Subtitle_Time(duration_split[2]);
			}

			m_subtitles.push_back(subtitle);

			//DBG_LOG("Loaded Subtitle: order=%s duration=%.2f/%.2f text=%s", order_number.c_str(), subtitle.Start_Time, subtitle.End_Time, description.c_str());
		}
	}

	std::sort(m_subtitles.begin(), m_subtitles.end(), &Theora_Subtitle_Sort);
}

void Theora_VideoRenderer::Start()
{
	DBG_ASSERT(m_state == VideoRendererState::Ready);

	if (m_clip)
	{
		m_clip->play();
	}
	
	DBG_LOG("[Video] Starting video: %s", m_video->Get_URL().c_str());

	m_state = VideoRendererState::Running;
}

void Theora_VideoRenderer::End()
{
	DBG_ASSERT(m_state == VideoRendererState::Running);

	if (m_clip)
	{
		m_clip->stop();
	}

	DBG_LOG("[Video] Ending video: %s", m_video->Get_URL().c_str());

	m_state = VideoRendererState::Finished;
}

VideoRendererState::Type Theora_VideoRenderer::Get_State()
{
	return m_state;
}

void Theora_VideoRenderer::Tick(float DeltaTime)
{
	switch (m_state)
	{
		case VideoRendererState::Idle:
		case VideoRendererState::Ready:
		case VideoRendererState::Finished:
		case VideoRendererState::Error:
		{
			// Do nothing.
			break;
		}
		case VideoRendererState::Loading:
		{
			if (m_clip->getNumReadyFrames() > 0)
			{
				DBG_LOG("[Video] Video is ready: %s", m_video->Get_URL().c_str());
				m_state = VideoRendererState::Ready;
			}
			break;
		}
		case VideoRendererState::Running:
		{
			m_manager->update(DeltaTime);

			TheoraVideoFrame* frame = m_clip->getNextFrame();
			if (frame)
			{
				if (!m_texture || 
					m_clip->getWidth() != m_pixelmap->Get_Width() || 
					m_clip->getHeight() != m_pixelmap->Get_Height() || 
					m_clip->getStride() != m_pixelmap->Get_Pitch())
				{
					u8* data = new u8[m_clip->getStride() * m_clip->getHeight() * 4];
					m_pixelmap = new Pixelmap(data, m_clip->getWidth(), m_clip->getHeight(), m_clip->getStride(), PixelmapFormat::R8G8B8);
					m_texture = Renderer::Get()->Create_Texture(m_pixelmap, TextureFlags::PersistSourceData);
				}

				memcpy(m_pixelmap->Get_Data(), frame->getBuffer(), m_pixelmap->Get_Data_Size());
				m_pixelmap->Flip_Vertical();
				m_texture->Set_Pixelmap(m_pixelmap);

			/*	DBG_LOG("[%i/%i] Size=%i,%i Position=%.2f Duration=%.2f FPS=%.2f", 
					frame->getFrameNumber(),
					m_clip->getNumFrames(),
					m_clip->getWidth(), 
					m_clip->getHeight(),
					m_clip->getTimePosition(),
					m_clip->getDuration(),
					m_clip->getFPS());*/

				m_clip->popFrame(); 
			}

			if (m_clip->isDone())
			{
				DBG_LOG("[Video] Video has finished: %s", m_video->Get_URL().c_str());
				m_state = VideoRendererState::Finished;
			}

			break;
		}
	}
}

Texture* Theora_VideoRenderer::Get_Texture()
{
	return m_texture;
}

std::string Theora_VideoRenderer::Get_Subtitle()
{
	float time = 0.0f;
	if (m_state == VideoRendererState::Running && m_clip)
	{
		time = m_clip->getTimePosition();
	}

	std::string output = "";

	for (std::vector<Theora_Subtitle>::iterator iter = m_subtitles.begin(); iter != m_subtitles.end(); iter++)
	{
		Theora_Subtitle& subtitle = *iter;
		if (time >= subtitle.Start_Time && time < subtitle.End_Time)
		{
			if (output != "")
			{
				output += "\n";
			}
			output += S(subtitle.Text.c_str());
		}
	}

	return output;
}
