// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Scene/PostProcess/GamePostProcessManager.h"
#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMBinary.h"

#include "Engine/Engine/GameEngine.h"
#include "Engine/Scripts/EngineVirtualMachine.h"

#include "Engine/Engine/EngineOptions.h"

#include "Generic/Stats/Stats.h"

#include "Game/Runner/Game.h"
#include "Game/Runner/GameMode.h"
#include "Game/Runner/GameOptions.h"

DEFINE_GLOBAL_STATISTIC("PostProcess/Active Effects", int, g_stat_post_process_active_effects,				false);

GamePostProcessManager::GamePostProcessManager()
	: m_init(false)
	, m_first_load(false)
{
}

GamePostProcessManager::~GamePostProcessManager()
{
	for (std::vector<PostProcess>::iterator iter = m_instances.begin(); iter != m_instances.end(); iter++)
	{
		PostProcess& p = *iter;
		for (int i = 0; i < MAX_LOCAL_PLAYERS; i++)
		{
			SAFE_DELETE(p.effects[i].event_listener);
		}
	}

	m_active_instances.clear();
}

/*int GamePostProcessManager::Get_PostProcess_Count()
{
	return m_instances.size();
}

PostProcess* GamePostProcessManager::Get_PostProcess(int index)
{
	return &m_instances.at(index);
}*/

PostProcessEffect* GamePostProcessManager::Get_Instance(CVMLinkedSymbol* sym, int camera_index)
{
	DBG_ASSERT(camera_index >= 0 && camera_index < MAX_LOCAL_PLAYERS);

	// This is some tricksy shit. Basically if we only have one camera active (and all players are sharing the same one)
	// we redirect all requests to index 0. Should probably do this in script :S.
	GameMode* mode = Game::Get()->Get_Game_Mode();
	if (mode != NULL && mode->Get_Camera_Count() == 1)
	{
		camera_index = 0;
	}

	for (std::vector<PostProcess>::iterator iter = m_instances.begin(); iter != m_instances.end(); iter++)
	{
		PostProcess& p = *iter;
		if (p.effects[camera_index].script_object.Get().Get()->Get_Symbol() == sym)
		{
			return &p.effects[camera_index];
		}
	}

	return NULL;
}

bool GamePostProcessManager::Init()
{	
	DBG_ASSERT(!m_init);
	DBG_LOG("Loading post process effects ...");

	EngineVirtualMachine* vm = GameEngine::Get()->Get_VM();
	CVMLinkedSymbol* base_symbol = vm->Find_Class("Post_Process_FX");
	DBG_ASSERT(base_symbol != NULL);

	std::vector<CVMLinkedSymbol*> extended_classes = vm->Find_Derived_Classes(base_symbol);

	m_instances.resize(extended_classes.size());

	CVMContextLock lock = vm->Set_Context(vm->Get_Static_Context());

	int index = 0;
	for (std::vector<CVMLinkedSymbol*>::iterator iter = extended_classes.begin(); iter != extended_classes.end(); iter++, index++)
	{
		CVMLinkedSymbol* achievement_class = *iter;
		DBG_LOG("Loading Post Process FX Class: %s", achievement_class->symbol->name);

		PostProcess& p = m_instances.at(index);
		for (int i = 0; i < MAX_LOCAL_PLAYERS; i++)
		{
			p.effects[i].active = false;
			p.effects[i].camera_index = i;
			p.effects[i].script_object = vm->New_Object(achievement_class, true, &p.effects[i]);
			vm->Set_Default_State(p.effects[i].script_object);

			p.effects[i].event_listener = new ScriptEventListener(vm->Get_Static_Context(), p.effects[i].script_object);
		}
	}

	m_init = true;
	return true;
}

void GamePostProcessManager::Tick(const FrameTime& time)
{
	if (!m_init)
		return;

	m_active_instances.clear();
	for (std::vector<PostProcess>::iterator iter = m_instances.begin(); iter != m_instances.end(); iter++)
	{
		PostProcess& p = *iter;
		for (int i = 0; i < MAX_LOCAL_PLAYERS; i++)
		{
			if (p.effects[i].active == true)
			{
				m_active_instances.push_back(&p.effects[i]);
			}
		}
	}

	g_stat_post_process_active_effects.Set((int)m_active_instances.size());
}

int GamePostProcessManager::Get_Pass_Count(int camera_index)
{
	if (!(*GameOptions::shaders_enabled))
	{
		return 0;
	}

	int count = 0;
	for (std::vector<PostProcessEffect*>::iterator iter = m_active_instances.begin(); iter != m_active_instances.end(); iter++)
	{
		if ((*iter)->camera_index == camera_index)
		{
			count++;
		}
	}

	return count;
}

RenderPipeline_Shader* GamePostProcessManager::Get_Pass_Shader(int camera_index, int pass)
{
	int count = 0;
	for (std::vector<PostProcessEffect*>::iterator iter = m_active_instances.begin(); iter != m_active_instances.end(); iter++)
	{
		PostProcessEffect* e = *iter;
		if (e->camera_index == camera_index)
		{
			if (count == pass)
			{
				// have to re-resolve each frame as render-pipeline may be reloaded at any time ;_;
				e->resolved_shader = RenderPipeline::Get()->Get_Shader_From_Name(e->shader.c_str());
				return e->resolved_shader;
			}
			count++;
		}
	}
	return NULL;
}

void GamePostProcessManager::Set_Pass_Uniforms(int camera_index, int pass)
{
	int count = 0;
	for (std::vector<PostProcessEffect*>::iterator iter = m_active_instances.begin(); iter != m_active_instances.end(); iter++)
	{
		PostProcessEffect* e = *iter;
		if (e->camera_index == camera_index)
		{
			if (count == pass)
			{
				e->event_listener->On_Shader_Setup();
				return;
			}
			count++;
		}
	}
}
