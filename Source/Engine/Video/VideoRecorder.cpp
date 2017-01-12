// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Video/VideoRecorder.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/Textures/Pixelmap.h"
#include "Engine/Renderer/Textures/PixelmapFactory.h"
#include "Generic/Helper/StringHelper.h"
#include "Engine/Engine/GameEngine.h"
#include "Engine/IO/StreamFactory.h"
#include "Engine/IO/Stream.h"

#include "Generic/Threads/Mutex.h"
#include "Generic/Threads/MutexLock.h"
#include "Generic/Threads/Thread.h"

VideoRecorder::VideoRecorder()
	: m_active(false)
	, m_shutting_down(false)
{
	m_save_list_mutex = Mutex::Create();

	for (int i = 0; i < max_threads; i++)
	{
		m_save_threads[i] = Thread::Create("Video Encoder", &VideoRecorder::Static_Thread_Entry_Point, this);
		m_save_threads[i]->Start();
	}
}

VideoRecorder::~VideoRecorder()
{
	m_shutting_down = true;
	for (int i = 0; i < max_threads; i++)
	{
		m_save_threads[i]->Join();
		SAFE_DELETE(m_save_threads[i]);
	}
	SAFE_DELETE(m_save_list_mutex);
}

void VideoRecorder::Thread_Entry_Point()
{
	while (!m_shutting_down)
	{
		PendingVideoFrame pix;
		pix.pixelmap = NULL;

		{
			MutexLock lock(m_save_list_mutex);
			if (m_save_list.size() > 0)
			{
				pix = m_save_list[0];
				m_save_list.erase(m_save_list.begin());
			}
		}
		
		if (pix.pixelmap == NULL)
		{
			Platform::Get()->Sleep(10);
		}
		else
		{
			double time = Platform::Get()->Get_Ticks();
	
			unsigned char* buffer = pix.pixelmap->Get_Data();
			unsigned int buffer_size = pix.pixelmap->Get_Data_Size();
			PixelmapFormat::Type format = pix.pixelmap->Get_Format();
			int width = pix.pixelmap->Get_Width();
			int height = pix.pixelmap->Get_Height();

			m_raw_output_stream->Write(buffer_size);
			m_raw_output_stream->Write(format);
			m_raw_output_stream->Write(width);
			m_raw_output_stream->Write(height);
			m_raw_output_stream->WriteBuffer((char*)buffer, 0, buffer_size);

			SAFE_DELETE(pix.pixelmap); 

			/*	std::string filename = StringHelper::Format("%s\\%i.png", m_temp_directory.c_str(), pix.frame);
			PixelmapFactory::Save(filename.c_str(), pix.pixelmap);
			SAFE_DELETE(pix.pixelmap); */

			double elapsed = Platform::Get()->Get_Ticks() - time;
			DBG_LOG("Saved frame in %.1f ms: %i", elapsed, pix.frame); 
		}
	}
}

bool VideoRecorder::IsActive()
{
	return m_active;
}

void VideoRecorder::Start(std::string temp_directory, std::string output_filename)
{
	m_temp_directory = temp_directory;
	m_output_filename = output_filename;
	m_active = true;
	m_frame_count = 0;

	if (!Platform::Get()->Is_Directory(temp_directory.c_str()))
	{
		Platform::Get()->Create_Directory(temp_directory.c_str(), true);
	}

	m_raw_output_filename = m_output_filename + ".raw";
	m_raw_output_stream = StreamFactory::Open(m_raw_output_filename.c_str(), StreamMode::Write);

	GameEngine::Get()->Set_Fixed_Step(true);

	DBG_LOG("[Video] Recording frames to: %s", temp_directory.c_str());
}

void VideoRecorder::End()
{
	DBG_LOG("[Video] Finished recording.");

	// Wait till save list is empty.
	while (true)
	{
		{
			MutexLock lock(m_save_list_mutex);
			if (m_save_list.size() <= 0)
			{
				break;
			}
		}
		Platform::Get()->Sleep(10);
	}

	GameEngine::Get()->Set_Fixed_Step(false);

	m_raw_output_stream->Close();
	SAFE_DELETE(m_raw_output_stream);

	m_active = false;
}

void VideoRecorder::Tick(const FrameTime& time)
{
	if (m_active)
	{
		PendingVideoFrame frame;
		frame.pixelmap = Renderer::Get()->Capture_Backbuffer();		
		frame.frame = m_frame_count++;

		bool bAdded = false;
		while (!bAdded)
		{
			{
				MutexLock lock(m_save_list_mutex);
				if (m_save_list.size() < max_pending_saves)
				{
					m_save_list.push_back(frame);
					bAdded = true;
				}
			}

			if (!bAdded)
			{
				Platform::Get()->Sleep(1);
			}
		}
	}
}
