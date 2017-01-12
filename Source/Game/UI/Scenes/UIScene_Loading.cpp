// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_Loading.h"
#include "Game/UI/Scenes/UIScene_MainMenu.h"
#include "Engine/UI/Transitions/UISlideInTransition.h"
#include "Engine/UI/Layouts/UILayoutFactory.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Display/GfxDisplay.h"
#include "Engine/Input/Input.h"
#include "Engine/UI/UIManager.h"
#include "Engine/UI/Elements/UIPanel.h"
#include "Engine/UI/Elements/UIProgressBar.h"
#include "Engine/UI/Elements/UILabel.h"
#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Resources/ResourceFactory.h"

#include "Engine/Scene/Map/MapFileFactory.h"
#include "Engine/Scene/Map/MapFile.h"
#include "Engine/Scene/Map/MapFileHandle.h"

#include "Game/Online/AchievementManager.h"
#include "Game/Online/StatisticsManager.h"
#include "Game/Online/RankingsManager.h"

#include "Engine/Online/OnlineMods.h"

#include "Game/Profile/ProfileManager.h"

#include "Engine/Scripts/EngineVirtualMachine.h"

#include "Engine/Engine/GameEngine.h"
#include "Engine/Engine/EngineOptions.h"

#include "Game/UI/Scenes/UIScene_Game.h"
#include "Game/UI/Scenes/UIScene_Server.h"
#include "Game/UI/Scenes/UIScene_MapLoading.h"
//#include "Game/UI/Scenes/UIScene_SetupServer.h"

#include "Game/Runner/SaveData/SaveData.h"

#include "Game/Runner/GameOptions.h"

#include "Game/Runner/Game.h"

void FinalizeLoadingTask::Run()
{
	// Load in all online mods we are subscribed to.
	if (OnlineMods::Try_Get() != NULL)
	{
 		std::vector<OnlineSubscribedMod*> mods = OnlineMods::Get()->Get_Subscribed_Mods();
		for (std::vector<OnlineSubscribedMod*>::iterator iter = mods.begin(); iter != mods.end(); iter++)
		{
			OnlineSubscribedMod* mod = *iter;
			if (mod->Installed)
			{
				LoadMod(mod);
			}
		}
	}

	// Finalize the loading state of all resources.
	Game::Get()->Finalize_Loading();
}

void FinalizeLoadingTask::LoadMod(OnlineSubscribedMod* mod)
{
	std::string map_file = (mod->InstallDir + "/map.zgmap");

	// Not a map file :(.
	if (!Platform::Get()->Is_File(map_file.c_str()))
	{
		return;
	}

	DBG_LOG("Loading subscribed mod: %s", mod->Title.c_str());

	// Load preview image.
	//mod->Preview  = PixelmapFactory::Load((mod->InstallDir + "/preview.png").c_str());
	//if (mod->Preview != NULL)
	{
		//mod->PreviewTexture = Renderer::Get()->Create_Texture(mod->Preview, TextureFlags::NONE);

		// Load the map.
		MapFileHandle* sm_handle = MapFileFactory::Load(map_file.c_str());
		if (sm_handle == NULL)
		{
			DBG_LOG("Failed to load map from subscribed mod, setting mod to non-installed.");
		}
		else
		{
			mod->Loaded = true;
			sm_handle->Get()->Get_Header()->Workshop_ID = mod->ID;
			sm_handle->Get()->Set_Mod(true);
			sm_handle->Get()->Set_Package(NULL);
			sm_handle->Get()->Set_Online_Mod(mod);
			ResourceFactory::Get()->Add_Map(sm_handle);
		}
	}
	//else
	//{
	//	DBG_LOG("Failed to load preview from subscribed mod, setting mod to non-installed.");
	//}
}

UIScene_Loading::UIScene_Loading()
	: m_load_sequence(0)
	, m_finalize_task(NULL)
{
	Set_Layout("loading");	
	Game::Get()->Set_Loading(true);
}

const char* UIScene_Loading::Get_Name()
{
	return "UIScene_Loading";
}

bool UIScene_Loading::Can_Accept_Invite()
{
	return false;
}

bool UIScene_Loading::Should_Render_Lower_Scenes()
{
	return true;
}

UIScene* UIScene_Loading::Get_Background(UIManager* manager)
{
	return manager->Retrieve_Persistent_Scene("UIScene_MenuBackground");
}	

void UIScene_Loading::Enter(UIManager* manager)
{
	ResourceFactory::Get()->Load_Resources("", false);
	ResourceFactory::Get()->Load_Resources("gfx", false);
}	

void UIScene_Loading::Exit(UIManager* manager)
{
}	

void UIScene_Loading::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{	
	Renderer::Get()->Warmup_Textures();

	if (!ResourceFactory::Get()->Is_Loading())
	{
		if (m_load_sequence == 0)
		{
			ResourceFactory::Get()->Load_Resources("last", false);
			m_load_sequence++;
		}
		else
		{
			if (m_finalize_task != NULL)
			{
				if (m_finalize_task->Is_Completed())
				{
					SAFE_DELETE(m_finalize_task);

					Game::Get()->Finalize_Loading_Main_Thread();

					if (*GameOptions::convert_legacy_maps == true)
					{
						Game::Get()->Convert_Legacy_Maps();
					}

					Game::Get()->Set_Loading(false);
					GameEngine::Get()->Get_VM()->Set_MultiThreaded(false);

					if (*EngineOptions::server)
					{
						manager->Go(UIAction::Replace(new UIScene_Server(), new UISlideInTransition()));
					}
					else
					{
						manager->Go(UIAction::Replace(new UIScene_MainMenu(), new UISlideInTransition()));
					}
			
					return;
				}
			}
			else
			{
				m_finalize_task = new FinalizeLoadingTask();
				m_finalize_task_id = TaskManager::Get()->Add_Task(m_finalize_task);
				TaskManager::Get()->Queue_Task(m_finalize_task_id);
			}
		}
	}

	Renderer::Get()->Warmup_Textures();

	Find_Element<UIProgressBar*>("progress_bar")->Set_Progress(
		(m_load_sequence == 0 ? ResourceFactory::Get()->Load_Percentage() * 0.5f : 0.5f) + 
		(0.25f * m_load_sequence) + 
		(0.25f * GameEngine::Get()->Get_VM()->Get_Register_Progress())
	);

	UIScene::Tick(time, manager, scene_index);
}

void UIScene_Loading::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Draw(time, manager, scene_index);
}



