// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Resources/ResourceFactory.h"
#include "Engine/Resources/ResourceLoadTask.h"
#include "Engine/IO/StreamFactory.h"

#include "Engine/Localise/LanguageFactory.h"
#include "Engine/Localise/LanguageHandle.h"
#include "Engine/Localise/Language.h"


#include "Engine/Video/VideoFactory.h"
#include "Engine/Video/VideoHandle.h"
#include "Engine/Video/Video.h"


#include "Engine/Scripts/ScriptFactory.h"
#include "Engine/Scripts/ScriptHandle.h"
#include "Engine/Scripts/Script.h"

#include "Engine/Resources/PackageFileStream.h"

#include "Engine/Resources/Compiled/Atlases/CompiledAtlas.h"
#include "Engine/Resources/Compiled/Audio/CompiledAudio.h"
#include "Engine/Resources/Compiled/Fonts/CompiledFont.h"
#include "Engine/Resources/Compiled/Languages/CompiledLanguage.h"
#include "Engine/Resources/Compiled/Layouts/CompiledLayout.h"
#include "Engine/Resources/Compiled/Maps/CompiledMap.h"
#include "Engine/Resources/Compiled/Raw/CompiledRaw.h"
#include "Engine/Resources/Compiled/Scripts/CompiledScript.h"
#include "Engine/Resources/Compiled/Shaders/CompiledShader.h"
#include "Engine/Resources/Compiled/CompiledPixelmap.h"

#include "Engine/IO/PatchedBinaryStream.h"

#include "Engine/Audio/Sounds/Sound.h"
#include "Engine/Audio/Sounds/SoundHandle.h"
#include "Engine/Audio/Sounds/SoundFactory.h"

#include "Engine/Renderer/Atlases/AtlasFactory.h"
#include "Engine/Renderer/Text/FontFactory.h"
#include "Engine/Renderer/Shaders/ShaderFactory.h"
#include "Engine/Renderer/Shaders/ShaderProgramHandle.h"
#include "Engine/Scene/Map/MapFileFactory.h"
#include "Engine/Scene/Map/MapFile.h"
#include "Engine/Scene/Map/MapFileHandle.h"

#include "Engine/Online/OnlineMods.h"

#include "Engine/UI/Layouts/UILayoutFactory.h"
#include "Engine/UI/Layouts/UILayoutHandle.h"
#include "Engine/UI/Layouts/UILayout.h"

#include "Engine/Particles/ParticleFXFactory.h"
#include "Engine/Particles/ParticleFXHandle.h"
#include "Engine/Particles/ParticleFX.h"

#include "Engine/Config/ConfigFile.h"

#include "Engine/Platform/Platform.h"

#include "Engine/Tasks/TaskManager.h"
#include "Engine/Tasks/Task.h"
#include "Engine/Tasks/GroupTask.h"

#include "Engine/Localise/Locale.h"

#include <algorithm>

unsigned char PUBLIC_RESOURCE_KEY[160] =
{
	0x30,0x81,0x9D,0x30,0x0D,0x06,0x09,0x2A,0x86,0x48,0x86,
	0xF7,0x0D,0x01,0x01,0x01,0x05,0x00,0x03,0x81,0x8B,0x00,
	0x30,0x81,0x87,0x02,0x81,0x81,0x00,0xDE,0xBB,0xAA,0xE4,
	0x19,0xD8,0xDB,0x99,0xB0,0x4E,0x19,0x75,0x68,0x99,0x13,
	0xF4,0x55,0x4E,0x05,0x9F,0x78,0x64,0x55,0xF9,0xA3,0x4A,
	0xE7,0x01,0xAA,0x1A,0x00,0xA1,0x19,0xEC,0x45,0xC5,0x64,
	0xAC,0xD4,0x33,0x11,0xF3,0x30,0x7E,0xAD,0x49,0x6A,0x79,
	0xED,0x30,0x9A,0x32,0x9F,0x01,0x58,0x94,0x9A,0x78,0x99,
	0xB3,0xE0,0x72,0xAD,0x0B,0xE3,0x64,0xF6,0xDE,0x75,0x42,
	0xD5,0x76,0xE7,0x90,0x63,0x17,0x54,0xF4,0x54,0x58,0x81,
	0x3B,0x17,0x05,0x2E,0xCF,0x47,0xF8,0x31,0x84,0x2E,0x49,
	0xD0,0x7D,0xFD,0xFC,0x66,0xC2,0x50,0xAF,0xCD,0xF6,0x87,
	0xC7,0x37,0x23,0xEC,0x57,0x0F,0x22,0x6F,0x75,0xF9,0x61,
	0x9C,0xE9,0x8F,0x52,0x05,0xE6,0xAA,0x37,0x7E,0x8D,0xD7,
	0x2E,0x0B,0xDD,0x02,0x01,0x11,
};

ResourceFactory::ResourceFactory(const char* data_directory)
	: m_data_directory("")
	, m_initial_load_items(0)
{
	Platform* platform = Platform::Get();

	DBG_LOG("Base working directory: %s", platform->Get_Working_Dir().c_str());

	// Kepp going up directory levels until we find data directory.
	std::string full_path = platform->Get_Absolute_Path(data_directory);
	DBG_LOG("Full search path: %s", full_path.c_str());

	if (full_path.at(full_path.size() - 1) == '/' ||
		full_path.at(full_path.size() - 1) == '\\')
	{
		full_path = std::string(full_path.begin(), full_path.end() - 1);
	}

	DBG_LOG("Full search path, post-cut: %s", full_path.c_str());

	std::string dir = platform->Extract_Directory(full_path);
	std::string top_level_dir_name = platform->Extract_Filename(full_path);

	DBG_LOG("Initial search=%s top-level=%s", dir.c_str(), top_level_dir_name.c_str());

	while (true)
	{
		std::string full = platform->Join_Path(dir, top_level_dir_name);
		DBG_LOG("Looking for resource directory at: %s", full.c_str());

		if (platform->Is_Directory(full.c_str()) &&
			platform->Is_File(platform->Join_Path(full, "GeoIP.dat").c_str()))
		{
			m_data_directory = full + "/";
			DBG_LOG("Found resource directory at: %s", m_data_directory.c_str());
			break;
		}

		std::string new_dir = platform->Extract_Directory(dir);
		if (dir == new_dir || new_dir == "")
		{
			break;
		}

		dir = new_dir;
	}

	if (m_data_directory == "")
	{
		DBG_ASSERT_STR(false, "Resource directory does not exist at path: %s", data_directory);
	}
	else
	{
		DBG_LOG("Using data directory: %s", m_data_directory.c_str());
	}

	m_load_mutex = Mutex::Create();
	m_atlas_load_mutex = Mutex::Create();
	m_resource_load_mutex = Mutex::Create();
	m_main_thread = Thread::Get_Current();
}

ResourceFactory::~ResourceFactory()
{
	m_packages.clear();
}

std::vector<PackageFile*> ResourceFactory::Get_Packages()
{
	return m_packages;
}

Mutex* ResourceFactory::Get_Resource_Load_Mutex()
{
	return m_resource_load_mutex;
}

std::string ResourceFactory::Get_Root_Directory()
{
	return m_data_directory;
}

Stream* ResourceFactory::Open(const char* path, StreamMode::Type mode)
{
	DBG_ASSERT_STR(mode == StreamMode::Read, "Resource factory can only open files in read-only mode!");

	// Search for file in reverse order. Later loaded packages override earlier loaded packages.
	for (int i = m_packages.size() - 1; i >= 0; i--)
	{
		PackageFile* package = m_packages[i];

		PackageFileStream* stream = package->Open_Chunk_Stream(path);
		if (stream != NULL)
		{
			return stream;
		}
	}

	return NULL;
}

Stream* ResourceFactory::Open_Base(const char* path, StreamMode::Type mode)
{
	DBG_ASSERT_STR(mode == StreamMode::Read, "Resource factory can only open files in read-only mode!");

	PackageFile* package = m_packages[0];

	PackageFileStream* stream = package->Open_Chunk_Stream(path);
	if (stream != NULL)
	{
		return stream;
	}

	return NULL;
}

bool ResourceFactory::Parse_Resources()
{
	Platform* platform = Platform::Get();
	std::vector<std::string> folders;

	if (!platform->List_Directory(m_data_directory.c_str(), folders, ListDirectoryFlags::DirectoriesOnly))
	{
		DBG_ASSERT_STR(false, "Could not list data directory: %s", m_data_directory.c_str());
	}

	std::vector<std::string> names;

	int load_priority = 0;

	// Load main packages.
	for (int priority = 0; priority < 2; priority++)
	{
		for (std::vector<std::string>::iterator iter = folders.begin(); iter != folders.end(); iter++)
		{
			std::string resource_dir_name	= *iter;
			std::string base_path			= m_data_directory + resource_dir_name + "/";
			std::string package_path		= base_path + resource_dir_name + ".dat";

			// Check the config file exists.
			if (!platform->Is_File(package_path.c_str()))
			{
				continue;
			}

			// Always load base first.
			if (resource_dir_name == "Base")
			{
				if (priority != 0)
				{
					continue;
				}
			}
			else
			{
				if (priority == 0)
				{
					continue;
				}
			}

			DBG_LOG("Found resource package: %s", package_path.c_str());

			names.push_back(StringHelper::Lowercase(resource_dir_name.c_str()));

			// Load package.
			PackageFile* package_file = new PackageFile();
			CryptoKey public_key(PUBLIC_RESOURCE_KEY, sizeof(PUBLIC_RESOURCE_KEY));
			package_file->Set_Keys(CryptoKey::Empty, public_key);
			package_file->Open(package_path.c_str());
			m_packages.push_back(package_file);
			package_file->Priority = load_priority++;

			// Enforce signing on base resource package.
			if (priority == 0)
			{
				DBG_ASSERT_STR(package_file->Is_Signed(), "'Base' resource package is corrupt or has been tampered with. Game is unable to continue.");
			}
		}
	}

	// Load online-mods.
	if (OnlineMods::Try_Get() != NULL)
	{
		std::vector<OnlineSubscribedMod*> mods = OnlineMods::Get()->Get_Subscribed_Mods();
		for (std::vector<OnlineSubscribedMod*>::iterator iter = mods.begin(); iter != mods.end(); iter++)
		{
			OnlineSubscribedMod* mod = *iter;
			if (mod->Installed)
			{
				std::string name = "";
				std::vector<std::string> files;
				std::string dat_file = "";

				// Try and find the xml file!
				Platform::Get()->List_Directory(mod->InstallDir.c_str(), files, ListDirectoryFlags::FilesOnly);
				for (std::vector<std::string>::iterator fileIter = files.begin(); fileIter != files.end(); fileIter++)
				{
					std::string file = *fileIter;
					std::string ext = Platform::Get()->Extract_Extension(file);
					if (ext == "xml")
					{
						name = Platform::Get()->Extract_Basename(file);
					}
					else if (ext == "dat")
					{
						dat_file = mod->InstallDir + "/" + file;
					}
				}

				if (name == "")
				{
					continue;
				}

				// If mod has same name as local mod, ignore.
				std::vector<std::string>::iterator name_pos = std::find(names.begin(), names.end(), StringHelper::Lowercase(name.c_str()));
				if (name_pos == names.end())
				{
					DBG_LOG("Found workshop resource package: %s", mod->InstallDir.c_str());

					// Load package.
					PackageFile* package_file = new PackageFile();
					CryptoKey public_key(PUBLIC_RESOURCE_KEY, sizeof(PUBLIC_RESOURCE_KEY));
					package_file->Set_Keys(CryptoKey::Empty, public_key);
					package_file->Open(dat_file.c_str());
					m_packages.push_back(package_file);
					package_file->Priority = load_priority++;

					// Whitelist package?
					if (mod->IsWhitelisted)
					{
						DBG_LOG("Workshop package is whitelisted.");
						package_file->Whitelist();
					}

					names.push_back(StringHelper::Lowercase(name.c_str()));

					mod->Loaded = true;
				}
			}
		}
	}

	return true;
}

void ResourceFactory::Load_Resources(const char* priority, bool wait_till_finished, bool main_thread_only)
{
	if (wait_till_finished == true)
	{
		DBG_LOG("Performing blocking load of resources with priority '%s'.", priority);
	}
	else
	{
		DBG_LOG("Performing non-blocking load of resources with priority '%s'.", priority);
	}

	m_initial_load_items = 0;

	TaskID parent_task_id = -1;
	GroupTask* parent_task = NULL;

	if (wait_till_finished == true && main_thread_only == false)
	{
		parent_task = new GroupTask(StringHelper::Format("Resource Load Group (Priority=%s)", priority));
		parent_task_id = TaskManager::Get()->Add_Task(parent_task);
	}

	unsigned int priority_hash = StringHelper::Hash(priority);
	u32 affinity = 0xFFFFFFFF;

	// GFX resources need to be pinned to a couple of workers only, otherwise we can
	// blow our memory when we try to load a lot of un-warmed textures.
	if (strcmp(priority, "gfx") == 0)
	{
		affinity = 2;
	}

	// Go through packages in reverse. load first instance of each resource file. This allows mod packages to override base packages.
	std::vector<int> loaded_hashes;
	for (int i = m_packages.size() - 1; i >= 0; i--)
	{
		PackageFile* file = m_packages[i];

		int chunk_count = file->Get_Chunk_Count();
		for (int i = 0; i < chunk_count; i++)
		{
			PackageFileChunk* chunk = file->Get_Chunk(i);
			
			// Ignore source/raw files, we will never use them, no point
			// loading them.
			if (chunk->type == PackageFileChunkType::Source ||
				chunk->type == PackageFileChunkType::Raw)
			{
				continue;
			}

			unsigned int chunk_priority = file->Get_String_Hash(chunk->priority_offset);
			const char* chunk_name = file->Get_String(chunk->name_offset);

			int search_hash = StringHelper::Hash(StringHelper::Lowercase(chunk_name).c_str());

			// Only load first instance.
			if (std::find(loaded_hashes.begin(), loaded_hashes.end(), search_hash) != loaded_hashes.end())
			{
				DBG_LOG("Chunk '%s' has been overriden in a mod package.", chunk_name);
				continue;
			}
			loaded_hashes.push_back(search_hash);

			if (chunk_priority == priority_hash)
			{
				ResourceLoadTask* task = new ResourceLoadTask(this, file, chunk);

				TaskID task_id = TaskManager::Get()->Add_Task(task, parent_task_id, affinity);	
				if (main_thread_only == true)
				{			
					TaskManager::Get()->Run_Task(task_id);
				}
				else
				{
					TaskManager::Get()->Queue_Task(task_id);
					m_load_tasks.push_back(task);
					DBG_LOG("\tAdded file '%s' to load queue.", chunk_name);
				}
	
				m_initial_load_items++;
			}
		}
	}

	// Block until we are done!
	if (wait_till_finished == true || main_thread_only == true)
	{
		if (main_thread_only == true)
		{
			Pump();
			DBG_LOG("Loading complete.");
		}
		else
		{
			DBG_LOG("Waiting until loading has finished.");
			TaskManager::Get()->Queue_Task(parent_task_id);
			TaskManager::Get()->Wait_For(parent_task_id);
			Pump();
			DBG_LOG("Loading complete.");
		}
	}
}

bool ResourceFactory::Is_Loading()
{
	return m_load_tasks.size() > 0;
}

float ResourceFactory::Load_Percentage()
{
	int to_load  = m_load_tasks.size();
	int loaded   = m_initial_load_items - to_load; 

	return (float)loaded / (float)m_initial_load_items;
}

void ResourceFactory::Pump()
{
	// Have old tasks finished?
	for (std::vector<Task*>::iterator iter = m_load_tasks.begin(); iter != m_load_tasks.end(); )
	{
		Task* task = *iter;

		if (task->Is_Completed())
		{
			iter = m_load_tasks.erase(iter);
		}
		else
		{
			iter++;
		}
	}
}

void ResourceFactory::Load_Atlas(PackageFile* file, PackageFileChunk* chunk)
{
	MutexLock lock(m_atlas_load_mutex);

	const char* path = file->Get_String(chunk->name_offset);

	AtlasHandle* atlas_handle = AtlasFactory::Load(path);
	Atlas* atlas = atlas_handle->Get();

	{
		MutexLock lock2(m_load_mutex);

		// Insert atlas into map.
		unsigned int hash = StringHelper::Hash(atlas->Get_Name().c_str());
		m_atlases.Set(hash, atlas_handle);

		// Insert frames.
		std::vector<AtlasFrame*>& frames = atlas->Get_Frames_List();
		for (std::vector<AtlasFrame*>::iterator iter = frames.begin(); iter != frames.end(); iter++)
		{
			AtlasFrame* frame = *iter;

			if (m_atlas_frames.Contains(frame->NameHash))
			{
				int priority = m_atlas_frames_priority.Get(frame->NameHash);
				if (file->Priority >= priority)
				{
					DBG_LOG("Encountered duplicate atlas frame (all frames in game have to have unique names!) frame '%s' in atlas '%s'. Overriding.", frame->Name.c_str(), path);
				}
				else
				{
					continue;
				}

				m_atlas_frames.Remove(frame->NameHash);
				m_atlas_frames_priority.Remove(frame->NameHash);
			}

			m_atlas_frames.Set(frame->NameHash, frame);
			m_atlas_frames_priority.Set(frame->NameHash, file->Priority);
		}

		// Insert animations.
		std::vector<AtlasAnimation*>& anims = atlas->Get_Animations_List();
		for (std::vector<AtlasAnimation*>::iterator iter = anims.begin(); iter != anims.end(); iter++)
		{
			AtlasAnimation* frame = *iter;

			if (m_atlas_animations.Contains(frame->NameHash))
			{
				int priority = m_atlas_animations_priority.Get(frame->NameHash);
				if (file->Priority >= priority)
				{
					DBG_LOG("Encountered duplicate atlas animation (all animations in game have to have unique names!) animation '%s' in atlas '%s'. Overriding.", frame->Name.c_str(), path)
				}
				else
				{
					continue;
				}

				m_atlas_animations.Remove(frame->NameHash);
				m_atlas_animations_priority.Remove(frame->NameHash);
			}

			m_atlas_animations.Set(frame->NameHash, frame);
			m_atlas_animations_priority.Set(frame->NameHash, file->Priority);
		}
	}
}

void ResourceFactory::Load_Sound(PackageFile* file, PackageFileChunk* chunk)
{
	const char* path = file->Get_String(chunk->name_offset);

	SoundHandle* sound_handle = SoundFactory::Load(path);
	DBG_ASSERT_STR(sound_handle, "Failed to load sound: %s", path);

	const Sound* bank = sound_handle->Get();

	{
		MutexLock lock(m_load_mutex);

		unsigned int hash = StringHelper::Hash(bank->Get_Name().c_str());

		if (m_sounds.Contains(hash))
		{
			int priority = m_sounds_priority.Get(hash);
			if (file->Priority >= priority)
			{
				DBG_LOG("Encountered duplicate sound '%s'. Overriding.", bank->Get_Name().c_str())
			}
			else
			{
				return;
			}

			m_sounds.Remove(hash);
			m_sounds_priority.Remove(hash);
		}

		m_sounds.Set(hash, sound_handle);
		m_sounds_priority.Set(hash, file->Priority);
	}
}

void ResourceFactory::Load_Font(PackageFile* file, PackageFileChunk* chunk)
{
	const char* path = file->Get_String(chunk->name_offset);

	FontHandle* font_handle = FontFactory::Load(path, FontFlags::NONE);
	DBG_ASSERT_STR(font_handle, "Failed to load font: %s", path);

	Font* font = font_handle->Get();

	{
		MutexLock lock(m_load_mutex);

		// Insert atlas into map.
		unsigned int hash = StringHelper::Hash(font->Get_Name().c_str());
		m_fonts.Set(hash, font_handle);
	}
}

void ResourceFactory::Load_Language(PackageFile* file, PackageFileChunk* chunk)
{
	const char* path = file->Get_String(chunk->name_offset);

	LanguageHandle* language_handle = LanguageFactory::Load(path);
	DBG_ASSERT_STR(language_handle, "Failed to load language: %s", path);

	Language* language = language_handle->Get();
	language->Priority = file->Priority;

	{
		MutexLock lock(m_load_mutex);

		unsigned int hash = StringHelper::Hash(language->Short_Name);

		if (m_languages.Contains(hash))
		{
			Language* existing_language = m_languages.Get(hash)->Get();
			existing_language->Add_Sub_Language(language);
			existing_language->Refresh_Strings();
		}
		else
		{
			m_languages.Set(hash, language_handle);
			language->Refresh_Strings();
		}
	}
}

void ResourceFactory::Load_Layout(PackageFile* file, PackageFileChunk* chunk)
{
	const char* path = file->Get_String(chunk->name_offset);

	UILayoutHandle* sm_handle = UILayoutFactory::Load(path);
	DBG_ASSERT_STR(sm_handle, "Failed to load layout: %s", path);

	UILayout* sm = sm_handle->Get();
	
	{
		MutexLock lock(m_load_mutex);

		// Insert atlas into map.
		unsigned int hash = StringHelper::Hash(sm->Get_Name().c_str());
		m_layouts.Set(hash, sm_handle);
	}
}

void ResourceFactory::Load_Video(PackageFile* file, PackageFileChunk* chunk)
{
	const char* path = file->Get_String(chunk->name_offset);

	VideoHandle* sm_handle = VideoFactory::Load(path);
	DBG_ASSERT_STR(sm_handle, "Failed to load video: %s", path);

	Video* sm = sm_handle->Get();

	{
		MutexLock lock(m_load_mutex);

		// Insert atlas into map.
		unsigned int hash = StringHelper::Hash(sm_handle->Get_URL().c_str());
		m_videos.Set(hash, sm_handle);
	}
}

void ResourceFactory::Load_Map(PackageFile* file, PackageFileChunk* chunk)
{
	MutexLock lock(m_load_mutex);

	const char* path = file->Get_String(chunk->name_offset);

	MapFileHandle* sm_handle = MapFileFactory::Load(path);
	DBG_ASSERT_STR(sm_handle, "Failed to load map: %s", path);

	MapFile* sm = sm_handle->Get();
	
	// Insert atlas into map.
	unsigned int hash = StringHelper::Hash(sm->Get_Header()->GUID.c_str());

	// Update workshop id if loaded as part of a mod.
	sm->Get_Header()->Workshop_ID = file->Get_Header()->workshop_id;
	sm->Set_Package(file);
	sm->Set_Online_Mod(NULL);
	sm->Set_Mod(false);
	m_maps.Set(hash, sm_handle);
}

void ResourceFactory::Add_Map(MapFileHandle* sm_handle)
{
	MapFile* sm = sm_handle->Get();

	{
		MutexLock lock(m_load_mutex);

		// Insert atlas into map.
		unsigned int hash = StringHelper::Hash(sm->Get_Header()->GUID.c_str());
		m_maps.Set(hash, sm_handle);
	}
}

void ResourceFactory::Load_Script(PackageFile* file, PackageFileChunk* chunk)
{
	const char* path = file->Get_String(chunk->name_offset);

	ScriptHandle* sm_handle = ScriptFactory::Load(path);
	DBG_ASSERT_STR(sm_handle, "Failed to load script: %s", path);

	Script* script = sm_handle->Get();
	
	{
		MutexLock lock(m_load_mutex);

		// Insert atlas into map.
		unsigned int hash = StringHelper::Hash(script->Get_Name().c_str());
		m_scripts.Set(hash, sm_handle);
	}
}

void ResourceFactory::Load_Shader(PackageFile* file, PackageFileChunk* chunk)
{
	const char* path = file->Get_String(chunk->name_offset);

	ShaderProgramHandle* shader_handle = ShaderFactory::Load(path);
	if (shader_handle == NULL)
	{
		return;
	}
	//DBG_ASSERT_STR(shader_handle, "Failed to load shader: %s", path);

	ShaderProgram* shader = shader_handle->Get();

	{
		MutexLock lock(m_load_mutex);

		// Insert atlas into map.
		unsigned int hash = shader->Get_Resource()->name_hash;
		m_shaders.Set(hash, shader_handle);
	}
}

void ResourceFactory::Load_ParticleFX(PackageFile* file, PackageFileChunk* chunk)
{
	const char* path = file->Get_String(chunk->name_offset);

	ParticleFXHandle* sm_handle = ParticleFXFactory::Load(path);
	DBG_ASSERT_STR(sm_handle, "Failed to load particlefx: %s", path);

	ParticleFX* sm = sm_handle->Get();
	
	{
		MutexLock lock(m_load_mutex);

		// Insert particlefx into map.
		unsigned int hash = StringHelper::Hash(sm->Get_Name().c_str());
		m_particlefx.Set(hash, sm_handle);
	}
}

AtlasHandle* ResourceFactory::Get_Atlas(const char* name)
{
	{
		MutexLock lock(m_load_mutex);
		return m_atlases.Get(StringHelper::Hash(name));
	}
}

AtlasFrame*	ResourceFactory::Get_Atlas_Frame(const char* name)
{
	{
		MutexLock lock(m_load_mutex);
		return m_atlas_frames.Get(StringHelper::Hash(name));
	}
}

AtlasFrame*	ResourceFactory::Get_Atlas_Frame(int hash)
{
	{
		MutexLock lock(m_load_mutex);
		return m_atlas_frames.Get(hash);
	}
}

AtlasAnimation*	ResourceFactory::Get_Atlas_Animation(const char* name)
{
	{
		MutexLock lock(m_load_mutex);
		return m_atlas_animations.Get(StringHelper::Hash(name));
	}
}

AtlasAnimation*	ResourceFactory::Get_Atlas_Animation(int hash)
{
	{
		MutexLock lock(m_load_mutex);
		return m_atlas_animations.Get(hash);
	}
}

ThreadSafeHashTable<AtlasFrame*, unsigned int>& ResourceFactory::Get_Atlas_Frames()
{
	{
		MutexLock lock(m_load_mutex);
		return m_atlas_frames;
	}
}

ThreadSafeHashTable<AtlasAnimation*, unsigned int>& ResourceFactory::Get_Atlas_Animations()
{
	{
		MutexLock lock(m_load_mutex);
		return m_atlas_animations;
	}
}

ThreadSafeHashTable<SoundHandle*, unsigned int>& ResourceFactory::Get_Sounds()
{
	{
		MutexLock lock(m_load_mutex);
		return m_sounds;
	}
}

VideoHandle* ResourceFactory::Get_Video(const char* url)
{
	{
		MutexLock lock(m_load_mutex);
		return m_videos.Get(StringHelper::Hash(url));
	}
}

SoundHandle* ResourceFactory::Get_Sound(const char* name)
{	
	{
		MutexLock lock(m_load_mutex);
		return m_sounds.Get(StringHelper::Hash(name));
	}
}

SoundHandle* ResourceFactory::Get_Sound(int name)
{
	{
		MutexLock lock(m_load_mutex);
		return m_sounds.Get(name);
	}
}

FontHandle* ResourceFactory::Get_Font(const char* name)
{
	{
		MutexLock lock(m_load_mutex);
		return m_fonts.Get(StringHelper::Hash(name));
	}
}

UILayoutHandle* ResourceFactory::Get_Layout(const char* name)
{
	{
		MutexLock lock(m_load_mutex);
		return m_layouts.Get(StringHelper::Hash(name));
	}
}

ParticleFXHandle* ResourceFactory::Get_ParticleFX(const char* name)
{
	{
		MutexLock lock(m_load_mutex);
		return m_particlefx.Get(StringHelper::Hash(name));
	}
}

ParticleFXHandle* ResourceFactory::Get_ParticleFX(int hash)
{
	{
		MutexLock lock(m_load_mutex);
		return m_particlefx.Get(hash);
	}
}

ThreadSafeHashTable<ParticleFXHandle*, unsigned int>& ResourceFactory::Get_ParticleFXs()
{
	MutexLock lock(m_load_mutex);
	return m_particlefx;
}

LanguageHandle* ResourceFactory::Get_Language(const char* name)
{
	{
		MutexLock lock(m_load_mutex);

		unsigned int hash = StringHelper::Hash(name);
	
		// Hack for converting en-us to en-gb :(
		if (StringHelper::Lowercase(name) == "en-uk")
		{
			hash = StringHelper::Hash("EN-GB");
		}

		return m_languages.Get(hash);
	}
}

ShaderProgramHandle* ResourceFactory::Get_Shader(const char* name)
{
	{
		MutexLock lock(m_load_mutex);
		return m_shaders.Get(StringHelper::Hash(name));
	}
}

ScriptHandle* ResourceFactory::Get_Script(const char* name)
{
	{
		MutexLock lock(m_load_mutex);
		return m_scripts.Get(StringHelper::Hash(name));
	}
}

ThreadSafeHashTable<LanguageHandle*, unsigned int>& ResourceFactory::Get_Languages()
{
	{
		MutexLock lock(m_load_mutex);
		return m_languages;
	}
}

MapFileHandle* ResourceFactory::Get_Map_File_By_GUID(const char* guid)
{
	{
		MutexLock lock(m_load_mutex);
		return m_maps.Get(StringHelper::Hash(guid));
	}
}

MapFileHandle* ResourceFactory::Get_Map_File_By_Short_Name(const char* short_name)
{
	{
		MutexLock lock(m_load_mutex);

		ThreadSafeHashTable<MapFileHandle*, unsigned int> maps = Get_Map_Files();

		for (ThreadSafeHashTable<MapFileHandle*, unsigned int>::Iterator iter = maps.Begin(); iter != maps.End(); iter++)
		{
			MapFileHandle* handle = *iter;
			if (handle->Get()->Get_Header()->Short_Name == short_name)
			{
				return handle;
			}
		}

		return NULL;
	}
}

MapFileHandle* ResourceFactory::New_Map(const char* name)
{
	MapFileHandle* sm_handle = MapFileFactory::New(name);
	MapFile* sm = sm_handle->Get();

	{
		MutexLock lock(m_load_mutex);

		// Insert atlas into map.
		unsigned int hash = StringHelper::Hash(name);
		m_maps.Set(hash, sm_handle);
	}

	return sm_handle;
}

ThreadSafeHashTable<MapFileHandle*, unsigned int>& ResourceFactory::Get_Map_Files()
{
	{
		MutexLock lock(m_load_mutex);
		return m_maps;
	}
}

bool ResourceFactory::Sort_Maps_Predicate(MapFileHandle* a, MapFileHandle* b)
{
	return a->Get()->Get_Header()->Short_Name < b->Get()->Get_Header()->Short_Name;
}

std::vector<MapFileHandle*> ResourceFactory::Get_Sorted_Map_Files()
{
	ThreadSafeHashTable<MapFileHandle*, unsigned int> maps = Get_Map_Files();
	std::vector<MapFileHandle*> result;

	for (ThreadSafeHashTable<MapFileHandle*, unsigned int>::Iterator iter = maps.Begin(); iter != maps.End(); iter++)
	{
		MapFileHandle* handle = *iter;
		result.push_back(handle);
	}

	std::sort(result.begin(), result.end(), &ResourceFactory::Sort_Maps_Predicate);

	return result;
}

ThreadSafeHashTable<ScriptHandle*, unsigned int>& ResourceFactory::Get_Scripts()
{
	{
		MutexLock lock(m_load_mutex);
		return m_scripts;
	}
}
