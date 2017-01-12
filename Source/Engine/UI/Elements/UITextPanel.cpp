// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/UI/Elements/UITextPanel.h"
#include "Engine/Renderer/Atlases/Atlas.h"
#include "Engine/Renderer/Atlases/AtlasHandle.h"
#include "Engine/Renderer/Atlases/AtlasRenderer.h"
#include "Engine/Resources/ResourceFactory.h"
#include "Engine/UI/UIFrame.h"
#include "Engine/UI/UIManager.h"
#include "Engine/UI/UIScene.h"
#include "Engine/Engine/GameEngine.h"
#include "Engine/Input/Input.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Localise/Locale.h"

#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/RenderPipeline.h"

#include "Generic/Math/Math.h"

UITextPanel::UITextPanel()
	: m_slider(NULL)
	, m_draw_text_only(false)
	, m_shadowed_text(false)
	, m_override_global_alpha(false)
{
}

UITextPanel::~UITextPanel()
{
}

void UITextPanel::RefreshBoxes(UIManager* manager)
{
	Vector2 ui_scale				= Get_Draw_Scale(manager);
	float padding				= 4;
	float slider_size			= 8.0f;
	float slider_padding		= 8.0f;
	
	m_padded_box = Rect2D
	(
		m_screen_box.X + (padding * ui_scale.X),
		m_screen_box.Y + (padding * ui_scale.Y),
		m_screen_box.Width - ((padding * 2.0f) * ui_scale.X),
		m_screen_box.Height - ((padding * 2.0f) * ui_scale.Y)
	);
	
	m_slider_box = Rect2D
	(
		m_padded_box.X + m_padded_box.Width - (slider_size * ui_scale.X),
		m_padded_box.Y + (slider_padding * ui_scale.Y),
		(slider_size * ui_scale.X),
		m_padded_box.Height - ((slider_padding * 2.0f) * ui_scale.Y)
	);

	m_item_box = Rect2D
	(
		m_padded_box.X,
		m_padded_box.Y,
		m_padded_box.Width - m_slider_box.Width - (slider_size * ui_scale.X),
		m_padded_box.Height
	);

	// Update slider box.	
	if (m_slider != NULL)
	{
		m_slider->Set_Box(Rect2D(m_slider_box.X - m_screen_box.X, m_slider_box.Y - m_screen_box.Y, m_slider_box.Width, m_slider_box.Height));
	}
}

void UITextPanel::Refresh()
{
	UIManager* manager	= GameEngine::Get()->Get_UIManager();

	// Calculate screen-space box.
	m_screen_box		= Calculate_Screen_Box();
	
	m_atlas					= manager->Get_Atlas();
	m_atlas_renderer		= AtlasRenderer(m_atlas);
	m_font					= manager->Get_Font();
	m_font_renderer			= FontRenderer(m_font, false, m_shadowed_text);
	m_markup_font_renderer	= MarkupFontRenderer(m_font, false, m_shadowed_text, true);
	
	// Get children to calculate their screen box.
	UIElement::Refresh();
	RefreshBoxes(manager);

	// Add the slider.
	if (m_slider == NULL)
	{
		m_slider = new UISlider();
		m_slider->Set_Scene(m_scene);
		m_slider->Set_Parent(this);
		m_slider->Set_Box(Rect2D(m_slider_box.X - m_screen_box.X, m_slider_box.Y - m_screen_box.Y, m_slider_box.Width, m_slider_box.Height));
		m_slider->Set_Direction(UISliderDirection::Vertical);
		m_slider->Set_Focusable(false);
		m_slider->Set_Progress(1.0f);
		Add_Child(m_slider);
	}

	Text_Changed();

	UIElement::Refresh();
}

void UITextPanel::Set_Scroll_Offset(float offset)
{
	m_slider->Set_Progress(offset);
}

float UITextPanel::Get_Scroll_Offset()
{
	return m_slider->Get_Progress();
}

void UITextPanel::Set_Draw_Text_Only(bool value)
{
	m_draw_text_only = value;
}

void UITextPanel::Override_Global_Alpha(bool value)
{
	m_override_global_alpha = value;
}

void UITextPanel::After_Refresh()
{
	// After-refresh children.
	UIElement::After_Refresh();
}

bool UITextPanel::Is_Focusable()
{
	return false;
}

void UITextPanel::Text_Changed()
{
	UIManager* manager = GameEngine::Get()->Get_UIManager();
	Vector2 ui_scale = Get_Draw_Scale(manager);

	Vector2 font_scale = Vector2(ui_scale.X * 0.5f, ui_scale.Y * 0.5f);

	m_draw_value	= m_markup_font_renderer.Word_Wrap(m_value.c_str(), m_item_box, 16.0f, font_scale);
	m_line_height	= m_markup_font_renderer.Calculate_String_Size(" ", 16.0f, font_scale).Y;

	m_lines.clear();
	StringHelper::Split(m_draw_value.c_str(), '\n', m_lines);

	m_text_size		= Vector2(m_item_box.Width, m_lines.size() * m_line_height);//m_markup_font_renderer.Calculate_String_Size(m_draw_value.c_str(), 16.0f, font_scale);
	m_scroll_range	= Max(0, m_text_size.Y - m_item_box.Height); 
}
	
void UITextPanel::Append_Text(std::string text)
{
	if (m_value != "")
	{
		m_value += "\n";
	}
	m_value += text;
	Text_Changed();
}

void UITextPanel::Clear_Text()
{
	m_value = "";
	Text_Changed();
}

void UITextPanel::Set_Text(std::string text)
{
	m_value = text;
	Text_Changed();
}

void UITextPanel::Tick(const FrameTime& time, UIManager* manager, UIScene* scene)
{
	// Update children.
	UIElement::Tick(time, manager, scene);
}

void UITextPanel::Draw(const FrameTime& time, UIManager* manager, UIScene* scene)
{
	Input* input				= Input::Get();
	MouseState* mouse			= input->Get_Mouse_State();
	Vector2 mouse_position		= mouse->Get_Position();
	
	Vector2 ui_scale = Get_Draw_Scale(manager);
	float item_height			= 12.0f;

	Vector2 font_scale			= Vector2(ui_scale.X * 0.5f, ui_scale.Y * 0.5f);
	
	Renderer* renderer			= Renderer::Get();
	RenderPipeline* pipeline	= RenderPipeline::Get();

	float padding = 2.0f;
	Rect2D viewport(m_item_box.X - padding, m_item_box.Y - padding, m_item_box.Width + (padding * 2), m_item_box.Height + (padding * 2));

	pipeline->Flush_Batches();

	// Perform scissor testing.
	pipeline->Set_Render_Batch_Scissor_Test(true);
	pipeline->Set_Render_Batch_Scissor_Rectangle(viewport);

	Color original_color = pipeline->Get_Render_Batch_Global_Color();
	if (m_override_global_alpha == true)
	{
		pipeline->Set_Render_Batch_Global_Color(Color::White);
	}

	m_markup_font_renderer.Clear_State();

	// Render the text box.
	float y_offset = (m_scroll_range * m_slider->Get_Progress());
	//y_offset = int(y_offset / m_lines.size()) * m_lines.size();

	Vector2 offset = Vector2(m_item_box.X, m_item_box.Y - y_offset);
	FontRenderState render_state = m_markup_font_renderer.Begin_Draw(offset);

	int start_index = (int)(y_offset / (float)m_line_height);
	int end_index = start_index + (int)ceilf(viewport.Height / m_line_height);
	int index = 0;

	if (m_start_line_prefix != "")
	{
		while (start_index > 0)
		{
			std::string& line = m_lines[start_index];
			if (line.find(m_start_line_prefix) == 0)
			{
				break;
			}
			else
			{
				start_index--;
			}
		}
	}

	// Always start at a message-beginning line.

	for (std::vector<std::string>::iterator iter = m_lines.begin(); iter != m_lines.end(); iter++, index++)
	{
		std::string& line = *iter;

		if (index >= start_index && index <= end_index)//(offset.Y + m_line_height > viewport.Y && offset.Y < viewport.Y + viewport.Height)
		{
			render_state.Pen = offset;
			m_markup_font_renderer.Draw(render_state, (line + "\n").c_str(), offset, 16.0f, Color::White, Vector2(0.0f, 0.0f), font_scale);
		}

		offset.Y += m_line_height;
	}

	// Stop performing scissor testing.
	if (m_override_global_alpha == true)
	{
		pipeline->Set_Render_Batch_Global_Color(original_color);
	}
	pipeline->Set_Render_Batch_Scissor_Test(false);

	// Draw children.
	if (m_draw_text_only == false)
	{
		UIElement::Draw(time, manager, scene);
	}
}
