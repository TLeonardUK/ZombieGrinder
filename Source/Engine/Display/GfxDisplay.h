// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_DISPLAY_
#define _ENGINE_DISPLAY_

#include "Generic/Patterns/Singleton.h"
//#include "Generic/Types/Vector2.h"
#include "Engine/Engine/FrameTime.h"

#include <vector>

struct GfxDisplayMode
{
	enum Type
	{
		Fullscreen				= 0,
		FullscreenWindowed		= 1,
		Windowed				= 2
	};
};

struct GfxDisplayResolution
{
public:
	int Width;
	int Height;
	int Hertz;
	int Depth;
};

class GfxDisplay : public Singleton<GfxDisplay>
{
	MEMORY_ALLOCATOR(GfxDisplay, "GfxDisplay");

public:
	virtual ~GfxDisplay() {} 

	static GfxDisplay* Create(const char* title, int width, int height, int hertz, GfxDisplayMode::Type fullscreen);
	
	// Base functions.	
	virtual void Tick(const FrameTime& time) = 0;
	virtual void Swap_Buffers() = 0;

	// Properties
	virtual const char* Get_Title() = 0;
	virtual int Get_Width() = 0;
	virtual int Get_Height() = 0;
	virtual int Get_Hertz() = 0;
	virtual GfxDisplayMode::Type Get_Mode() = 0;

	// Modifiers
	virtual void Set_Title(const char* title) = 0;
	virtual void Set_Hidden(bool bHidden) = 0;
	virtual bool Resize(int width, int height, int hertz, GfxDisplayMode::Type mode) = 0;

	// State of window.
	virtual bool  Is_Active() = 0;

	// General stuff.
	virtual std::vector<GfxDisplayResolution> Get_Available_Resolutions() = 0;

};

#endif

