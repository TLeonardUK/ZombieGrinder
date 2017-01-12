// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/Backgrounds/UIScene_GameBackground.h"
#include "Engine/UI/Layouts/UILayoutFactory.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Display/GfxDisplay.h"
#include "Engine/Input/Input.h"
#include "Engine/UI/UIManager.h"
#include "Engine/UI/Elements/UIPanel.h"
#include "Engine/UI/Elements/UILabel.h"
#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Resources/ResourceFactory.h"
#include "Engine/Display/GfxDisplay.h"

#include "Engine/Engine/GameEngine.h"
#include "Engine/Scene/Scene.h"

#include "Game/Network/GameNetManager.h"
#include "Game/Network/GameNetClient.h"
#include "Game/Network/GameNetServer.h"

#include "Generic/Math/Math.h"
#include "Generic/Math/Random.h"

#include "Engine/Localise/Locale.h"

#include "Game/UI/Scenes/UIScene_Game.h"

#include "Game/Runner/Game.h"
#include "Game/Runner/GameMode.h"

UIScene_GameBackground::UIScene_GameBackground()
{
	Set_Layout("game_background");	
}

const char* UIScene_GameBackground::Get_Name()
{
	return "UIScene_GameBackground";
}

bool UIScene_GameBackground::Should_Render_Lower_Scenes()
{
	return false;
}

bool UIScene_GameBackground::Should_Tick_When_Not_Top()
{
	return true;
}

void UIScene_GameBackground::Refresh(UIManager* manager)
{
	// Refresh base stuff.
	UIScene::Refresh(manager);
}

void UIScene_GameBackground::Enter(UIManager* manager)
{
}	

void UIScene_GameBackground::Exit(UIManager* manager)
{
}	

void UIScene_GameBackground::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Tick(time, manager, scene_index);
}

void UIScene_GameBackground::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{	
	bool is_editor_mode	= GameNetManager::Get()->Game_Client()->Get_Server_State().In_Editor_Mode;

	Color batch_color = RenderPipeline::Get()->Get_Render_Batch_Global_Color();

	float display_width = (float)GfxDisplay::Get()->Get_Width();
	float display_height = (float)GfxDisplay::Get()->Get_Height();

	// Render the game camera.
	if (is_editor_mode)
	{
		RenderPipeline::Get()->Set_Active_Camera(Game::Get()->Get_Camera(CameraID::Editor_Main));
		RenderPipeline::Get()->Get_Active_Camera()->Set_Screen_Viewport(Rect2D(0.0f, 0.0f, display_width, display_height));

		Renderer::Get()->Set_Depth_Test(false);
		RenderPipeline::Get()->Set_Render_Batch_Global_Color(Color::White);
		RenderPipeline::Get()->Draw_Game(time);
		Renderer::Get()->Set_Depth_Test(false);
	}
	else
	{
#ifdef OPT_FORCE_CAMERA_COUNT
		int count = OPT_FORCE_CAMERA_COUNT;
#else
		int count = Game::Get()->Get_Game_Mode()->Get_Camera_Count();
#endif
		for (int i = 0; i < count; i++)
		{
			Camera* camera = Game::Get()->Get_Camera((CameraID::Type)(CameraID::Game1 + i));
			RenderPipeline::Get()->Set_Active_Camera(camera); 
			//RenderPipeline::Get()->Get_Active_Camera()->Set_Screen_Viewport(viewport);

#ifdef OPT_FORCE_CAMERA_COUNT
//			camera->Set_Screen_Viewport(Rect2D(0.0f, 0.0f, (float)GfxDisplay::Get()->Get_Width(), (float)GfxDisplay::Get()->Get_Height()));
#endif

			RenderPipeline::Get()->Get_Active_Camera()->Set_PostProcess_Index(i);

			Renderer::Get()->Set_Depth_Test(false);
			RenderPipeline::Get()->Set_Render_Batch_Global_Color(Color::White);
			RenderPipeline::Get()->Draw_Game(time);
			Renderer::Get()->Set_Depth_Test(false);
		}
	}

	// Annnnd now all the scene elements.
	RenderPipeline::Get()->Set_Active_Camera(Game::Get()->Get_Camera(CameraID::UI));
	RenderPipeline::Get()->Set_Render_Batch_Global_Color(batch_color);
}



