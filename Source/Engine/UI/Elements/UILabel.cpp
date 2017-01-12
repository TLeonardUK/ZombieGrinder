// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/UI/Elements/UILabel.h"
#include "Engine/Renderer/Atlases/Atlas.h"
#include "Engine/Renderer/Atlases/AtlasHandle.h"
#include "Engine/Renderer/Atlases/AtlasRenderer.h"
#include "Engine/UI/UIManager.h"
#include "Engine/Engine/GameEngine.h"

#include "Engine/Renderer/Text/FontRenderer.h"
#include "Engine/Localise/Locale.h"

#include "Engine/Renderer/PrimitiveRenderer.h"

UILabel::UILabel()
	: m_halign(TextAlignment::Left)
	, m_valign(TextAlignment::Top)
	, m_color(Color::Black)
	, m_frame_name("")
	, m_frame_color(Color::White)
	, m_word_wrap(true)
	, m_word_wrap_in_date(false)
	, m_wrapped_text("")
	, m_game_style(true)
	, m_use_markup(false)
{
}

UILabel::~UILabel()
{
}
	
void UILabel::Refresh()
{
	m_manager				= GameEngine::Get()->Get_UIManager();
	m_atlas					= m_manager->Get_Atlas();
	m_screen_box			= Calculate_Screen_Box();
	m_atlas_renderer		= AtlasRenderer(m_atlas);
	m_font					= m_manager->Get_Font();
	m_font_renderer			= FontRenderer(m_font);
	m_markup_font_renderer	= MarkupFontRenderer(m_font);
	
	// Grab all atlas frames.
	if (m_frame_name != "")
	{
		m_frame = UIFrame(m_frame_name);
	}

	// Get children to calculate their screen box.
	UIElement::Refresh();
}

void UILabel::Tick(const FrameTime& time, UIManager* manager, UIScene* scene)
{
	// Update items.	
	UIElement::Tick(time, manager, scene);
}

void UILabel::Draw(const FrameTime& time, UIManager* manager, UIScene* scene)
{
	Vector2 ui_scale = Get_Draw_Scale(manager);
	Vector2 font_scale = ui_scale * 0.5f;
	float font_size = 16.0f;

	if (m_game_style == false)
	{
		ui_scale = Vector2(1, 1);
		font_scale = ui_scale * m_scale;
		font_size = 8.0f;
	}
	
	m_markup_font_renderer.Set_Shadow(m_game_style);
	m_font_renderer.Set_Shadow(m_game_style);

	// Draw frame.
	if (m_frame_name != "")
	{
		m_frame.Draw_Frame(m_atlas_renderer, m_screen_box, ui_scale, m_frame_color);
	}

	// Draw the text.
	const char* text = S(m_value);

	Rect2D text_box = Rect2D(m_screen_box.X + m_padding.X, m_screen_box.Y + m_padding.Y, m_screen_box.Width - m_padding.Width, m_screen_box.Height - m_padding.Height);

	if (m_use_markup == true)
	{
		std::string result = text;//m_font_renderer.Word_Wrap(text, text_box, ui_scale * 0.5f);
		if (m_word_wrap == true)
		{
			// Wrapping is expensive, avoid if we can.
			if (m_word_wrap_in_date == false)
			{
				Vector2 measure_scale = Vector2(font_scale);

				// Measure string size and scale font to fit it in.
				Vector2 string_size = m_font_renderer.Calculate_String_Size(text, font_size, measure_scale);

				// Scale text until it fits vertically.
				if (m_valign == TextAlignment::ScaleToFit)
				{
					float s = text_box.Height / string_size.Y;
					measure_scale = Vector2(s, s);
				}
				/*else
				{
					float s = Min(1.0f, text_box.Height / string_size.Y);
					if (s == 0.0f)
					{
						s = 1.0f;
					}
					measure_scale = Vector2(s, s);
				}*/

				m_wrapped_text = m_markup_font_renderer.Word_Wrap(text, text_box, font_size, measure_scale);
				m_word_wrap_in_date = true;
			}
			result = m_wrapped_text;
		}
		m_markup_font_renderer.Draw_String(result.c_str(), text_box, font_size, m_color, m_halign, m_valign, font_scale, RendererOption::E_Src_Alpha_One_Minus_Src_Alpha);
	}
	else
	{
		std::string result = text;
		if (m_word_wrap == true)
		{
			// Wrapping is expensive, avoid if we can.
			if (m_word_wrap_in_date == false)
			{
				Vector2 measure_scale = Vector2(font_scale);

				// Measure string size and scale font to fit it in.
				Vector2 string_size = m_font_renderer.Calculate_String_Size(text, font_size, measure_scale);

				// Scale text until it fits vertically.
				if (m_valign == TextAlignment::ScaleToFit)
				{
					float s = text_box.Height / string_size.Y;
					measure_scale = Vector2(s, s);
				}
				/*else
				{
					float s = Min(1.0f, text_box.Height / string_size.Y);
					if (s == 0.0f)
					{
						s = 1.0f;
					}
					measure_scale = Vector2(s, s);
				}*/

				m_wrapped_text = m_font_renderer.Word_Wrap(text, text_box, font_size, measure_scale);
				m_word_wrap_in_date = true;
			}
			result = m_wrapped_text;
		}
		m_font_renderer.Draw_String(result.c_str(), text_box, font_size, m_color, m_halign, m_valign, font_scale, RendererOption::E_Src_Alpha_One_Minus_Src_Alpha);
	}

	// Draw each menu item.
	UIElement::Draw(time, manager, scene);
}

