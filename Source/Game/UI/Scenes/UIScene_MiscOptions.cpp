// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_MiscOptions.h"
#include "Engine/UI/Transitions/UISlideInTransition.h"
#include "Engine/UI/Layouts/UILayoutFactory.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Display/GfxDisplay.h"
#include "Engine/Input/Input.h"
#include "Engine/UI/UIManager.h"
#include "Engine/UI/Elements/UIPanel.h"
#include "Engine/UI/Elements/UIComboBox.h"
#include "Engine/UI/Elements/UIProgressBar.h"
#include "Engine/UI/Elements/UILabel.h"
#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Resources/ResourceFactory.h"

#ifdef OPT_STEAM_PLATFORM
#include "Engine/Online/OnlinePlatform.h"
#include "Engine/Online/OnlineClient.h"
#include "Engine/Online/Steamworks/Steamworks_OnlinePlatform.h"
#include "Engine/Online/Steamworks/Steamworks_OnlineClient.h"
#endif

#include "Engine/Localise/Locale.h"
#include "Engine/Localise/Language.h"
#include "Engine/Localise/LanguageHandle.h"

#include "Game/UI/Scenes/UIScene_Game.h"

#include "Game/Runner/Game.h"
#include "Engine/Engine/EngineOptions.h"
#include "Game/Runner/GameOptions.h"

UIScene_MiscOptions::UIScene_MiscOptions()
{
	Set_Layout("misc_options");	

	UIComboBox* notification_box = Find_Element<UIComboBox*>("notification_box");
	UIComboBox* language_box = Find_Element<UIComboBox*>("language_box");

	// Keep in sync with ENotificationPosition in steamworks.
	notification_box->Add_Item("Top Left");
	notification_box->Add_Item("Top Right");	
	notification_box->Add_Item("Bottom Left");
	notification_box->Add_Item("Bottom Right");

#ifndef OPT_STEAM_PLATFORM
	notification_box->Set_Enabled(false);
#endif

	ThreadSafeHashTable<LanguageHandle*, unsigned int>& languages = ResourceFactory::Get()->Get_Languages();
	for (ThreadSafeHashTable<LanguageHandle*, unsigned int>::Iterator iter = languages.Begin(); iter != languages.End(); iter++)
	{
		LanguageHandle* language = iter.Get_Value();
		language_box->Add_Item(language->Get()->Long_Name, language);
	}
}

const char* UIScene_MiscOptions::Get_Name()
{
	return "UIScene_MiscOptions";
}

bool UIScene_MiscOptions::Should_Render_Lower_Scenes()
{
	return false;
}

bool UIScene_MiscOptions::Should_Display_Cursor()
{
	return true;
}

bool UIScene_MiscOptions::Is_Focusable()
{
	return true;
}

UIScene* UIScene_MiscOptions::Get_Background(UIManager* manager)
{
	if (manager->Get_Scene_By_Type<UIScene_Game*>() != NULL)
		return UIScene::Get_Background(manager);
	else
		return manager->Retrieve_Persistent_Scene("UIScene_MenuBackgroundSimple");
}	

void UIScene_MiscOptions::Enter(UIManager* manager)
{
#ifdef OPT_STEAM_PLATFORM
	Find_Element<UIComboBox*>("notification_box")->Set_Selected_Item_Index(*EngineOptions::steam_notification_corner);
#endif

	// Work out selected index.
	std::vector<UIComboBoxItem> items = Find_Element<UIComboBox*>("language_box")->Get_Items();
	int index = 0;
	for (std::vector<UIComboBoxItem>::iterator iter = items.begin(); iter != items.end(); iter++)
	{
		UIComboBoxItem& item = *iter;
		LanguageHandle* handle = reinterpret_cast<LanguageHandle*>(item.MetaData1);

		if (handle->Get()->Short_Name == *EngineOptions::language_default)
		{
			Find_Element<UIComboBox*>("language_box")->Set_Selected_Item_Index(index);
			break;
		}
		index++;
	}
}	

void UIScene_MiscOptions::Exit(UIManager* manager)
{	
#ifdef OPT_STEAM_PLATFORM
	*EngineOptions::steam_notification_corner = Find_Element<UIComboBox*>("notification_box")->Get_Selected_Item_Index();
#endif

	LanguageHandle* handle = reinterpret_cast<LanguageHandle*>(Find_Element<UIComboBox*>("language_box")->Get_Selected_Item().MetaData1);
	*EngineOptions::language_default = handle->Get()->Short_Name;
}	

void UIScene_MiscOptions::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Tick(time, manager, scene_index);
}

void UIScene_MiscOptions::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Draw(time, manager, scene_index);
}

void UIScene_MiscOptions::Recieve_Event(UIManager* manager, UIEvent e)
{
	switch (e.Type)
	{
	case UIEventType::ComboBox_SelectedIndexChanged:
		{
			if (e.Source->Get_Name() == "language_box")
			{
				UIComboBox* language = dynamic_cast<UIComboBox*>(e.Source);
				Locale::Get()->Change_Language(language->Get_Selected_Item().Text.c_str());
			}
#ifdef OPT_STEAM_PLATFORM
			else if (e.Source->Get_Name() == "notification_box")
			{
				UIComboBox* notification = dynamic_cast<UIComboBox*>(e.Source);
				static_cast<Steamworks_OnlineClient*>(OnlinePlatform::Get()->Client())->Set_Notification_Corner(notification->Get_Selected_Item_Index());
			}
#endif
		}
		break;
	case UIEventType::Element_Focus:
		{
		}
		break;
	case UIEventType::Button_Click:
		{	
			if (e.Source->Get_Name() == "back_button")
			{
				manager->Go(UIAction::Pop(new UISlideInTransition()));
			}
		}
		break;
	}
}
