// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_FRAMETIME_
#define _ENGINE_FRAMETIME_

class FrameTime
{
	MEMORY_ALLOCATOR(FrameTime, "Engine");

public:
	FrameTime(int target_frame_rate);
	FrameTime();
	~FrameTime();

	void	New_Tick(bool lock_fps = false, bool bFixedStep = false);
	float	Get_Delta()			const;
	float	Get_Delta_Seconds()	const;
	int		 Get_FPS()			const;
	float	 Get_Frame_Time()	const;
	float	 Get_Update_Time()	const;
	float	 Get_Render_Time()	const;
	float	 Get_VSync_Time()	const;
	float	 Get_GPU_Time()		const;
	float	 Get_Frame_Start_Time() const;
	int		Get_Frame()			const;

	float	Get_Frame_Time_No_Sync() const;

	bool Get_Paused() const;
	void Set_Paused(bool value);

	void Single_Step();
	
	void	Begin_Update();
	void	Finish_Update();
	void	Begin_Draw();
	void	Finish_Draw();
	void	Begin_VSync();
	void	Finish_VSync();
	void	Set_GPU_Time(float time);

private:
	int		m_target_frame_rate;
	
	double	m_update_timer;
	double	m_update_time;

	double	m_render_timer;
	double	m_render_time;

	double	m_vsync_timer;
	double	m_vsync_time;

	double	m_gpu_time;

	double	m_frame_start;
	double	m_frame_end;
	double	m_frame_elapsed;
	double	m_frame_delta;
	double	m_frame_delta_seconds;

	double	m_frame_elapsed_no_sync;

	int		m_frame;

	int		m_fps;
	int		m_fps_ticks;
	double	m_fps_timer;

	bool	m_paused;
	bool	m_single_step;

};

#endif

