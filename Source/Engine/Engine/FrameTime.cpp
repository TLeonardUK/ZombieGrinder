// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Engine/FrameTime.h"
#include "Engine/Platform/Platform.h"
#include "Generic/Math/Math.h"

#include "Engine/Profiling/ProfilingManager.h"

FrameTime::FrameTime(int target_frame_rate)
	: m_target_frame_rate(target_frame_rate)
	, m_frame_end(0)
	, m_frame_start(0)
	, m_frame_elapsed(0)
	, m_fps(0)
	, m_fps_ticks(0)
	, m_fps_timer(0)
	, m_frame_delta_seconds(0)
	, m_frame(0)
	, m_paused(false)
	, m_single_step(true)
	, m_gpu_time(0.0f)
	, m_frame_elapsed_no_sync(0.0f)
{
}

FrameTime::FrameTime()
{
}

FrameTime::~FrameTime()
{
}

void FrameTime::Single_Step()
{
	m_single_step = true;
	m_paused = true;
}

bool FrameTime::Get_Paused() const
{
	return m_paused;
}

void FrameTime::Set_Paused(bool value)
{
	m_paused = value;
}

float FrameTime::Get_Frame_Time_No_Sync() const
{
	return (float)m_frame_elapsed_no_sync;
}

void FrameTime::New_Tick(bool lock_fps, bool bFixedStep)
{
	double ticks = Platform::Get()->Get_Ticks();
	double tick_rate = 1000.0f / m_target_frame_rate;
	double max_delta = (1.0f);// / 2.0f);		// 1 fps
	double min_delta = (1.0f / 2000.0f);		// 2k fps

	double last_delta = m_frame_delta;

	m_frame_end		= ticks;
	m_frame_elapsed = m_frame_end - m_frame_start;
	m_frame_elapsed_no_sync = m_frame_elapsed;
	
	// Lock fps. 
	// DO NOT USE for anything but locking the tick rate of dedicated servers.
	// clients should use vsync for this.
	if (lock_fps == true)
	{
		double sleep_time = (tick_rate - m_frame_elapsed);
		while (sleep_time > 0 &&
			   Platform::Get()->Get_Ticks() - m_frame_end < sleep_time)
		{
			Platform::Get()->Sleep(1); // Sleep 0 = Yield timeslice
		}
	}

	ticks = Platform::Get()->Get_Ticks();

	m_frame_elapsed = ticks - m_frame_start;
	m_frame_start = ticks;

	if (m_frame_elapsed > 1000.0f)
	{
		DBG_LOG("[Warning] Large Hitch Detected! Frame took %.1f ms", m_frame_elapsed);
	}

	// Work out delta
	m_frame_delta = m_frame_elapsed / tick_rate;
	m_frame_delta_seconds = m_frame_elapsed / 1000.0f;

	// Delta done this way should mean things like x+=speed*delta will apply
	// the speed as a per-second speed.
	m_frame_delta = Math::Lerp(last_delta, (1.0 / tick_rate) * m_frame_delta, 0.2); // Smooth delta changes over several frames.
	m_frame_delta = Clamp(m_frame_delta, min_delta, max_delta);

	//DBG_LOG("Delta=%f min=%f max=%f", m_frame_delta, min_delta, max_delta);

	/*
#ifdef PROFILE_TRACK_BUDGET
	if (m_frame_elapsed < 0.0f || m_frame_elapsed >= 16.0f)
	{
		DBG_LOG("[Frame Budget Elapsed] FrameDelta=%f, Elapsed=%f", m_frame_delta, m_frame_elapsed);
	}
#endif
	*/

	// If paused, zero out delta.
	if (m_paused == true)
	{
		m_frame_delta = 0.0f;
		m_frame_elapsed = 0.0f;
	}

	if (m_single_step == true || bFixedStep == true)
	{
		m_frame_delta = 1.0f / tick_rate;
		m_frame_elapsed = tick_rate;
		m_single_step = false;
	}

	// Work out FPS.
	if (ticks - m_fps_timer >= 1000)
	{
		m_fps_timer = ticks;
		m_fps		= m_fps_ticks;
		m_fps_ticks = 1;
	}
	else
	{
		m_fps_ticks++;
	}

	m_frame++;
}

float FrameTime::Get_Frame_Start_Time() const
{
	return (float)m_frame_start;
}

float FrameTime::Get_Delta() const
{
	return (float)m_frame_delta;
}

float FrameTime::Get_Delta_Seconds() const
{
	return (float)m_frame_delta_seconds;
}

int	FrameTime::Get_Frame() const
{
	return m_frame;
}

int	FrameTime::Get_FPS() const
{
	return m_fps;
}

float FrameTime::Get_Frame_Time() const
{
	return (float)m_frame_elapsed;
}

float FrameTime::Get_Update_Time() const
{
	return (float)m_update_time;
}

float FrameTime::Get_Render_Time() const
{
	return (float)m_render_time;
}

float FrameTime::Get_VSync_Time() const
{
	return (float)m_vsync_time;
}

float FrameTime::Get_GPU_Time() const
{
	return (float)m_gpu_time;
}

void FrameTime::Set_GPU_Time(float time)
{
	m_gpu_time = (double)time;
}

void FrameTime::Begin_Update()
{
	m_update_timer = Platform::Get()->Get_Ticks();
}

void FrameTime::Finish_Update()
{
	m_update_time = Platform::Get()->Get_Ticks() - m_update_timer;
}

void FrameTime::Begin_Draw()
{
	m_render_timer = Platform::Get()->Get_Ticks();
}

void FrameTime::Finish_Draw()
{
	m_render_time = Platform::Get()->Get_Ticks() - m_render_timer;
}

void FrameTime::Begin_VSync()
{
	m_vsync_timer = Platform::Get()->Get_Ticks();
}

void FrameTime::Finish_VSync()
{
	m_vsync_time = Platform::Get()->Get_Ticks() - m_vsync_timer;
}
