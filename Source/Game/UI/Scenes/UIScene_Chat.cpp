// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_Chat.h"
#include "Engine/UI/Transitions/UISlideInTransition.h"
#include "Engine/UI/Transitions/UIFadeOutTransition.h"
#include "Engine/UI/Layouts/UILayoutFactory.h"
#include "Engine/UI/Elements/UIPanel.h"
#include "Engine/UI/Elements/UITextPanel.h"

#include "Engine/Online/OnlineVoiceChat.h"

#include "Engine/Platform/Platform.h"
#include "Engine/Display/GfxDisplay.h"

#include "Engine/Engine/GameEngine.h"
#include "Engine/Scene/Scene.h"

#include "Engine/Input/Input.h"

#include "Engine/UI/UIManager.h"

#include "Engine/Renderer/RenderPipeline.h"

#include "Game/UI/Scenes/Editor/UIScene_Editor.h"
#include "Game/UI/Scenes/UIScene_Game.h"

#include "Game/Game/ChatManager.h"

#include "Game/Runner/GameOptions.h"
#include "Game/Runner/Game.h"

UIScene_Chat::UIScene_Chat(UIScene_Game* game, bool as_overlay)
	: m_game_scene(game)
	, m_as_overlay(as_overlay)
	, m_last_chat_messages_recieved(-1)
{
	Set_Layout("Chat");

	if (!as_overlay)
	{
		Game::Get()->Set_Game_Input_Enabled(false);
	}
}

UIScene_Chat::~UIScene_Chat()
{
}

bool UIScene_Chat::Should_Tick_When_Not_Top()
{
	return false;
}

const char* UIScene_Chat::Get_Name()
{
	return "UIScene_Chat";
}

bool UIScene_Chat::Should_Render_Lower_Scenes()
{
	return !m_as_overlay;
}

bool UIScene_Chat::Should_Render_Lower_Scenes_Background()
{
	return true;
}

bool UIScene_Chat::Is_Focusable()
{
	return true;
}

bool UIScene_Chat::Should_Display_Cursor()
{
	return true;
}

bool UIScene_Chat::Should_Display_Focus_Cursor()
{
	return false;
}

void UIScene_Chat::Enter(UIManager* manager)
{
	Refresh_History();
	Set_Offset(m_game_scene->Get_Chat_Overlay()->Get_Offset());
}	

void UIScene_Chat::Exit(UIManager* manager)
{
}	

void UIScene_Chat::Refresh(UIManager* manager)
{
	UIScene::Refresh(manager);
	m_chat_panel = Find_Element<UITextPanel*>("chat_history");
	m_chat_panel->Set_Start_Line_Prefix("[fade=");
}

void UIScene_Chat::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	if (m_as_overlay == false)
	{
		// Close sub menu?
		if (Input::Get()->Was_Pressed_By_Any(OutputBindings::GUI_Back) ||
			Input::Get()->Was_Pressed_By_Any(OutputBindings::Escape))
		{
			Game::Get()->Set_Game_Input_Enabled(true);
			manager->Play_UI_Sound(UISoundType::Small_Click);
			manager->Go(UIAction::Pop(new UIFadeOutTransition()));
			return;
		}
	}
	else
	{
		m_game_scene->Get_Chat_Overlay()->Set_Offset(Get_Offset());
	}

	// Recieved messages?
	if (ChatManager::Get()->Get_Messages_Recieved() != m_last_chat_messages_recieved)
	{
		Refresh_History();
		Set_Offset(1.0f);
		m_game_scene->Get_Chat_Overlay()->Set_Offset(1.0f);
	}

	UIScene::Tick(time, manager, scene_index);
}

void UIScene_Chat::Refresh_History()
{
	ChatManager* manager = ChatManager::Get();

	std::string text = "";
	std::vector<ChatMessage>& messages = manager->Get_History();
	int index = 0;

	for (std::vector<ChatMessage>::iterator iter = messages.begin(); iter != messages.end(); iter++, index++)
	{
		ChatMessage& chat = *iter;
		if (m_as_overlay == true)
		{
			text += StringHelper::Format("[fade=%i,%i,%i]", (int)chat.Recieve_Time, (int)*GameOptions::chat_fade_delay, (int)*GameOptions::chat_fade_duration) + chat.Display_String + "[/fade]";
		}
		else
		{
			text += chat.Display_String;
		}

		if (index < (int)messages.size() - 1)
		{
			text += "\n";
		}
	}

	m_chat_panel->Set_Text(text);
	m_last_chat_messages_recieved = manager->Get_Messages_Recieved();
}

void UIScene_Chat::Set_Offset(float offset)
{
	if (m_chat_panel != NULL)
	{
		m_chat_panel->Set_Scroll_Offset(offset);
	}
}

float UIScene_Chat::Get_Offset()
{
	if (m_chat_panel != NULL)
	{
		return m_chat_panel->Get_Scroll_Offset();
	}
	return 0.0f;
}

void UIScene_Chat::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
#ifdef OPT_DISABLE_HUD_RENDERING
	return;
#endif

	// If we are rendering as an overlay, literally all we want to render is the text panel.
	if (m_as_overlay == true)
	{
		m_chat_panel->Set_Draw_Text_Only(true);
		m_chat_panel->Override_Global_Alpha(true);
		m_chat_panel->Draw(time, manager, this);
	}
	else
	{
		m_chat_panel->Override_Global_Alpha(false);
		UIScene::Draw(time, manager, scene_index);

		// Draw overlay text.
		if (manager->Transition_In_Progress())
		{
			m_game_scene->Get_Chat_Overlay()->Draw(time, manager, scene_index);
		}
	}
}

void UIScene_Chat::Recieve_Event(UIManager* manager, UIEvent e)
{
	if (m_as_overlay == false)
	{
		switch (e.Type)
		{
		case UIEventType::TextBox_Enter:
			{
				if (e.Source->Get_Name() == "chat_box")
				{
					std::string value = e.Source->Get_Value();
					if (value != "")
					{
						ChatManager::Get()->Parse(value);
					}

					Game::Get()->Set_Game_Input_Enabled(true);

					manager->Play_UI_Sound(UISoundType::Small_Click);
					manager->Go(UIAction::Pop(new UIFadeOutTransition()));
				}
			}
			break;
		}
	}
}