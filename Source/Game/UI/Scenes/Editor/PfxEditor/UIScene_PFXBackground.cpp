// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/Editor/PfxEditor/UIScene_PFXBackground.h"
#include "Engine/UI/Layouts/UILayoutFactory.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Display/GfxDisplay.h"
#include "Engine/Input/Input.h"
#include "Engine/UI/UIManager.h"
#include "Engine/UI/Elements/UIPanel.h"
#include "Engine/UI/Elements/UILabel.h"
#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Resources/ResourceFactory.h"

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

UIScene_PFXBackground::UIScene_PFXBackground()
{
	Set_Layout("game_background");	

	m_background_frame = ResourceFactory::Get()->Get_Atlas_Frame("editor_background");
}

const char* UIScene_PFXBackground::Get_Name()
{
	return "UIScene_PFXBackground";
}

bool UIScene_PFXBackground::Should_Render_Lower_Scenes()
{
	return false;
}

bool UIScene_PFXBackground::Should_Tick_When_Not_Top()
{
	return true;
}

void UIScene_PFXBackground::Refresh(UIManager* manager)
{
	// Refresh base stuff.
	UIScene::Refresh(manager);
}

void UIScene_PFXBackground::Enter(UIManager* manager)
{
}	

void UIScene_PFXBackground::Exit(UIManager* manager)
{
}	

void UIScene_PFXBackground::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Tick(time, manager, scene_index);
}

void UIScene_PFXBackground::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{	
	/*
	Camera* cam = Game::Get()->Get_Camera(CameraID::Editor_Main);
	RenderPipeline::Get()->Set_Active_Camera(cam);

	Rect2D bounds = Rect2D(-128, -128, 256, 256);

	AtlasRenderer renderer;
	renderer.Tile_Frame(
		m_background_frame, 
		bounds, 
		-1000.0f, 
		Color::White,
		false, 
		false, 
		RendererOption::E_Src_Alpha_One_Minus_Src_Alpha, 
		Vector2(1.0f, 1.0f), 
		0.0f
		);

	RenderPipeline::Get()->Flush_Batches();
	RenderPipeline::Get()->Set_Active_Camera(Game::Get()->Get_Camera(CameraID::UI));
	*/

	/*
	Color batch_color = RenderPipeline::Get()->Get_Render_Batch_Global_Color();

	PrimitiveRenderer pr;

	Camera* cam = Game::Get()->Get_Camera(CameraID::Editor_Sub);
	Rect2D viewport = Rect2D(0.0f, 0.0f, (float)GfxDisplay::Get()->Get_Width(), (float)GfxDisplay::Get()->Get_Height());

	cam->Set_Screen_Viewport(viewport);
	RenderPipeline::Get()->Set_Active_Camera(cam);


	Renderer::Get()->Set_Depth_Test(false);
	RenderPipeline::Get()->Set_Render_Batch_Global_Color(Color::White);
	RenderPipeline::Get()->Flush_Batches();
	
	Rect2D bounds = Rect2D(-128, -128, 256, 256);

	AtlasRenderer renderer;
	renderer.Tile_Frame(
		m_background_frame, 
		bounds, 
		0.0f, 
		Color::White,
		false, 
		false, 
		RendererOption::E_Src_Alpha_One_Minus_Src_Alpha, 
		Vector2(1.0f, 1.0f), 
		0.0f
	);

	RenderPipeline::Get()->Draw_Game(time);
	Renderer::Get()->Set_Depth_Test(false);
	
	// Annnnd now all the scene elements.
	RenderPipeline::Get()->Flush_Batches();
	RenderPipeline::Get()->Set_Active_Camera(Game::Get()->Get_Camera(CameraID::UI));
	RenderPipeline::Get()->Set_Render_Batch_Global_Color(batch_color);*/
}



