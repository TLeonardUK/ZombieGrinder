// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_GlobalChat.h"
#include "Engine/UI/Transitions/UISlideInTransition.h"
#include "Engine/UI/Transitions/UIFadeInTransition.h"
#include "Engine/UI/Layouts/UILayoutFactory.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Display/GfxDisplay.h"
#include "Engine/Input/Input.h"
#include "Engine/UI/UIManager.h"
#include "Engine/UI/Elements/UIPanel.h"
#include "Engine/UI/Elements/UISlider.h"
#include "Engine/UI/Elements/UIListView.h"
#include "Engine/UI/Elements/UIProgressBar.h"
#include "Engine/UI/Elements/UICheckBox.h"
#include "Engine/UI/Elements/UITextBox.h"
#include "Engine/UI/Elements/UILabel.h"
#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Resources/ResourceFactory.h"

#include "Engine/Localise/Locale.h"

#include "Game/UI/Scenes/UIScene_Game.h"

#include "Game/Runner/Game.h"

UIScene_GlobalChat::UIScene_GlobalChat()
	: m_last_chat_messages_recieved(0)
	, m_messages_recieve_count(0)
{
	Set_Layout("global_chat");	
}

const char* UIScene_GlobalChat::Get_Name()
{
	return "UIScene_GlobalChat";
}

bool UIScene_GlobalChat::Should_Render_Lower_Scenes()
{
	return false;
}

bool UIScene_GlobalChat::Should_Display_Cursor()
{
	return true;
}

bool UIScene_GlobalChat::Is_Focusable()
{
	return true;
}

UIScene* UIScene_GlobalChat::Get_Background(UIManager* manager)
{
	if (manager->Get_Scene_By_Type<UIScene_Game*>() != NULL)
		return UIScene::Get_Background(manager);
	else
		return manager->Retrieve_Persistent_Scene("UIScene_MenuBackgroundSimple");
}	

void UIScene_GlobalChat::Enter(UIManager* manager)
{
	Find_Element<UICheckBox*>("pipe_in_checkbox")->Set_Checked(*GameOptions::pipe_global_chat);
}	

void UIScene_GlobalChat::Refresh(UIManager* manager)
{
	UIScene::Refresh(manager);
	m_chat_panel = Find_Element<UITextPanel*>("chat_history");
}

void UIScene_GlobalChat::Exit(UIManager* manager)
{
	*GameOptions::pipe_global_chat = Find_Element<UICheckBox*>("pipe_in_checkbox")->Get_Checked();
}	

void UIScene_GlobalChat::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	m_messages_recieve_count = OnlineGlobalChat::Get()->Get_Messages(m_messages);
	if (m_messages_recieve_count != m_last_chat_messages_recieved)
	{
		Refresh_History();
	}

	Find_Element<UILabel*>("online_label")->Set_Value(SF("#menu_global_chat_online_label", OnlineGlobalChat::Get()->Get_User_Count()));

	UIScene::Tick(time, manager, scene_index);
}

void UIScene_GlobalChat::Refresh_History()
{
	std::string text = "";
	int index = 0;

	for (std::vector<GlobalChatMessage>::iterator iter = m_messages.begin(); iter != m_messages.end(); iter++, index++)
	{
		GlobalChatMessage& chat = *iter;
		text += "[c=128,128,128,255]"+ MarkupFontRenderer::Escape(chat.Source) + "[/c]: " + MarkupFontRenderer::Escape(chat.Message);

		if (index < (int)m_messages.size() - 1)
		{
			text += "\n";
		}
	}

	m_chat_panel->Set_Text(text);
	m_last_chat_messages_recieved = m_messages_recieve_count;
}

void UIScene_GlobalChat::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Draw(time, manager, scene_index);
}

void UIScene_GlobalChat::Recieve_Event(UIManager* manager, UIEvent e)
{
	switch (e.Type)
	{
	case UIEventType::Button_Click:
		{	
			if (e.Source->Get_Name() == "back_button")
			{
				manager->Go(UIAction::Pop(new UISlideInTransition()));
			}
		}
		break;

	case UIEventType::TextBox_Enter:
		{
			if (e.Source->Get_Name() == "chat_box")
			{
				std::string value = e.Source->Get_Value();
				if (value != "")
				{
					GlobalChatMessage msg;
					msg.Message = value;
					msg.Source = "";
					msg.Type = GlobalChatMessageType::Chat_Message;
					OnlineGlobalChat::Get()->Send_Message(msg);
				}

				manager->Play_UI_Sound(UISoundType::Small_Click);
				static_cast<UITextBox*>(e.Source)->Set_Value("");
			}
		}
		break;
	}
}
