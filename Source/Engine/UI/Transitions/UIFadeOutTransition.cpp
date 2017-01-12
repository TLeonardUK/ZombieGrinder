// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/UI/Transitions/UIFadeOutTransition.h"
#include "Engine/UI/UIScene.h"
#include "Engine/UI/UIManager.h"

#include "Generic/Math/Math.h"

#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/RenderPipeline.h"

UIFadeOutTransition::UIFadeOutTransition()
	: m_timer(0)
{
}

UIFadeOutTransition::~UIFadeOutTransition()
{
}
	
const char* UIFadeOutTransition::Get_Name()
{
	return "UIFadeOutTransition";
}

bool UIFadeOutTransition::Is_Complete()
{
	return m_timer >= FADE_INTERVAL;
}

float UIFadeOutTransition::Get_Delta()
{
	return m_timer / FADE_INTERVAL;
}

void UIFadeOutTransition::Tick(const FrameTime& time, UIManager* manager, UIScene* scene, int scene_index, bool is_current)
{
	m_timer += time.Get_Frame_Time();
	
	// EDIT: Don't tick transition scenes. We get into loops where we keep trying to change state. No
	//		 need to anyway.
	//scene->Tick(time, manager, scene_index);
}

void UIFadeOutTransition::Draw(const FrameTime& time, UIManager* manager, UIScene* scene, int scene_index, bool is_current)
{
	RenderPipeline* renderer = RenderPipeline::Get();

	float delta	= Clamp(m_timer / float(FADE_INTERVAL), 0.0f, 1.0f);

	if (is_current)
	{
		UIScene* top_scene = manager->Get_Scene_From_Top(0);

		renderer->Set_Render_Batch_Global_Color(Color(255.0f, 255.0f, 255.0f, 255.0f * (1.0f - delta)));
		top_scene->Set_Render_Lower_Scenes_Override(true, false);
		manager->Draw_Stack(time, false);
		top_scene->Set_Render_Lower_Scenes_Override(false, false);
	}
	else
	{
		renderer->Set_Render_Batch_Global_Color(Color(255.0f, 255.0f, 255.0f, 255.0f));		
		manager->Draw_Stack(time);
	}

	renderer->Set_Render_Batch_Global_Color(Color::White);
}

