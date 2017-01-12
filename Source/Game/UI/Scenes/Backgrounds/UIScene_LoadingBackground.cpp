// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/Backgrounds/UIScene_LoadingBackground.h"
#include "Engine/UI/Layouts/UILayoutFactory.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Display/GfxDisplay.h"
#include "Engine/Input/Input.h"
#include "Engine/UI/UIManager.h"
#include "Engine/UI/Elements/UIPanel.h"
#include "Engine/UI/Elements/UILabel.h"
#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Resources/ResourceFactory.h"

#include "Generic/Math/Math.h"
#include "Generic/Math/Random.h"

#include "Engine/Localise/Locale.h"

#include "Game/UI/Scenes/UIScene_Game.h"

#include "Game/Runner/Game.h"

Color UIScene_LoadingBackground::m_foreground_colors[MAX_STYLES] = 
{
	Color(0, 0, 0, 255),
	Color(0, 0, 0, 255),
	Color(0, 0, 0, 255),
	Color(0, 0, 0, 255)
};
 
Color UIScene_LoadingBackground::m_background_colors[MAX_STYLES] = 
{
	Color(51, 0, 0, 255),
	Color(12, 31, 58, 255),
	Color(0, 51, 51, 255),
	Color(40, 20, 0, 255)
};

UIScene_LoadingBackground::UIScene_LoadingBackground()
	: m_character_position(0, 0)
	, m_logo_position(0, 0)
	, m_style_index(0)
	, m_circle_radius_offset(0)
{
	Set_Layout("loading_background");

	m_title_renderer.Reset();
}

const char* UIScene_LoadingBackground::Get_Name()
{
	return "UIScene_LoadingBackground";
}

bool UIScene_LoadingBackground::Should_Render_Lower_Scenes()
{
	return false;
}

bool UIScene_LoadingBackground::Should_Tick_When_Not_Top()
{
	return true;
}

void UIScene_LoadingBackground::Set_Character_Position(float x, float y)
{
	m_character_position = Vector2(x, y);
}

void UIScene_LoadingBackground::Set_Logo_Position(float x, float y)
{
	m_logo_position = Vector2(x, y);
}

void UIScene_LoadingBackground::Set_Style_Index(int index)
{
	m_style_index = Min(MAX_STYLES - 1, Max(index, 0));
}

void UIScene_LoadingBackground::Refresh(UIManager* manager)
{
	for (int i = 0; i < MAX_STYLES; i++)
	{
		m_character_frames[i] = ResourceFactory::Get()->Get_Atlas_Frame(StringHelper::Format("screen_loading_person_%i", i + 1).c_str());
		DBG_ASSERT(m_character_frames[i] != NULL);
	}

	m_title_renderer.Reset();

	// Refresh base stuff.
	UIScene::Refresh(manager);
}

void UIScene_LoadingBackground::Enter(UIManager* manager)
{
	m_title_renderer.Reset();
}	

void UIScene_LoadingBackground::Exit(UIManager* manager)
{
}	

void UIScene_LoadingBackground::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	m_circle_radius_offset += time.Get_Delta() * 64.0f;

	UILabel* tip_label = Find_Element<UILabel*>("");

	UIScene::Tick(time, manager, scene_index);
}

void UIScene_LoadingBackground::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	float screen_width  = (float)GfxDisplay::Get()->Get_Width();
	float screen_height = (float)GfxDisplay::Get()->Get_Height();
	float resolution_scale = (screen_height / 480.0f);

	const float size_lerp = 0.8f;
	const float character_scale = 1.25f;

	PrimitiveRenderer prender;

	// Draw radiating cicles for background.
	float radius = (screen_width * 2.0f);
	float original_radius = radius;

	// Offset radius by our "scrolling offset".
	radius += m_circle_radius_offset;

	// Draw each circle of the background.
	for (int i = 0; i <= MAX_BACKROUND_OVALS; i++)
	{
		float circle_radius = radius;

		// Work out the containing rectangle of the oval.
		Rect2D circle_area = Rect2D
		(
			m_character_position.X - (circle_radius * 0.5f),
			m_character_position.Y - (circle_radius * 0.5f),
			circle_radius,
			circle_radius
		);

		// Draw the oval in alternating colors.
		if ((i % 2) == 0)
		{
			prender.Draw_Solid_Oval(circle_area, m_foreground_colors[m_style_index]);
		}
		else
		{
			prender.Draw_Solid_Oval(circle_area, m_background_colors[m_style_index]);
		}

		// Reset radius offset if we are now larger than the screen.
		if (i == 2)
		{
			if (circle_radius > original_radius)
			{
				m_circle_radius_offset = 0;
			}
		}

		// Shrink the circle for the next run.
		radius *= size_lerp;
	}
	
	// Draw base scene elements.
	UIScene::Draw(time, manager, scene_index);
	
	// Draw the character.
	m_atlas_renderer.Draw_Frame
	(
		m_character_frames[m_style_index], 
		Rect2D
		(
			m_character_position.X - (m_character_frames[m_style_index]->Rect.Width * resolution_scale * character_scale * 0.5f),
			m_character_position.Y - (m_character_frames[m_style_index]->Rect.Height * resolution_scale * character_scale * 0.5f),
			m_character_frames[m_style_index]->Rect.Width * resolution_scale * character_scale,
			m_character_frames[m_style_index]->Rect.Height * resolution_scale * character_scale
		), 
		0.0f,
		Color::White,
		false,
		false,
		RendererOption::E_Src_Alpha_One_Minus_Src_Alpha
	);

	// We always tick in the draw call
	// as we are a background and sometimes will not get ticked depending on the current
	// top-scene's settings.
	m_title_renderer.Tick(time);
	m_title_renderer.Draw(time, m_logo_position, Vector2(0, 0), false, 25.0f, 0.85f);
}



