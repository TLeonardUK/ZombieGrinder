// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_Credits.h"
#include "Game/UI/Scenes/UIScene_MainMenu.h"
#include "Game/UI/Scenes/UIScene_Options.h"
#include "Engine/UI/Transitions/UISlideInTransition.h"
#include "Engine/UI/Transitions/UIFadeTransition.h"
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

#include "Engine/Audio/AudioRenderer.h"

#include "Engine/Engine/GameEngine.h"

#include "Generic/Math/Math.h"

#include "Engine/Localise/Locale.h"

#include "Game/UI/Scenes/UIScene_Game.h"

#include "Game/Runner/Game.h"

UIScene_Credits::UIScene_Credits()
	: m_offset(0)
	, m_max_offset(0)
	, m_fade_out_timer(0)
{
	Set_Layout("credits");	
	AudioRenderer::Get()->Push_BGM("music_grindaa_das_zombi");

	m_font			= ResourceFactory::Get()->Get_Font("UI");
	m_font_renderer = MarkupFontRenderer(m_font);
	
	m_credits		= S("#credits");
	m_credits_size	= Vector2(0, 0);
	StringHelper::Split(m_credits.c_str(), '\n', m_credits_lines);
	
	Vector2 ui_scale = GameEngine::Get()->Get_UIManager()->Get_UI_Scale();
	m_text_scale = Vector2(ui_scale.X * 0.5f, ui_scale.Y * 0.5f);

	// Precalculate the sizes of all credit lines. Saves some rendering time :3
	for (std::vector<std::string>::iterator iter = m_credits_lines.begin(); iter != m_credits_lines.end(); iter++)
	{
		std::string line = *iter;

		// Strip starting tag saying what the line is.
		if (line.size() > 0 && line.at(0) == '{')
		{
			size_t offset = line.find('}');
			if (offset != std::string::npos)
			{
				line = line.substr(offset + 1);
			}
		}
		
		Vector2 line_size = m_font_renderer.Calculate_String_Size(line.c_str(), 16.0f * m_text_scale.Y);
		m_credits_lines_sizes.push_back(line_size);
	}	

	m_offset		= 0.0f;
	m_max_offset	= 0.0f;
}

UIScene_Credits::~UIScene_Credits()
{
	AudioRenderer::Get()->Pop_BGM();
}

const char* UIScene_Credits::Get_Name()
{
	return "UIScene_Credits";
}

bool UIScene_Credits::Should_Render_Lower_Scenes()
{
	return false;
}

bool UIScene_Credits::Should_Render_Lower_Scenes_Background()
{
	return false;
}

void UIScene_Credits::Enter(UIManager* manager)
{
	m_title_renderer.Reset();
}	

void UIScene_Credits::Exit(UIManager* manager)
{
}	

void UIScene_Credits::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	Vector2 ui_scale = manager->Get_UI_Scale();

	// Exit on escape.
	if (manager->Was_Pressed(OutputBindings::Escape) ||
		manager->Was_Pressed(OutputBindings::GUI_Select) ||
		manager->Was_Pressed(OutputBindings::GUI_Back))
	{	
		manager->Go(UIAction::Pop(new UIFadeTransition()));
		return;
	}

	// Calculate credits size.
//	m_credits_size = m_font_renderer.Calculate_String_Size(m_credits.c_str(), 16.0f);
//	if (m_max_offset == 0.0f)
//	{
//		m_max_offset = ((m_credits_size.Y * ui_scale.Y) - (GfxDisplay::Get()->Get_Height() * 1.5f) + (16.0f * ui_scale.Y));
//	}

	// Increment offset.
	float speed_scalar = time.Get_Delta_Seconds() * 75.0f;
	if (manager->Is_Down(OutputBindings::GUI_Down))
	{
		speed_scalar *= 0.25f;
		AudioRenderer::Get()->Get_Active_BGM_Channel()->Set_Rate(0.8f);
	}
	else if (manager->Is_Down(OutputBindings::GUI_Up))
	{
		speed_scalar *= 4.0f;
		AudioRenderer::Get()->Get_Active_BGM_Channel()->Set_Rate(1.2f);
	}
	else
	{	
		AudioRenderer::Get()->Get_Active_BGM_Channel()->Set_Rate(1.0f);
	}
	m_offset = Min(m_max_offset, m_offset + (1.0f * speed_scalar));
	
	// At end yet?
	if (m_offset == m_max_offset)
	{
		m_fade_out_timer += time.Get_Frame_Time();
		if (m_fade_out_timer >= FADE_OUT_DELAY)
		{
			manager->Go(UIAction::Pop(new UIFadeTransition()));
		}
	}

	// Tick the title renderer.
	m_title_renderer.Tick(time);

	UIScene::Tick(time, manager, scene_index);
}

void UIScene_Credits::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	float display_width = (float)GfxDisplay::Get()->Get_Width();
	float display_height = (float)GfxDisplay::Get()->Get_Height();

	int header_spacing = 6;
	int screen_border = 64;

	const char* header_tag = "{HEADER}";
	int header_tag_len = strlen(header_tag);

	const char* title_tag = "{TITLE}";
	int title_tag_len = strlen(title_tag);
	
	const char* center_tag = "{CENTER}";
	int center_tag_len = strlen(center_tag);

	// Draw the credits.
	float y = (display_height + (screen_border * 2)) - m_offset;

	//DBG_LOG("y=%f",y);

	int index = 0;
	for (std::vector<std::string>::iterator iter = m_credits_lines.begin(); iter != m_credits_lines.end(); iter++, index++)
	{
		std::string line = *iter;
		Vector2 line_size = m_credits_lines_sizes.at(index);

		bool on_screen = (y >= -screen_border && y <= display_height + screen_border);

		// Header?
		if (strncmp(line.c_str(), header_tag, header_tag_len) == 0)
		{
			if (on_screen == true)
			{
				line = line.c_str() + header_tag_len;

				float x = ((display_width * 0.5f) - header_spacing) - line_size.X;

				m_font_renderer.Draw_String(line.c_str(), Vector2(x, y), 16.0f, Color::White, Vector2(0, 0), m_text_scale);
			}
		}

		// Title?
		else if (strncmp(line.c_str(), title_tag, title_tag_len) == 0)
		{		
			line = line.c_str() + title_tag_len;

			m_title_renderer.Draw(time, Vector2(display_width*0.5f, y + 130), Vector2(0,0), false, 0, 1.0f);

			y += 280;
		}

		// Center?
		else if (strncmp(line.c_str(), center_tag, center_tag_len) == 0)
		{
			if (on_screen == true)
			{
				line = line.c_str() + center_tag_len;
			
				float x = (display_width * 0.5f) - (line_size.X * 0.5f);

				m_font_renderer.Draw_String(line.c_str(), Vector2(x, y), 16.0f, Color::White, Vector2(0, 0), m_text_scale);
			}

			y += 20.0f * m_text_scale.Y;
		}

		// Normal line.
		else
		{
			if (on_screen == true)
			{
				float x = (display_width * 0.5f) + header_spacing;
				m_font_renderer.Draw_String(line.c_str(), Vector2(x, y), 16.0f, Color::White, Vector2(0, 0), m_text_scale);
			}

			y += 20.0f * m_text_scale.Y;
		}
	}

	// Store maximum scroll height (saves pre-calcing it.
	float scroll_height = y - (display_height * 0.5f);
	if (scroll_height > m_max_offset)
	{
		m_max_offset = scroll_height;
	}

//	m_font_renderer.Draw_String(m_credits.c_str(), Rectangle(0.0f, GfxDisplay::Get()->Get_Height() - m_offset, display_width, m_credits_size.Y), Color::White, TextAlignment::Center, TextAlignment::Top,  Point(ui_scale.X * 0.5f, ui_scale.Y * 0.5f));

	// Draw the fade/overlay text.
	UIScene::Draw(time, manager, scene_index);
}



