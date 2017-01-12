// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_SCENE_POST_PROCESS_MANAGER_
#define _GAME_SCENE_POST_PROCESS_MANAGER_

#include "Engine/IO/BinaryStream.h"
#include "Engine/Engine/FrameTime.h"
#include "Engine/Scene/PostProcess/PostProcessManager.h"

#include "Engine/Online/OnlinePlatform.h"

#include "XScript/VirtualMachine/CVMObject.h"

#include "Game/Scripts/ScriptEventListener.h"

#include "Generic/Patterns/Singleton.h"

struct AtlasFrame;
class ShaderProgramHandle;

struct PostProcessEffect
{
public:
	bool					active;
	RenderPipeline_Shader*	resolved_shader;
	std::string				shader;

	ScriptEventListener*	event_listener;
	CVMGCRoot				script_object;

	int						camera_index;

};

struct PostProcess
{
public:
	PostProcessEffect		effects[MAX_LOCAL_PLAYERS];

};

class GamePostProcessManager : public PostProcessManager
{
	MEMORY_ALLOCATOR(GamePostProcessManager, "Game");

private:
	bool m_init;
	bool m_first_load;

	std::vector<PostProcess> m_instances;
	std::vector<PostProcessEffect*> m_active_instances;

public:
	static GamePostProcessManager* Get()
	{
		return dynamic_cast<GamePostProcessManager*>(PostProcessManager::Get());
	}

	GamePostProcessManager();
	~GamePostProcessManager();

	bool Init();

	//int Get_PostProcess_Count();
	//PostProcessEffect* Get_PostProcess(int index);
	PostProcessEffect* Get_Instance(CVMLinkedSymbol* sym, int camera_index);

	void Tick(const FrameTime& time);

	virtual int Get_Pass_Count(int camera_index);
	virtual RenderPipeline_Shader* Get_Pass_Shader(int camera_index, int pass);
	virtual void Set_Pass_Uniforms(int camera_index, int pass);

};

#endif

