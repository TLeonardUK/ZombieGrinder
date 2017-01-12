// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/UI/Transitions/UISlideInTransition.h"
#include "Engine/UI/UIScene.h"
#include "Engine/UI/UIManager.h"
#include "Engine/UI/UIElement.h"

#include "Engine/Display/GfxDisplay.h"

#include "Generic/Math/Math.h"

#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/RenderPipeline.h"

UISlideInTransition::UISlideInTransition(bool include_fade, bool do_not_fade_bg)
	: m_timer(0)
	, m_include_fade(include_fade)
	, m_do_not_fade_bg(do_not_fade_bg)
{
}

UISlideInTransition::~UISlideInTransition()
{
}
	
const char* UISlideInTransition::Get_Name()
{
	return "UISlideInTransition";
}

bool UISlideInTransition::Is_Complete()
{
	return m_timer >= SLIDE_IN_INTERVAL;
}

float UISlideInTransition::Get_Delta()
{
	return m_timer / SLIDE_IN_INTERVAL;
}

void UISlideInTransition::Tick(const FrameTime& time, UIManager* manager, UIScene* scene, int scene_index, bool is_current)
{
	m_timer += time.Get_Frame_Time();
	
	// EDIT: Don't tick transition scenes. We get into loops where we keep trying to change state. No
	//		 need to anyway.
	//scene->Tick(time, manager, scene_index);
}

Vector2 UISlideInTransition::Calculate_Element_Offset(UIElement* element, float delta)
{
	Rect2D original_box = element->Calculate_Screen_Box(true);

	// Get the size of the display.
	GfxDisplay* display = GfxDisplay::Get();
	float display_width = (float)display->Get_Width();
	float display_height = (float)display->Get_Height();

	// Calculate center of boxes.
	float center_x = original_box.X + (original_box.Width * 0.5f);
	float center_y = original_box.Y + (original_box.Height * 0.5f);

	// Work out minimum distance to a side of the screen.
	float min_distance_x = fabs(center_x);
	float min_distance_y = fabs(center_y);

	if (center_x > display_width * 0.5f) 
	{
		min_distance_x = display_width - min_distance_x;
	}
	
	if (center_y > display_height * 0.5f) 
	{
		min_distance_y = display_height - min_distance_y;
	}

	// Work out where to slide off too.
	float target_x = 0.0f;
	float target_y = 0.0f;

	// If distances are equal (aka, direct center, just fade).
	if (original_box.X == 0 && original_box.Y == 0 && original_box.Width == display_width && original_box.Height == display_height)
	{
		target_x = original_box.X;
		target_y = original_box.Y;
	}
	
	// Horizontal
	else if (fabs(min_distance_x) <= fabs(min_distance_y))
	{
		// Right
		if (center_x > display_width * 0.5f)
		{
			target_x = display_width + original_box.X;
			target_y = original_box.Y;
		}

		// Left
		else
		{
			target_x = original_box.X - display_width;
			target_y = original_box.Y;
		}
	}

	// Vertical
	else
	{
		// Down		
		if (center_y > display_height * 0.5f)
		{
			target_x = original_box.X;
			target_y = display_height + original_box.Y;
		}

		// Up
		else
		{
			target_x = original_box.X;
			target_y = original_box.Y - display_height;
		}
	}

	// Work out final offset.
	float offset_x = Math::SmoothStep(0, target_x - original_box.X, delta);
	float offset_y = Math::SmoothStep(0, target_y - original_box.Y, delta);

	return Vector2(offset_x, offset_y);
}

void UISlideInTransition::Draw(const FrameTime& time, UIManager* manager, UIScene* scene, int scene_index, bool is_current)
{
	RenderPipeline* renderer = RenderPipeline::Get();

	float delta				= Clamp(m_timer / float(SLIDE_IN_INTERVAL), 0.0f, 1.0f);
	float slide_in_delta	= 1.0f - delta;
	float slide_out_delta	= delta;
	
	float fade_in_delta		= Max(delta - 0.5f, 0.0f) * 2.0f;
	float fade_out_delta	= Max(1.0f - (delta * 4.0f), 0.0f);

	float fade_in_bg_delta	= delta;
	float fade_out_bg_delta	= 1.0f - delta;
	
	Color old_global_color = renderer->Get_Render_Batch_Global_Color();
	
	if (is_current)
	{
		for (std::vector<UIElement*>::iterator iter = scene->Get_Elements().begin(); iter != scene->Get_Elements().end(); iter++)
		{
			UIElement* element = *iter;
			element->Set_Offset(Calculate_Element_Offset(element, slide_out_delta));
		}
		
		if (m_include_fade == true && m_do_not_fade_bg == false)
		{
			renderer->Set_Render_Batch_Global_Color(Color(255.0f, 255.0f, 255.0f, 255.0f * fade_out_bg_delta));
		}
		else
		{
			renderer->Set_Render_Batch_Global_Color(Color(255.0f, 255.0f, 255.0f, 255.0f));
		}
		manager->Draw_Background(time);
		
		if (m_include_fade == true)
		{
			renderer->Set_Render_Batch_Global_Color(Color(255.0f, 255.0f, 255.0f, 255.0f * fade_out_delta));
		}
		else
		{
			renderer->Set_Render_Batch_Global_Color(Color(255.0f, 255.0f, 255.0f, 255.0f));
		}
		scene->Refresh_Elements();
		manager->Draw_Stack(time, false);
	}
	else
	{
		for (std::vector<UIElement*>::iterator iter = scene->Get_Elements().begin(); iter != scene->Get_Elements().end(); iter++)
		{
			UIElement* element = *iter;
			element->Set_Offset(Calculate_Element_Offset(element, slide_in_delta));
		}

		if (m_do_not_fade_bg == false)
		{
			if (m_include_fade == true && m_do_not_fade_bg == false)
			{	
				renderer->Set_Render_Batch_Global_Color(Color(255.0f, 255.0f, 255.0f, 255.0f * fade_in_bg_delta));
			}
			else
			{
				renderer->Set_Render_Batch_Global_Color(Color(255.0f, 255.0f, 255.0f, 255.0f));
			}
			manager->Draw_Background(time);
		}
		
		if (m_include_fade == true)
		{
			renderer->Set_Render_Batch_Global_Color(Color(255.0f, 255.0f, 255.0f, 255.0f * fade_in_delta));
		}
		else
		{
			renderer->Set_Render_Batch_Global_Color(Color(255.0f, 255.0f, 255.0f, 255.0f));
		}
		scene->Refresh_Elements();
		manager->Draw_Stack(time, false);
	}

	renderer->Set_Render_Batch_Global_Color(old_global_color);
}

