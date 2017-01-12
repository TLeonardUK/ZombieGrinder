// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_VoteSelectMap.h"
#include "Engine/UI/Transitions/UISlideInTransition.h"
#include "Engine/UI/Transitions/UISlideInTopLevelTransition.h"
#include "Engine/UI/Layouts/UILayoutFactory.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Display/GfxDisplay.h"
#include "Engine/Input/Input.h"
#include "Engine/UI/UIManager.h"
#include "Engine/UI/Elements/UIPanel.h"
#include "Engine/UI/Elements/UIComboBox.h"
#include "Engine/UI/Elements/UIPanel.h"
#include "Engine/UI/Elements/UICheckBox.h"
#include "Engine/UI/Elements/UISlider.h"
#include "Engine/UI/Elements/UIListView.h"
#include "Engine/UI/Elements/UIProgressBar.h"
#include "Engine/UI/Elements/UILabel.h"
#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Resources/ResourceFactory.h"

#include "Engine/Scene/Map/MapFileHandle.h"
#include "Engine/Scene/Map/MapFile.h"

#include "Game/Game/VoteManager.h"

#include "Engine/Audio/AudioRenderer.h"

#include "Engine/Localise/Locale.h"

#include "Game/Network/GameNetUser.h"
#include "Game/Network/GameNetManager.h"

#include "Game/UI/Scenes/UIScene_Game.h"

#include "Game/Runner/Game.h"
#include "Engine/Engine/EngineOptions.h"
#include "Game/Runner/GameOptions.h"

UIScene_VoteSelectMap::UIScene_VoteSelectMap(Vote* vote)
	: m_vote(vote)
{
	Set_Layout("vote_select_map");	
}

const char* UIScene_VoteSelectMap::Get_Name()
{
	return "UIScene_VoteSelectMap";
}

bool UIScene_VoteSelectMap::Should_Render_Lower_Scenes()
{
	return false;
}

bool UIScene_VoteSelectMap::Should_Display_Cursor()
{
	return true;
}

bool UIScene_VoteSelectMap::Is_Focusable()
{
	return true;
}

void UIScene_VoteSelectMap::Enter(UIManager* manager)
{
	UIComboBox* box = Find_Element<UIComboBox*>("map_box");
	std::vector<MapFileHandle*> files = ResourceFactory::Get()->Get_Sorted_Map_Files();

	box->Clear_Items();

	for (std::vector<MapFileHandle*>::iterator iter = files.begin(); iter != files.end(); iter++)
	{
		MapFileHandle* file = *iter;

		// We don't care about internal map files.
		if (file->Get()->Is_Internal())
		{
			continue;
		}

		box->Add_Item(file->Get()->Get_Header()->Short_Name.c_str(), file);
	}

	Refresh_Preview();
}	

void UIScene_VoteSelectMap::Refresh_Preview()
{
	UIComboBox* map_box = Find_Element<UIComboBox*>("map_box");
	UIPanel* map_panel = Find_Element<UIPanel*>("map_image_panel");
	UILabel* name_label = Find_Element<UILabel*>("map_name_label");
	UILabel* description_label = Find_Element<UILabel*>("map_description_label");

	MapFileHandle* handle = reinterpret_cast<MapFileHandle*>(map_box->Get_Selected_Item().MetaData1);

	name_label->Set_Value(S(handle->Get()->Get_Header()->Long_Name.c_str()));
	description_label->Set_Value(StringHelper::Replace(S(handle->Get()->Get_Header()->Description.c_str()), "\\n", "\n"));
	map_panel->Set_Background_Image(handle->Get()->Get_Preview_Image());
}

void UIScene_VoteSelectMap::Exit(UIManager* manager)
{
}	

void UIScene_VoteSelectMap::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Tick(time, manager, scene_index);
}

void UIScene_VoteSelectMap::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Draw(time, manager, scene_index);
}

void UIScene_VoteSelectMap::Recieve_Event(UIManager* manager, UIEvent e)
{
	switch (e.Type)
	{
	case UIEventType::ComboBox_SelectedIndexChanged:
		{	
			if (e.Source->Get_Name() == "map_box")
			{
				Refresh_Preview();
			}
		}
		break;
	case UIEventType::Button_Click:
		{	
			if (e.Source->Get_Name() == "ok_button")
			{				
				UIComboBox* box = Find_Element<UIComboBox*>("map_box");

				MapFileHandle* handle = reinterpret_cast<MapFileHandle*>(box->Get_Selected_Item().MetaData1);

				m_vote->Set_Extra(handle->Get()->Get_Header()->GUID.c_str());
				VoteManager::Get()->Call_Vote(m_vote);
			
				manager->Play_UI_Sound(UISoundType::Exit_SubMenu);
				manager->Go(UIAction::Pop(new UISlideInTransition(), "UIScene_Game"));
			}
			else if (e.Source->Get_Name() == "back_button")
			{
				SAFE_DELETE(m_vote);
				manager->Go(UIAction::Pop(new UISlideInTopLevelTransition()));
			}
		}
		break;
	}
}
