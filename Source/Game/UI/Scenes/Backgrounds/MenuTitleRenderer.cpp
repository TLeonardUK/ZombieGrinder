// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/Backgrounds/MenuTitleRenderer.h"

#include "Engine/Engine/GameEngine.h"

#include "Engine/Localise/Locale.h"

#include "Engine/Resources/ResourceFactory.h"

#include "Engine/Display/GfxDisplay.h"
#include "Generic/Math/Math.h"
#include "Generic/Math/Random.h"

void MenuBackgroundTitle::Tick(const FrameTime& time)
{
	m_timer -= time.Get_Frame_Time();

	if (m_timer <= 0)
	{
		m_active = false;
		return;
	}
}

void MenuBackgroundTitle::Deactivate()
{
	m_active = false;
}

void MenuBackgroundTitle::Activate(float delta)
{
	float offset = GfxDisplay::Get()->Get_Width() * 0.06f;

	m_x			= (float)Random::Static_Next_Double(-offset, offset);
	m_y			= (float)Random::Static_Next_Double(-offset, offset);
	m_timer		= 2000 * delta;
	m_interval	= 2000;
	m_active	= true;

	switch (Random::Static_Next(0, 4))
	{
		case 0: m_x = -offset; break;
		case 1: m_x =  offset; break;
		case 2: m_y = -offset; break;
		case 3: m_y =  offset; break;
	}
}

bool MenuBackgroundTitle::Is_Active()
{
	return m_active;
}

float MenuBackgroundTitle::Get_Delta()
{
	return 1.0f - (m_timer / m_interval);
}

float MenuBackgroundTitle::Get_X_Offset()
{
	return Math::SmoothStep(0, m_x, Get_Delta());
}

float MenuBackgroundTitle::Get_Y_Offset()
{
	return Math::SmoothStep(0, m_y, Get_Delta());
}

MenuTitleRenderer::MenuTitleRenderer()
	: m_menu_timer(0)
	, m_tagline_flicker_change_timer(0)
	, m_tagline_change_timer(0)
	, m_scale_timer(0)
	, m_scale_direction(false)
	, m_scale(1.0f)
	, m_resolution_scale(1.0f)
	, m_scalar(1.0f)
	, m_tagline(NULL)
	, m_ticked(false)
{
	Reset();
}

void MenuTitleRenderer::Reset()
{
	// Randomise all titles.
	for (int i = 0; i < TITLE_COUNT; i++)
	{
		float delta = float(i) / float(TITLE_COUNT);
		m_titles[i].Activate(delta);
	}

	// Grab the frames we care about.
	m_logo_frame = ResourceFactory::Get()->Get_Atlas_Frame("screen_main_title");
	m_tag_color  = Color(144, 204, 219, 255);

	// Load all tag lines.
	m_tagline_count = 0;
	while (true)
	{
		char name[32];
		sprintf(name, "#tagline_%i", m_tagline_count);

		const char* tagline = Locale::Get()->Get_String(name, true);
		if (tagline != NULL)
		{
			m_taglines.push_back(tagline);
			m_tagline_count++;
		}
		else
		{
			break;
		}
	}

	// No taglines defined? Put a dummy one in so we know.
	if (m_taglines.size() <= 0)
	{
		m_taglines.push_back("< !! NO TAGLINES DEFINED !! >");
		m_tagline_count++;
	}

	// Create some font renderers!
	m_font				= GameEngine::Get()->Get_UIManager()->Get_Font();
	m_font_renderer		= FontRenderer(m_font, false, true);

	// Get first tag-line.
	m_tagline_index = Random::Static_Next(0, m_tagline_count - 1);
	m_tagline = m_taglines.at(m_tagline_index);
}

void MenuTitleRenderer::Tick(const FrameTime& time)
{
	int display_width  = GfxDisplay::Get()->Get_Width();
	int display_height = GfxDisplay::Get()->Get_Height();

	// Produce a new title.
	m_menu_timer += time.Get_Frame_Time();
	if (m_menu_timer >= 150)
	{
		for (int i = 0; i < TITLE_COUNT; i++)
		{
			if (!m_titles[i]. Is_Active())
			{
				m_titles[i].Activate();
				m_menu_timer = 0.0f;
				break;
			}
		}
	}

	// Update the background titles.
	for (int i = 0; i < TITLE_COUNT; i++)
	{
		if (m_titles[i].Is_Active())
		{
			m_titles[i].Tick(time);
		}
	}
	
	// Flicker through new tag lines.
	m_tagline_flicker_change_timer -= time.Get_Frame_Time();
	if (m_tagline_flicker_change_timer > 0)
	{
		m_tagline_index = Random::Static_Next(0, m_tagline_count - 1);
		m_tagline = m_taglines.at(m_tagline_index);
	}

	m_tagline_change_timer -= time.Get_Frame_Time();
	if (m_tagline_change_timer < 0)
	{
		m_tagline_change_timer = 8000;
		m_tagline_flicker_change_timer = 500;
	}

	// Update scaling.
	m_resolution_scale = (display_height / 480.0f);
	m_scalar   = 0.055f * 2.0f * m_resolution_scale;
	float interval = 2000;
	float scale	   = 1.0f;

	m_scale_timer += time.Get_Frame_Time();
	if (m_scale_timer > interval)
	{
		m_scale_direction = !m_scale_direction;
		m_scale_timer = 0.0f;
	}

	if (m_scale_direction == true)
	{
		m_scale = (m_scale_timer / interval);
	}
	else
	{
		m_scale = 1.0f - (m_scale_timer / interval);
	}
}

void MenuTitleRenderer::Draw(const FrameTime& time, Vector2 logo_position, Vector2 tagline_position, bool render_tag, float rotation, float scale, float alpha)
{
	// Makes sure scale has been calculated at least once.
	if (m_ticked == false)
	{
		Tick(time);
		m_ticked = true;
	}

	float logo_x		= logo_position.X;
	float logo_y		= logo_position.Y;

	float tag_x			= tagline_position.X;
	float tag_y			= tagline_position.Y;

	Vector2 logo_scale	= Vector2(m_resolution_scale + (m_scalar * m_scale), m_resolution_scale + (m_scalar * m_scale)) * scale;

	// Render the background titles.
	for (int i = 0; i < TITLE_COUNT; i++)
	{
		if (m_titles[i].Is_Active())
		{
			float delta = m_titles[i].Get_Delta();
			float x		= logo_x + (m_titles[i].Get_X_Offset() * scale);
			float y		= logo_y + (m_titles[i].Get_Y_Offset() * scale);
			float tx	= tag_x  + ((m_titles[i].Get_X_Offset() * scale) * 0.5f);
			float ty	= tag_y  + ((m_titles[i].Get_Y_Offset() * scale) * 0.5f);

			m_atlas_renderer.Draw_Frame
			(
				m_logo_frame, 
				Vector2(x, y), 
				0.0f, 
				Color(255.0f, 255.0f, 255.0f, ((1.0f - delta) * 255) * alpha),
				false,
				false,
				RendererOption::E_Src_Alpha_One,
				logo_scale,
				rotation
			);

			if (render_tag == true)
			{
				// Render the tag-line.
				m_font_renderer.Draw_String
				(
					m_tagline, 
					Rect2D(tx, ty, 0.0f, 0.0f), 
					16.0f,
					Color(m_tag_color.R * 0.3f, m_tag_color.G * 0.3f, m_tag_color.B * 0.3f, ((1.0f - delta) * 255) * alpha), 
					TextAlignment::Center, 
					TextAlignment::Center, 
					logo_scale,
					RendererOption::E_Src_Alpha_One
				);
			}
		}
	}

	// Render the title.
	m_atlas_renderer.Draw_Frame
	(
		m_logo_frame, 
		Vector2(logo_x, logo_y), 
		0.0f,
		Color(255.0f, 255.0f, 255.0f, 255.0f*alpha),
		false,
		false,
		RendererOption::E_Src_Alpha_One_Minus_Src_Alpha,
		logo_scale,
		rotation
	);

	if (render_tag == true)
	{
		// Render the tag-line.
		m_font_renderer.Draw_String
		(
			m_tagline, 
			Rect2D(tag_x, tag_y, 0.0f, 0.0f), 
			16.0f,
			Color((float)m_tag_color.R, (float)m_tag_color.G, (float)m_tag_color.B, m_tag_color.A*alpha),
			TextAlignment::Center, 
			TextAlignment::Center, 
			logo_scale,
			RendererOption::E_Src_Alpha_One_Minus_Src_Alpha
		);
	}
}