// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/UI/Elements/UITextBox.h"
#include "Engine/Renderer/Atlases/Atlas.h"
#include "Engine/Renderer/Atlases/AtlasHandle.h"
#include "Engine/Renderer/Atlases/AtlasRenderer.h"
#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Resources/ResourceFactory.h"
#include "Engine/UI/UIFrame.h"
#include "Engine/UI/UIManager.h"
#include "Engine/UI/UIScene.h"
#include "Engine/Engine/GameEngine.h"
#include "Engine/Input/Input.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Localise/Locale.h"
#include "Engine/Online/OnlinePlatform.h"
#include "Engine/Online/OnlineUser.h"
#include "Generic/Types/UTF8String.h"

#include "Generic/Math/Math.h"

UITextBox::UITextBox()
	: m_frame_color(Color::White)
	, m_draw_carrot(false)
	, m_carrot_timer(0.0f)
	, m_carrot_offset(0)
	, m_always_active(false)
	, m_max_length(-1)
	, m_controller_input_pending(false)
	, m_game_style(true)
	, m_draw_border(true)
	, m_was_entered(false)
	, m_font(NULL)
{
}

UITextBox::~UITextBox()
{
}

bool UITextBox::Was_Entered()
{
	return m_was_entered;
}

void UITextBox::Set_Draw_Border(bool value)
{
	m_draw_border = value;
}

void UITextBox::Refresh()
{
	// Calculate screen-space box.
	m_screen_box		= Calculate_Screen_Box();
	
	UIManager* manager	= GameEngine::Get()->Get_UIManager();
	m_atlas				= manager->Get_Atlas();
	m_atlas_renderer	= AtlasRenderer(m_atlas);
	m_font				= manager->Get_Font();
	m_font_renderer		= FontRenderer(m_font, false, true);
	
	// Grab all atlas frames.
	m_game_background_frame			= UIFrame("screen_main_input_inactive_#");
	m_game_background_active_frame	= UIFrame("screen_main_input_active_#");
	m_background_frame				= UIFrame("toolbaritem_background_#");
	m_background_active_frame		= UIFrame("toolbaritem_background_hover_#");
		
	// Get children to calculate their screen box.
	UIElement::Refresh();

	// Figure out padded area.
	float padding = 4;
	float viewport_padding = 1;
	Vector2 ui_scale = Get_Draw_Scale(manager);

	if (m_game_style == false)
		padding = 1;

	m_padded_box = Rect2D
	(
		m_screen_box.X + (padding * ui_scale.X),
		m_screen_box.Y + (padding * ui_scale.Y),
		m_screen_box.Width - ((padding * ui_scale.X) * 2),
		m_screen_box.Height - ((padding * ui_scale.Y) * 2)
	);
	m_viewport_box = Rect2D
	(
		m_screen_box.X + (viewport_padding * ui_scale.X),
		m_screen_box.Y + (viewport_padding * ui_scale.Y),
		m_screen_box.Width - ((viewport_padding * ui_scale.X) * 2),
		m_screen_box.Height - ((viewport_padding * ui_scale.Y) * 2)
	);

	// Update text offset.
	Text_Changed();
}

void UITextBox::After_Refresh()
{
	// After-refresh children.
	UIElement::After_Refresh();
}

void UITextBox::Focus()
{
	Input::Get()->Flush_Character_Stack();

	UTF8String utfstr = m_value;
	m_carrot_offset = utfstr.Length();
}

bool UITextBox::Is_Focusable()
{
	return true;
}

bool UITextBox::Focus_Left(UIManager* manager)
{
	if (m_carrot_offset == 0)
	{
		return UIElement::Focus_Left(manager);
	}
	else
	{
		m_carrot_offset = Max(0, m_carrot_offset - 1);
		Text_Changed();

		return true;
	}
}

bool UITextBox::Focus_Right(UIManager* manager)
{
	UTF8String utfstr = m_value;
	if (m_carrot_offset == utfstr.Length())
	{
		return UIElement::Focus_Right(manager);
	}
	else
	{
		m_carrot_offset = Min((int)utfstr.Length(), m_carrot_offset + 1);
		Text_Changed();

		return true;
	}
}

void UITextBox::Text_Changed()
{
	Vector2 ui_scale = Get_Draw_Scale(GameEngine::Get()->Get_UIManager());
	Vector2 font_scale = Vector2(ui_scale.X * 0.5f, ui_scale.Y * 0.5f);

	float right_text_padding = 2.0f * font_scale.X;

	if (m_game_style == true)
	{
		m_text_size = m_font_renderer.Calculate_String_Size((m_value + "_").c_str(), 16.0f * font_scale.Y);
		m_text_offset = Max(0.0f, m_text_size.X - (m_padded_box.Width - right_text_padding));
	}
	else
	{
		m_text_size = m_font_renderer.Calculate_String_Size((m_value + "_").c_str(), 8.0f);
		m_text_offset = Max(0.0f, m_text_size.X - (m_padded_box.Width - right_text_padding));
	}
}

void UITextBox::Set_Value(std::string value)
{
	m_value = value;

	UTF8String utfstr = m_value;
	m_carrot_offset = utfstr.Length();

	if (m_font != NULL)
		Text_Changed();
}

void UITextBox::Set_Max_Length(int len)
{
	m_max_length = len;
}

void UITextBox::Tick(const FrameTime& time, UIManager* manager, UIScene* scene)
{
	Input* input = Input::Get();

	m_was_entered = false;

	// TODO

	/*
	// Controller input pending?
	if (m_controller_input_pending)
	{
		if (!OnlinePlatform::Get()->Is_Dialog_Open())
		{
			std::string value = "";
			if (OnlinePlatform::Get()->Get_Gamepad_Text_Dialog_Result(value))
			{
				Set_Value(value);
				m_controller_input_pending = false;

				scene->Dispatch_Event(manager, UIEvent(UIEventType::TextBox_Enter, this));
			}
		}
		return;
	}
	*/

	// Update text.
	if (Is_Focused(manager))
	{
		if (!Is_Always_Focused)
		{
			// Is selecting with a gamepad, if so show a IME input dialog.
			bool bGamepadSelected = false;

			JoystickState* joystick = NULL;

			for (int i = 0; i < MAX_INPUT_SOURCES; i++)
			{
				Input* subinput = Input::Get(i);

				// Select by keyboard/joystick.
				InputBindings::Type joystick_select = subinput->Output_To_Active_Input(OutputBindings::GUI_Select, InputBindingType::Joystick);
				if (subinput->Get_Joystick_State()->Was_Key_Pressed(joystick_select) ||
					subinput->Get_Joystick_State()->Was_Action_Pressed(OutputBindings::GUI_Select))
				{
					joystick = subinput->Get_Joystick_State();
					bGamepadSelected = true;
					break;
				}
			}

			if (bGamepadSelected)
			{
				scene->Dispatch_Event(manager, UIEvent(UIEventType::TextBox_JoystickSelect, this, NULL, NULL, joystick));
			}
		}

		std::string in = input->Read_Character();
		if (in != "")
		{
			UTF8String utfstr = m_value;

			if (in == "\n" ||
				in == "\r")
			{
			//	manager->Play_UI_Sound(UISoundType::Select);
				scene->Dispatch_Event(manager, UIEvent(UIEventType::TextBox_Enter, this));
				m_was_entered = true;
			}
			else if (in == "\b")
			{
				if (m_value.size() > 0)
				{	
					if (m_carrot_offset > 0)
					{
						std::string result = "";
						
						if (m_carrot_offset > 0)
						{
							result += utfstr.Sub_String(0, m_carrot_offset - 1).C_Str();//m_value.substr(0, m_carrot_offset - 1);
						}
						if (m_carrot_offset < (int)utfstr.Length())
						{
							result += utfstr.Sub_String(m_carrot_offset).C_Str();//m_value.substr(m_carrot_offset);
						}

						m_carrot_offset--;
						
						manager->Play_UI_Sound(UISoundType::Character);
	
						m_value = result;
						Text_Changed();
					}
				}
			}
			else
			{			
				if (m_max_length == -1 || (int)utfstr.Length() < m_max_length)
				{
					manager->Play_UI_Sound(UISoundType::Character);

					std::string result = "";

					if (m_carrot_offset > 0)
					{
						result += utfstr.Sub_String(0, m_carrot_offset).C_Str();
					}

					result += in;

					if (m_carrot_offset < (int)utfstr.Length())
					{
						result += utfstr.Sub_String(m_carrot_offset).C_Str();
					}

					m_carrot_offset += 1;//in.size();

					m_value = result;
					Text_Changed();
				}
				else
				{
					manager->Play_UI_Sound(UISoundType::Back);
				}
			}
		}

		// Carrot blink timer.
		m_carrot_timer += time.Get_Frame_Time();
		if (m_carrot_timer > CARROT_BLINK_INTERVAL)
		{
			m_draw_carrot = !m_draw_carrot;
			m_carrot_timer = 0.0f;
		}
	}
	else
	{
		m_draw_carrot = false;
	}

	// Update children.
	UIElement::Tick(time, manager, scene);
}

void UITextBox::Draw(const FrameTime& time, UIManager* manager, UIScene* scene)
{
	Vector2 ui_scale = Get_Draw_Scale(manager);
	Vector2 font_scale = Vector2(ui_scale.X * 0.5f, ui_scale.Y * 0.5f);
	float font_size = 16.0f;
	Color font_color = Color::White;
	Vector2 text_pos = Vector2(m_padded_box.X - m_text_offset, m_padded_box.Y - (2 * ui_scale.X));

	Renderer* renderer			= Renderer::Get();
	RenderPipeline* pipeline	= RenderPipeline::Get();

	// Draw background frame.
	if (m_game_style)
	{
		if (m_draw_border)
		{
			if (m_always_active == true || Is_Focused(manager))
			{	
				m_game_background_active_frame.Draw_Frame(m_atlas_renderer, m_screen_box, ui_scale, m_frame_color);
			}
			else
			{
				m_game_background_frame.Draw_Frame(m_atlas_renderer, m_screen_box, ui_scale, m_frame_color);
			}
		}
	}
	else
	{
		if (m_draw_border)
		{
			if (m_always_active == true || Is_Focused(manager))
			{	
				m_background_active_frame.Draw_Frame(m_atlas_renderer, m_screen_box, Vector2(1.0f, 1.0f), m_frame_color);
			}
			else
			{
				m_background_frame.Draw_Frame(m_atlas_renderer, m_screen_box, Vector2(1.0f, 1.0f), m_frame_color);
			}
		}

		text_pos = Vector2(m_padded_box.X - m_text_offset, m_screen_box.Y + (m_screen_box.Height * 0.5f) - (6.0f));
		font_scale = Vector2(1.0f, 1.0f);
		font_size = 8.0f;
		font_color = Color::Black;
	}
	
	// Perform scissor testing.
	pipeline->Set_Render_Batch_Scissor_Test(true);
	pipeline->Set_Render_Batch_Scissor_Rectangle(m_viewport_box);

	// Draw the current text.
	UTF8String nocarrot = "";
	UTF8String carrot = "";

	UTF8String input = m_value;

	if (m_carrot_offset > 0)
	{
		nocarrot += input.Sub_String(0, m_carrot_offset);
		carrot += input.Sub_String(0, m_carrot_offset);
	}
	
	nocarrot += " ";
	carrot += "|";

	if (m_carrot_offset < (int)input.Length())
	{
		nocarrot += input.Sub_String(m_carrot_offset);
	}

	m_font_renderer.Set_Shadow(m_game_style);

	//m_font_renderer.Draw_String(S(nocarrot.C_Str()), text_pos, font_size, font_color, Vector2(0.0f, 0.0f), font_scale);
	m_font_renderer.Draw_String(nocarrot.C_Str(), text_pos, font_size, font_color, Vector2(0.0f, 0.0f), font_scale);

	if (m_draw_carrot == true)
	{
		m_font_renderer.Draw_String(carrot.C_Str(), text_pos, font_size, font_color, Vector2(0.0f, 0.0f), font_scale);
		//m_font_renderer.Draw_String(S(carrot.C_Str()), text_pos, font_size, font_color, Vector2(0.0f, 0.0f), font_scale);
	}

	// Stop performing scissor testing.
	pipeline->Set_Render_Batch_Scissor_Test(false);

	// Draw children.
	UIElement::Draw(time, manager, scene);
}
