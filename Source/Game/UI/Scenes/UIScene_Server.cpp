// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_Server.h"
#include "Game/UI/Scenes/UIScene_MapLoading.h"
#include "Game/Network/GameNetManager.h"
#include "Game/Network/GameNetServer.h"
#include "Game/Runner/Game.h"

#include "Engine/UI/UIManager.h"
#include "Engine/UI/Layouts/UILayoutFactory.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Display/GfxDisplay.h"
#include "Engine/Engine/GameEngine.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Input/Input.h"
#include "Engine/Engine/EngineOptions.h"
#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Scene/Map/MapFile.h"
#include "Engine/Scene/Map/MapFileHandle.h"
#include "Engine/Scene/Map/Blocks/MapFileHeaderBlock.h"

#include "Engine/Resources/ResourceFactory.h"

#include "Generic/Math/Random.h"

UIScene_Server::UIScene_Server()
{
	Set_Layout("game");

	DBG_LOG("Initializing server ...");
	GameNetManager::Get()->Init_Server();

	MapFileHandle* mapHandle = ResourceFactory::Get()->Get_Map_File_By_Short_Name((*EngineOptions::server_map_name).c_str());
	if (mapHandle == NULL)
	{
		mapHandle = ResourceFactory::Get()->Get_Map_File_By_Short_Name((*EngineOptions::server_safe_map_name).c_str());
	}
	DBG_ASSERT(mapHandle != NULL);

	GameNetManager::Get()->Server()->Force_Map_Change(
		mapHandle->Get()->Get_Header()->GUID.c_str(),
		mapHandle->Get()->Get_Header()->Workshop_ID, 
		Random::Static_Next(),
		0);

	Game::Get()->Set_Update_Game(true);
}

UIScene_Server::~UIScene_Server()
{
	DBG_LOG("Destroying server ...");
	GameNetManager::Get()->Destroy_Server();

	Game::Get()->Set_Update_Game(false);
}

const char* UIScene_Server::Get_Name()
{
	return "UIScene_Server";
}

bool UIScene_Server::Should_Render_Lower_Scenes()
{
	return false;
}

void UIScene_Server::Enter(UIManager* manager)
{
}	

void UIScene_Server::Exit(UIManager* manager)
{
}	

void UIScene_Server::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	// Has map changed? Do we need to load the next one?
	if (NetManager::Get()->Is_Map_Load_Pending() && manager->Get_Scene_By_Type<UIScene_MapLoading*>() == NULL)
	{
		manager->Go(UIAction::Push(new UIScene_MapLoading()));
		return;
	}

	Game::Get()->Set_Update_Actors(!GameNetManager::Get()->Game_Server()->Get_Server_State().In_Editor_Mode);

	UIScene::Tick(time, manager, scene_index);
}

void UIScene_Server::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
}



