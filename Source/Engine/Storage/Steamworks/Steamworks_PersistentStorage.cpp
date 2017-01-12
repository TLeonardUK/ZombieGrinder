// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Storage/Steamworks/Steamworks_PersistentStorage.h"

#include "public/steam/steam_api.h"

#include "Engine/Platform/Platform.h"

Steamworks_PersistentStorage::Steamworks_PersistentStorage()
{
	m_pending_writes_mutex = Mutex::Create();
	m_pending_writes_semaphore = Semaphore::Create();
	m_pending_writes_thread = Thread::Create("Save Thread", &EntryPoint, this);
	m_closing = false;

	m_pending_writes_thread->Start();
}

Steamworks_PersistentStorage::~Steamworks_PersistentStorage()
{
	m_closing = true;
	m_pending_writes_semaphore->Signal();

	SAFE_DELETE(m_pending_writes_thread);
	SAFE_DELETE(m_pending_writes_semaphore);
	SAFE_DELETE(m_pending_writes_mutex);
}

void Steamworks_PersistentStorage::EntryPoint(Thread* self, void* ptr)
{
	Steamworks_PersistentStorage* storage = reinterpret_cast<Steamworks_PersistentStorage*>(ptr);
	while (!storage->m_closing)
	{
		storage->m_pending_writes_semaphore->Wait();

		Steamworks_PersistentStoragePendingWrite pending_write;
		bool bIsValid = false;

		{
			MutexLock lock(storage->m_pending_writes_mutex);

			if (storage->m_pending_writes.size() > 0)
			{
				pending_write = storage->m_pending_writes[0];
				storage->m_pending_writes.erase(storage->m_pending_writes.begin());
				bIsValid = true;
			}
		}

		// Write time!
		if (bIsValid)
		{
			double start_time = Platform::Get()->Get_Ticks();
			SteamRemoteStorage()->FileWrite(pending_write.filename.c_str(), pending_write.buffer.Buffer(), pending_write.buffer.Size());
			DBG_LOG("[Steamworks Persistent Storage] Persistent storage write to '%s' completed after %.2f ms.", pending_write.filename.c_str(), Platform::Get()->Get_Ticks() - start_time);
		}
	}
}

bool Steamworks_PersistentStorage::File_Exists(const char* url)
{/*
	int32 count = SteamRemoteStorage()->GetFileCount();

	for (int32 i = 0; i < count; i++)
	{
		int32 size;
		const char* url = SteamRemoteStorage()->GetFileNameAndSize(i, &size);
		DBG_LOG("[%i] Size=%i Url=%s", i, size, url);
	}
	*/
	return SteamRemoteStorage()->FileExists(url);
}

int Steamworks_PersistentStorage::File_Size(const char* url)
{
	return SteamRemoteStorage()->GetFileSize(url);
}

bool Steamworks_PersistentStorage::Write_File(const char* url, void* data, int size)
{
	{
		MutexLock lock(m_pending_writes_mutex);

		Steamworks_PersistentStoragePendingWrite pending_write;
		pending_write.filename = url;
		pending_write.buffer.Set((char*)data, size);

		m_pending_writes.push_back(pending_write);
	}

	m_pending_writes_semaphore->Signal();

	return true;
}

int Steamworks_PersistentStorage::Read_File(const char* url, void* data, int size)
{
	return SteamRemoteStorage()->FileRead(url, data, size);
}

void Steamworks_PersistentStorage::Wait_For_Pending_Writes()
{
	m_pending_writes_semaphore->Signal();

	while (m_pending_writes.size() > 0)
	{
		Platform::Get()->Sleep(0.01f);
	}
}