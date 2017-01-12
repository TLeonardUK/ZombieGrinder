// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Resources/ResourceLoadTask.h"

ResourceLoadTask::ResourceLoadTask(ResourceFactory* factory, PackageFile* file, PackageFileChunk* chunk)
	: m_factory(factory)
	, m_package_file(file)
	, m_package_chunk(chunk)
{
}

void ResourceLoadTask::Run()
{
	switch (m_package_chunk->type)
	{		
	case PackageFileChunkType::Atlas:
		m_factory->Load_Atlas(m_package_file, m_package_chunk);
		break;
	case PackageFileChunkType::Sound:
		{
			MutexLock lock(ResourceFactory::Get()->Get_Resource_Load_Mutex());
			m_factory->Load_Sound(m_package_file, m_package_chunk);
			break;
		}
	case PackageFileChunkType::Font:
		m_factory->Load_Font(m_package_file, m_package_chunk);
		break;
	case PackageFileChunkType::Language:
		m_factory->Load_Language(m_package_file, m_package_chunk);
		break;
	case PackageFileChunkType::Layout:
		m_factory->Load_Layout(m_package_file, m_package_chunk);
		break;
	case PackageFileChunkType::Map:
		m_factory->Load_Map(m_package_file, m_package_chunk);
		break;
	case PackageFileChunkType::Script:
		m_factory->Load_Script(m_package_file, m_package_chunk);
		break;
	case PackageFileChunkType::Shader:
		m_factory->Load_Shader(m_package_file, m_package_chunk);
		break;
	case PackageFileChunkType::ParticleFX:
		m_factory->Load_ParticleFX(m_package_file, m_package_chunk);
		break;
	case PackageFileChunkType::Video:
		m_factory->Load_Video(m_package_file, m_package_chunk);
		break;
	case PackageFileChunkType::Raw:
	case PackageFileChunkType::Source:
		// We should never try to load these directly.
		break;
	}

	const char* name = m_package_file->Get_String(m_package_chunk->name_offset);
	DBG_LOG("Finished loading resource '%s'.", name);
}
