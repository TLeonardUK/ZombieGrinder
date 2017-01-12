// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Scene/Animation.h"
#include "Engine/Renderer/Atlases/AtlasRenderer.h"

#include "Engine/Resources/ResourceFactory.h"

#include "Engine/IO/BinaryStream.h"

#include "Generic/Math/Math.h"

AnimationMode::Type AnimationMode::Parse(const char* mode)
{
	AnimationMode::Type		 result		= (AnimationMode::Type)0;
	std::vector<std::string> segments;

	StringHelper::Split(mode, '|', segments);

	for (std::vector<std::string>::iterator iter = segments.begin(); iter != segments.end(); iter++)
	{
		std::string& seg = *iter;

		if (stricmp(seg.c_str(), "once") == 0)
		{
			result = (AnimationMode::Type)((int)result | (int)AnimationMode::Once);
		}
		else if (stricmp(seg.c_str(), "loop") == 0)
		{
			result = (AnimationMode::Type)((int)result | (int)AnimationMode::Loop);
		}
		else if (stricmp(seg.c_str(), "ping_pong") == 0)
		{
			result = (AnimationMode::Type)((int)result | (int)AnimationMode::PingPong);
		}
		else
		{
			DBG_ASSERT_STR(false, "Unknown or invalid animation mode '%s'", seg.c_str());
		}
	}

	return result;
}

Animation::Animation()
	: m_time(0.0f)
	, m_paused(false)
	, m_frame_count(0)
	, m_current_frame(0)
	, m_atlas_animation(NULL)
	, m_atlas_frame(NULL)
	, m_ping_pong_loop(0)
	, m_reversed(false)
{
}

Animation::Animation(AtlasAnimation* anim, bool reversed)
	: m_atlas_frame(NULL)
{
	m_atlas_animation = anim;
	
	if (anim == NULL)
		return;

	m_mode	= anim->Mode;
	m_speed = fabs(anim->Speed);
	m_speed_inv = fabs(1.0f / m_speed);
	m_time	= 0.0f;
	m_paused = false;
	m_frame_count = (u16)anim->Frames.size();
	m_ping_pong_loop = 0;
	m_reversed = reversed;

	DBG_ASSERT(anim->Frames.size() <= MAX_ANIMATION_FRAMES);

	int index = 0;
	for (std::vector<AtlasFrame*>::iterator iter = anim->Frames.begin(); iter != anim->Frames.end(); iter++, index++)
	{
		m_frames[index] = (*iter)->Index;
	}

	if (m_reversed)
	{
		m_current_frame = m_frames[anim->Frames.size() - 1];
	}
	else
	{
		m_current_frame = m_frames[0];
	}
}

void Animation::Serialize_Demo(BinaryStream* stream, DemoVersion::Type version, bool bSaving, float frameDelta)
{
	int nameHash = m_atlas_frame == NULL ? 0 : m_atlas_frame->NameHash;

	stream->Serialize<int>(bSaving, nameHash);

	if (!bSaving)
	{
		m_atlas_frame = (nameHash == 0) ? NULL : ResourceFactory::Get()->Get_Atlas_Frame(nameHash);

		// We always load demo animations as paused.
		m_paused = true;
	}
}

void Animation::Set_Reversed(bool rev)
{
	if (m_reversed != rev)
	{
		m_reversed = rev;
		m_time = 0.0f;

		if (m_atlas_animation != NULL)
		{
			if (m_reversed)
			{
				m_current_frame = m_frames[m_atlas_animation->Frames.size() - 1];
			}
			else
			{
				m_current_frame = m_frames[0];
			}
		}
	}
}

bool Animation::Get_Reversed()
{
	return m_reversed;
}

// Modifiers.
int	Animation::Get_Frame()
{
	return m_current_frame;
}

AtlasFrame* Animation::Get_Atlas_Frame()
{
	return m_atlas_frame;
}

void Animation::Reset()
{
	m_time = 0.0f;
	m_paused = false;
	m_current_frame = m_frames[0];
	m_ping_pong_loop = 0;
}

void Animation::Pause()
{
	m_paused = true;
}

void Animation::Resume()
{
	m_paused = false;
}

bool Animation::Is_Complete()
{
	if (m_mode == AnimationMode::Loop ||
		m_mode == AnimationMode::Once)
	{
		if ((m_mode & AnimationMode::PingPong) != 0)
		{
			return m_time > (m_speed * ((m_frame_count + 1) * 2));
		}
		else
		{
			return m_time > (m_speed * (m_frame_count + 1));
		}
	}
	return false;
}

bool Animation::Is_Paused()
{
	return m_paused;
}

void Animation::Advance(const FrameTime& time, float speed_mult)
{
	if (!m_paused)
	{
		m_time += time.Get_Frame_Time();

		int frame = (int)(m_time * m_speed_inv * speed_mult);
		int original_frame = frame;
		if ((m_mode & AnimationMode::Once) != 0)
		{
			frame = Min(frame, m_frame_count - 1);
		}
		else if ((m_mode & AnimationMode::Loop) != 0)
		{
			if (m_frame_count > 0)
				frame = frame % m_frame_count;
			else
				frame = 0;
		}

		// If ping-pong'ing then reverse frame every other loop.
		if ((m_mode & AnimationMode::PingPong) != 0 && m_frame_count > 0)
		{
			if (original_frame != 0 && (original_frame % m_frame_count) == 0)
			{
				m_ping_pong_loop++;

				if ((m_mode & AnimationMode::Once) != 0)
				{
					m_ping_pong_loop = Min(m_ping_pong_loop, 1);
				}
			}

			if ((m_ping_pong_loop % 2) == 1)
			{
				if ((m_mode & AnimationMode::Once) != 0)
				{
					int real_new_frame = Min(original_frame, (m_frame_count * 2) - 1);
					frame = (m_frame_count - 1) - (real_new_frame % m_frame_count);
				}
				else
				{
					frame = (m_frame_count - 1) - frame;
				}
			}
		}

		if (m_atlas_animation == NULL)
		{
			m_atlas_frame = NULL;
		}
		else
		{
			frame = Clamp(frame, 0, (int)m_atlas_animation->Frames.size() - 1);
			if (m_reversed == true)
			{
				frame = (m_atlas_animation->Frames.size() - 1) - frame;
			}
			m_atlas_frame = m_atlas_animation->Frames[frame];
			m_current_frame = m_frames[frame];
		}
	}
}
