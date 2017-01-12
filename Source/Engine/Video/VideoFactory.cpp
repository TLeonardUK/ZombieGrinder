// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Video/VideoFactory.h"

#include "Engine/Resources/ResourceFactory.h"
#include "Engine/IO/PatchedBinaryStream.h"

#include "Engine/Platform/Platform.h"

#include "Engine/Config/ConfigFile.h"

#include "Engine/IO/StreamFactory.h"

#include "Generic/Helper/StringHelper.h"

ThreadSafeHashTable<VideoHandle*, int> VideoFactory::m_loaded_banks;

VideoHandle* VideoFactory::Load(const char* url)
{
	int url_hash = StringHelper::Hash(url);

	// Texture already loaded?
	if (m_loaded_banks.Contains(url_hash))
	{
		DBG_LOG("Loaded video from cache: %s", url);
		return m_loaded_banks.Get(url_hash);
	}

	// Try and load texture!
	Video* sound = Load_Without_Handle(url);
	if (sound != NULL)
	{
		VideoHandle* handle = new VideoHandle(url, sound);

		m_loaded_banks.Set(url_hash, handle);

		return handle;
	}

	return NULL;
}

Video* VideoFactory::Load_Without_Handle(const char* url)
{
	Video* result = Try_Load(url);
	if (result != NULL)
	{
		DBG_LOG("Loaded video: %s", url);
		return result;
	}

	DBG_LOG("Failed to load video: %s", url);
	return NULL;
}

Video* VideoFactory::Try_Load(const char* url)
{
	Platform* platform = Platform::Get();

	Video* video = new Video();
	video->m_url = url;
	video->m_subtitle_url = Platform::Get()->Change_Extension(url, ".srt");

	DBG_LOG("Finished loading video from '%s'.", url);
	return video;
}

void VideoFactory::Dispose()
{
	for (ThreadSafeHashTable<VideoHandle*, int>::Iterator iter = m_loaded_banks.Begin(); iter != m_loaded_banks.End(); iter++)
	{
		VideoHandle* handle = *iter;
		SAFE_DELETE(handle);
	}
	m_loaded_banks.Clear();
}

std::vector<VideoHandle*> VideoFactory::Get_Languages()
{
	std::vector<VideoHandle*> result;
	for (ThreadSafeHashTable<VideoHandle*, int>::Iterator iter = m_loaded_banks.Begin(); iter != m_loaded_banks.End(); iter++)
	{
		VideoHandle* handle = *iter;
		result.push_back(handle);
	}
	return result;
}