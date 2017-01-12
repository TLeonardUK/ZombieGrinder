// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/UI/Transitions/UIFadeTopLevelTransition.h"
#include "Engine/UI/UIScene.h"
#include "Engine/UI/UIManager.h"

#include "Generic/Math/Math.h"

#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/RenderPipeline.h"

UIFadeTopLevelTransition::UIFadeTopLevelTransition()
	: m_timer(0)
{
}

UIFadeTopLevelTransition::~UIFadeTopLevelTransition()
{
}
	
const char* UIFadeTopLevelTransition::Get_Name()
{
	return "UIFadeTopLevelTransition";
}

bool UIFadeTopLevelTransition::Is_Complete()
{
	return m_timer >= FADE_INTERVAL;
}

float UIFadeTopLevelTransition::Get_Delta()
{
	return m_timer / FADE_INTERVAL;
}

void UIFadeTopLevelTransition::Tick(const FrameTime& time, UIManager* manager, UIScene* scene, int scene_index, bool is_current)
{
	m_timer += time.Get_Frame_Time();
	
	// EDIT: Don't tick transition scenes. We get into loops where we keep trying to change state. No
	//		 need to anyway.
	//scene->Tick(time, manager, scene_index);
}

void UIFadeTopLevelTransition::Draw(const FrameTime& time, UIManager* manager, UIScene* scene, int scene_index, bool is_current)
{
	RenderPipeline* renderer = RenderPipeline::Get();

	float delta				= Clamp(m_timer / float(FADE_INTERVAL), 0.0f, 1.0f);
	float fade_in_delta		= Max(delta - 0.5f, 0.0f) * 2.0f;
	float fade_out_delta	= Max(1.0f - (delta * 2.0f), 0.0f);

	UIScene* top_scene = manager->Get_Scene_From_Top(0);

	if (is_current)
	{	
		manager->Draw_Background(time);
		
		// Render scenes below top-scene full alpha.
		renderer->Set_Render_Batch_Global_Color(Color::White);
		UIScene* lower_scene = manager->Get_Scene_From_Top(1);
		lower_scene->Draw(time, manager, manager->Get_Top_Scene_Index() - 1);
	
		// Render top-level with fade alpha.
		renderer->Set_Render_Batch_Global_Color(Color(255.0f, 255.0f, 255.0f, 255.0f * (fade_out_delta)));
		top_scene->Set_Render_Lower_Scenes_Override(true, false);
		manager->Draw_Stack(time, false);
		top_scene->Set_Render_Lower_Scenes_Override(false, false);
	}
	else
	{
		// Render top-level with fade alpha.
		renderer->Set_Render_Batch_Global_Color(Color(255.0f, 255.0f, 255.0f, 255.0f * (fade_in_delta)));
		top_scene->Set_Render_Lower_Scenes_Override(true, false);
		manager->Draw_Stack(time, false);
		top_scene->Set_Render_Lower_Scenes_Override(false, false);
	}

	renderer->Set_Render_Batch_Global_Color(Color::White);
}

