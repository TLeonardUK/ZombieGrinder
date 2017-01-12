// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_VIDEO_THEORA_VIDEORENDERER_
#define _ENGINE_VIDEO_THEORA_VIDEORENDERER_

#include "Engine/Video/VideoRenderer.h"

#include <string>
#include <vector>

class Texture;
class Video;

class Theora_AudioInterfaceFactory;
class TheoraVideoManager;
class TheoraVideoClip;
class TheoraDataSource;
class Pixelmap;
class Texture;
class SoundChannel;
class Stream;

struct Theora_Subtitle
{
	int Order_Number;
	float Start_Time;
	float End_Time;
	std::string Text;
};

class Theora_VideoRenderer : public VideoRenderer
{
	MEMORY_ALLOCATOR(Theora_VideoRenderer, "Engine");

private:
	Video* m_video;
	VideoRendererState::Type m_state;
	TheoraVideoManager* m_manager;
	Theora_AudioInterfaceFactory* m_audio_interface_factory;
	TheoraVideoClip* m_clip;
	TheoraDataSource* m_data_source;
 
	Pixelmap* m_pixelmap;
	Texture* m_texture;

	std::vector<Theora_Subtitle> m_subtitles;

	float Parse_Subtitle_Time(std::string time);
	void Load_Subtitles(Stream* stream);

public:

	Theora_VideoRenderer();
	~Theora_VideoRenderer();

	// Loading methods.
	virtual void Load(Video* video);

	// Control methods.
	virtual void Start();
	virtual void End();

	// Status methods.
	virtual VideoRendererState::Type Get_State();

	// Progresses the video by the given time.
	virtual void Tick(float DeltaTime);

	// Retrieves the texture to display this frame for the video.
	virtual Texture* Get_Texture();

	// Retrieves the string to display this frame for subtitles.
	virtual std::string Get_Subtitle();

};

#endif

