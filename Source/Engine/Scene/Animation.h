// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_ANIMATION_
#define _ENGINE_ANIMATION_

#include "Engine/Engine/FrameTime.h"

#include "Generic/Types/Vector3.h"

#include "Engine/Scene/Actor.h"

struct AtlasAnimation;
struct AtlasFrame;

#define MAX_ANIMATION_FRAMES 32

struct AnimationMode
{
	enum Type
	{
		Once		= 1,
		Loop		= 2,
		PingPong	= 4
	};

	static AnimationMode::Type Parse(const char* mode);
};

class Animation
{
	MEMORY_ALLOCATOR(Animation, "Scene");

protected:
	u16					m_frames[MAX_ANIMATION_FRAMES];
	u16					m_frame_count;
	float				m_speed;
	float				m_speed_inv;
	AnimationMode::Type m_mode;
	float				m_time;
	u16					m_current_frame;
	int					m_ping_pong_loop;

	bool				m_paused;

	bool				m_reversed;

	AtlasFrame*			m_atlas_frame;
	AtlasAnimation*		m_atlas_animation;

public:

	// Constructors.
	Animation();
	Animation(AtlasAnimation* anim, bool reversed = false);

	void Serialize_Demo(BinaryStream* stream, DemoVersion::Type version, bool bSaving, float frameDelta);

	// Modifiers.
	int		Get_Frame();

	INLINE int Get_Frame_Count()
	{
		return m_frame_count;
	}

	INLINE AtlasAnimation* Get_Atlas_Animation()
	{
		return m_atlas_animation;
	}

	INLINE int Get_Start_Frame()
	{
		return m_frames[0];
	}

	void Set_Reversed(bool val);
	bool Get_Reversed();

	AtlasFrame* Get_Atlas_Frame();
	
	void	Reset();
	void	Pause();
	void	Resume();

	bool	Is_Complete();
	bool	Is_Paused();

	void	Advance(const FrameTime& time, float speed_mult = 1.0f);

};

#endif

