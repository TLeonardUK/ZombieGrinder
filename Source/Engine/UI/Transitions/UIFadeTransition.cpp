// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/UI/Transitions/UIFadeTransition.h"
#include "Engine/UI/UIScene.h"
#include "Engine/UI/UIManager.h"

#include "Generic/Math/Math.h"

#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/RenderPipeline.h"

UIFadeTransition::UIFadeTransition()
	: m_timer(0)
{
}

UIFadeTransition::~UIFadeTransition()
{
}
	
const char* UIFadeTransition::Get_Name()
{
	return "UIFadeTransition";
}

bool UIFadeTransition::Is_Complete()
{
	return m_timer >= FADE_INTERVAL;
}

float UIFadeTransition::Get_Delta()
{
	return m_timer / FADE_INTERVAL;
}

void UIFadeTransition::Tick(const FrameTime& time, UIManager* manager, UIScene* scene, int scene_index, bool is_current)
{
	m_timer += time.Get_Frame_Time();
	
	// EDIT: Don't tick transition scenes. We get into loops where we keep trying to change state. No
	//		 need to anyway.
	//scene->Tick(time, manager, scene_index);
}

void UIFadeTransition::Draw(const FrameTime& time, UIManager* manager, UIScene* scene, int scene_index, bool is_current)
{
	RenderPipeline* renderer = RenderPipeline::Get();

	float delta				= Clamp(m_timer / float(FADE_INTERVAL), 0.0f, 1.0f);
	float fade_in_delta		= Max(delta - 0.5f, 0.0f) * 2.0f;
	float fade_out_delta	= Max(1.0f - (delta * 2.0f), 0.0f);

	if (is_current)
	{
		renderer->Set_Render_Batch_Global_Color(Color(255.0f, 255.0f, 255.0f, 255.0f * fade_out_delta));
		manager->Draw_Stack(time);
	}
	else
	{
		renderer->Set_Render_Batch_Global_Color(Color(255.0f, 255.0f, 255.0f, 255.0f * fade_in_delta));
		manager->Draw_Stack(time);
	}

	renderer->Set_Render_Batch_Global_Color(Color::White);
}

