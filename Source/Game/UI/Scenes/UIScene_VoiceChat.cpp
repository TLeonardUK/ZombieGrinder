// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_VoiceChat.h"
#include "Engine/UI/Transitions/UISlideInTransition.h"
#include "Engine/UI/Transitions/UIFadeOutTransition.h"
#include "Engine/UI/Layouts/UILayoutFactory.h"
#include "Engine/UI/Elements/UIPanel.h"
#include "Engine/UI/Elements/UITextPanel.h"
#include "Engine/UI/Elements/UISimpleListView.h"

#include "Engine/Platform/Platform.h"
#include "Engine/Display/GfxDisplay.h"

#include "Engine/Engine/GameEngine.h"
#include "Engine/Scene/Scene.h"

#include "Engine/Input/Input.h"

#include "Engine/UI/UIManager.h"

#include "Engine/Renderer/RenderPipeline.h"

#include "Game/UI/Scenes/Editor/UIScene_Editor.h"
#include "Game/UI/Scenes/UIScene_Game.h"

#include "Engine/Network/NetUser.h"
#include "Game/Network/GameNetManager.h"
#include "Game/Network/GameNetUser.h"

#include "Game/Game/ChatManager.h"

#include "Game/Runner/GameOptions.h"
#include "Game/Runner/Game.h"

UIScene_VoiceChat::UIScene_VoiceChat()
{
	Set_Layout("VoiceChat");
}

UIScene_VoiceChat::~UIScene_VoiceChat()
{
}

bool UIScene_VoiceChat::Should_Tick_When_Not_Top()
{
	return false;
}

const char* UIScene_VoiceChat::Get_Name()
{
	return "UIScene_VoiceChat";
}

bool UIScene_VoiceChat::Should_Render_Lower_Scenes()
{
	return false;
}

bool UIScene_VoiceChat::Should_Render_Lower_Scenes_Background()
{
	return true;
}

bool UIScene_VoiceChat::Is_Focusable()
{
	return false;
}

bool UIScene_VoiceChat::Should_Display_Cursor()
{
	return false;
}

bool UIScene_VoiceChat::Should_Display_Focus_Cursor()
{
	return false;
}

void UIScene_VoiceChat::Enter(UIManager* manager)
{
	Update_List();
}	

void UIScene_VoiceChat::Exit(UIManager* manager)
{
}	

void UIScene_VoiceChat::Refresh(UIManager* manager)
{
	UIScene::Refresh(manager);

	UISimpleListView* list_view = Find_Element<UISimpleListView*>("list_view");
	
	// Clear existing columns/items.
	list_view->Clear_Columns();
	list_view->Clear_Items();
	
	list_view->Add_Column("", 0.1f, false, true); // Voice Chat
	list_view->Add_Column("", 0.1f, false, true); // Avatar
	list_view->Add_Column("", 0.8f, false, false); // Name
}

void UIScene_VoiceChat::Update_List()
{
	UISimpleListView* list_view = Find_Element<UISimpleListView*>("list_view");
	list_view->Clear_Items();

	std::vector<std::string> item_values;

	std::vector<NetUser*> users = NetManager::Get()->Get_Net_Users();
	for (std::vector<NetUser*>::iterator useriter = users.begin(); useriter != users.end(); useriter++)
	{
		GameNetUser* user = static_cast<GameNetUser*>(*useriter);
		if (user->Get_Online_User()->Is_Talking())
		{
			item_values.clear();

			item_values.push_back("[img=voice_chat_indicator_1,1.0,1.0,0,0]");
			item_values.push_back(StringHelper::Format("[avatar=%i,1,1,0,0]", user->Get_Online_User()->Get_ID()));
			item_values.push_back(user->Get_Username());
	
			list_view->Add_Item(item_values, NULL);
		}
	}
}

void UIScene_VoiceChat::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	Update_List();
	UIScene::Tick(time, manager, scene_index);
}

void UIScene_VoiceChat::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Draw(time, manager, scene_index);
}

void UIScene_VoiceChat::Recieve_Event(UIManager* manager, UIEvent e)
{
}