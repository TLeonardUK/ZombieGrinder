// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_VIDEO_VIDEORENDERER_
#define _ENGINE_VIDEO_VIDEORENDERER_

#include <string>

class Texture;
class Video;

struct VideoRendererState
{
	enum Type
	{
		Idle,
		Loading,
		Ready,
		Running,
		Finished,
		Error
	};
};

class VideoRenderer
{
	MEMORY_ALLOCATOR(VideoRenderer, "Engine");

public:

	virtual ~VideoRenderer() {};

	static VideoRenderer* Create();

	// Loading methods.
	virtual void Load(Video* video) = 0;

	// Control methods.
	virtual void Start() = 0;
	virtual void End() = 0;

	// Status methods.
	virtual VideoRendererState::Type Get_State() = 0;

	// Progresses the video by the given time.
	virtual void Tick(float DeltaTime) = 0;

	// Retrieves the texture to display this frame for the video.
	virtual Texture* Get_Texture() = 0;

	// Retrieves the string to display this frame for subtitles.
	virtual std::string Get_Subtitle() = 0;

};

#endif

