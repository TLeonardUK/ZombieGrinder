// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_VIDEORECORDER_
#define _ENGINE_VIDEORECORDER_

#include "Engine/Engine/FrameTime.h"
#include <string>
#include <vector>

class Pixelmap;
class Thread;
class Mutex;
class Stream;

struct PendingVideoFrame
{
	Pixelmap* pixelmap;
	int frame;
};

class VideoRecorder
{
private:
	std::string m_temp_directory;
	std::string m_output_filename;
	std::string m_raw_output_filename;

	Stream* m_raw_output_stream;

	bool m_active;
	int m_frame_count;
	bool m_shutting_down;

	enum 
	{
		max_threads = 1,
		max_pending_saves = 16
	};

	Thread* m_save_threads[max_threads];
	Mutex* m_save_list_mutex;
	std::vector<PendingVideoFrame> m_save_list;

protected:
	void Thread_Entry_Point();
	static void Static_Thread_Entry_Point(Thread* self, void* meta)
	{
		reinterpret_cast<VideoRecorder*>(meta)->Thread_Entry_Point();
	}

public:
	VideoRecorder();
	~VideoRecorder();

	void Start(std::string temp_directory, std::string output_filename);
	void End();
	bool IsActive();
	void Tick(const FrameTime& time);

};

#endif