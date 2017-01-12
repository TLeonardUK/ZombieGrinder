// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/UI/Transitions/UIFadeInToNewTransition.h"
#include "Engine/UI/UIScene.h"
#include "Engine/UI/UIManager.h"

#include "Generic/Math/Math.h"

#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/RenderPipeline.h"

UIFadeInToNewTransition::UIFadeInToNewTransition()
	: m_timer(0)
{
}

UIFadeInToNewTransition::~UIFadeInToNewTransition()
{
}
	
const char* UIFadeInToNewTransition::Get_Name()
{
	return "UIFadeInToNewTransition";
}

UITransitionDrawOrder::Type UIFadeInToNewTransition::Get_Draw_Order()
{
	return UITransitionDrawOrder::OldFirstOnPopElseNew;
}

bool UIFadeInToNewTransition::Is_Complete()
{
	return m_timer >= FADE_INTERVAL;
}

float UIFadeInToNewTransition::Get_Delta()
{
	return m_timer / FADE_INTERVAL;
}

void UIFadeInToNewTransition::Tick(const FrameTime& time, UIManager* manager, UIScene* scene, int scene_index, bool is_current)
{
	m_timer += time.Get_Frame_Time();
	
	// EDIT: Don't tick transition scenes. We get into loops where we keep trying to change state. No
	//		 need to anyway.
	//scene->Tick(time, manager, scene_index);
}

void UIFadeInToNewTransition::Draw(const FrameTime& time, UIManager* manager, UIScene* scene, int scene_index, bool is_current)
{
	RenderPipeline* renderer = RenderPipeline::Get();

	float delta	= Clamp(m_timer / float(FADE_INTERVAL), 0.0f, 1.0f);

	if (is_current)
	{
		UIScene* top_scene = manager->Get_Scene_From_Top(0);
		
		renderer->Set_Render_Batch_Global_Color(Color(255.0f, 255.0f, 255.0f, 255.0f));		
	//	top_scene->Set_Render_Lower_Scenes_Override(true, false);
		manager->Draw_Stack(time);
	//	top_scene->Set_Render_Lower_Scenes_Override(false, false);
	}
	else
	{
		renderer->Set_Render_Batch_Global_Color(Color(255.0f, 255.0f, 255.0f, 255.0f * delta));
		//renderer->Set_Render_Batch_Global_Color(Color(255.0f, 255.0f, 255.0f, 255.0f));		
		manager->Draw_Stack(time);
	}

	renderer->Set_Render_Batch_Global_Color(Color::White);
}

