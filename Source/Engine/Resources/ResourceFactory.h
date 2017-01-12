// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RESOURCES_RESOURCEFACTORY_
#define _ENGINE_RESOURCES_RESOURCEFACTORY_

#include "Generic/Types/LinkedList.h"
#include "Generic/Types/ThreadSafeHashTable.h"
#include "Engine/IO/Stream.h"
#include "Engine/IO/FileWatcher.h"
#include "Generic/Patterns/Singleton.h"
#include "Engine/Scene/Animation.h"

#include "Generic/Threads/Mutex.h"
#include "Generic/Threads/MutexLock.h"

#include "Engine/Platform/Platform.h"

#include "Engine/Resources/PackageFile.h"

#include <vector>
#include <string>

class GameEngine;
class AtlasHandle;
struct AtlasFrame;
class SoundHandle;
class FontHandle;
class UILayoutHandle;
class SoundHandle;
class SoundHandle;
class MapFileHandle;
class LanguageHandle;
class ScriptHandle;
class VideoHandle;
class ShaderProgramHandle;
class ParticleFXHandle;
class Task;
class Mutex;

class ResourceFactory : public Singleton<ResourceFactory>
{
	MEMORY_ALLOCATOR(ResourceFactory, "Engine");

private:
	std::string											m_data_directory;
	std::vector<PackageFile*>							m_packages;

	ThreadSafeHashTable<AtlasHandle*,				unsigned int>	m_atlases;

	ThreadSafeHashTable<AtlasFrame*,				unsigned int>	m_atlas_frames;
	ThreadSafeHashTable<int,						unsigned int>	m_atlas_frames_priority;

	ThreadSafeHashTable<AtlasAnimation*,			unsigned int>	m_atlas_animations;	
	ThreadSafeHashTable<int,						unsigned int>	m_atlas_animations_priority;

	ThreadSafeHashTable<SoundHandle*,				unsigned int>	m_sounds;
	ThreadSafeHashTable<int,						unsigned int>	m_sounds_priority;

	ThreadSafeHashTable<FontHandle*,				unsigned int>	m_fonts;

	ThreadSafeHashTable<UILayoutHandle*,			unsigned int>	m_layouts;

	ThreadSafeHashTable<MapFileHandle*,				unsigned int>	m_maps;

	ThreadSafeHashTable<LanguageHandle*,			unsigned int>	m_languages;

	ThreadSafeHashTable<ShaderProgramHandle*,		unsigned int>	m_shaders;

	ThreadSafeHashTable<ScriptHandle*,				unsigned int>	m_scripts;

	ThreadSafeHashTable<ParticleFXHandle*,			unsigned int>	m_particlefx;

	ThreadSafeHashTable<VideoHandle*,				unsigned int>	m_videos;

	std::vector<Task*>									m_load_tasks;
	int													m_initial_load_items;
	Mutex*												m_load_mutex;
	Mutex*												m_atlas_load_mutex;
	Mutex*												m_resource_load_mutex;
	Thread*												m_main_thread;

protected:
	friend class ResourceLoadTask;

	void Load_Atlas				(PackageFile* file, PackageFileChunk* chunk);
	void Load_Sound				(PackageFile* file, PackageFileChunk* chunk);
	void Load_Font				(PackageFile* file, PackageFileChunk* chunk);
	void Load_Language			(PackageFile* file, PackageFileChunk* chunk);
	void Load_Script			(PackageFile* file, PackageFileChunk* chunk);
	void Load_Shader			(PackageFile* file, PackageFileChunk* chunk);
	void Load_Layout			(PackageFile* file, PackageFileChunk* chunk);
	void Load_Map				(PackageFile* file, PackageFileChunk* chunk);
	void Load_ParticleFX		(PackageFile* file, PackageFileChunk* chunk);
	void Load_Video				(PackageFile* file, PackageFileChunk* chunk);

	static bool Sort_Maps_Predicate(MapFileHandle* a, MapFileHandle* b);

public:

	// Constructors
	ResourceFactory									(const char* data_directory);	
	~ResourceFactory								();	

	std::string		Get_Root_Directory				();
	Mutex* Get_Resource_Load_Mutex();
	std::vector<PackageFile*> Get_Packages();

	// General methods.
	bool			Parse_Resources					();

	void			Load_Resources					(const char* priority, bool wait_till_finished, bool main_thread_only = false);
	bool			Is_Loading						();
	float			Load_Percentage					();

	// Pumps the loading queue, should be called once a frame at least.
	void			Pump							();

	// These are duplicate methods from StreamFactory and Platform that
	// take resource mappings into account. These should be used in 
	// preference to their respective brothers.	
	Stream*				Open						(const char* path, StreamMode::Type mode);
	Stream*				Open_Base					(const char* path, StreamMode::Type mode);

	// Atlas resources.
	AtlasHandle*		Get_Atlas					(const char* name);
	AtlasFrame*			Get_Atlas_Frame				(const char* name);
	AtlasFrame*			Get_Atlas_Frame				(int hash);
	AtlasAnimation*		Get_Atlas_Animation			(const char* name);
	AtlasAnimation*		Get_Atlas_Animation			(int hash);

	ThreadSafeHashTable<AtlasFrame*, unsigned int>&		Get_Atlas_Frames		();
	ThreadSafeHashTable<AtlasAnimation*, unsigned int>&	Get_Atlas_Animations	();

	// Audio resources.
	SoundHandle*		Get_Sound					(const char* name);
	SoundHandle*		Get_Sound					(int name);
	ThreadSafeHashTable<SoundHandle*, unsigned int>&	Get_Sounds	();

	// Font resources.
	FontHandle*			Get_Font					(const char* name);
	
	// Layout resources.
	UILayoutHandle*		Get_Layout					(const char* name);

	// Shader resources.
	ShaderProgramHandle* Get_Shader					(const char* name);

	// Video resources.
	VideoHandle* Get_Video							(const char* name);
	
	// ParticleFX resources.
	ParticleFXHandle*    Get_ParticleFX				(const char* name);
	ParticleFXHandle*    Get_ParticleFX				(int hash);
	ThreadSafeHashTable<ParticleFXHandle*, unsigned int>&	Get_ParticleFXs	();
	
	// Shader resources.
	ScriptHandle*							Get_Script	(const char* name);
	ThreadSafeHashTable<ScriptHandle*, unsigned int>&	Get_Scripts	();

	// Map resources.
	void										Add_Map			(MapFileHandle* map);
	MapFileHandle*								Get_Map_File_By_GUID		(const char* guid);
	MapFileHandle*								Get_Map_File_By_Short_Name	(const char* guid);	
	ThreadSafeHashTable<MapFileHandle*, unsigned int>&	Get_Map_Files	();
	std::vector<MapFileHandle*>					Get_Sorted_Map_Files();
	MapFileHandle*								New_Map			(const char* name);

	// Language resources.
	LanguageHandle*								Get_Language	(const char* name);
	ThreadSafeHashTable<LanguageHandle*, unsigned int>&	Get_Languages	();

};

#endif

