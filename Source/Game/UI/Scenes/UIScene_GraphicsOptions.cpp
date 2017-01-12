// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_GraphicsOptions.h"
#include "Game/UI/Scenes/UIScene_AdvancedGraphicsOptions.h"
#include "Engine/UI/Transitions/UISlideInTransition.h"
#include "Engine/UI/Layouts/UILayoutFactory.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Display/GfxDisplay.h"
#include "Engine/Input/Input.h"
#include "Engine/UI/UIManager.h"
#include "Engine/UI/Elements/UIPanel.h"
#include "Engine/UI/Elements/UIComboBox.h"
#include "Engine/UI/Elements/UISlider.h"
#include "Engine/UI/Elements/UIProgressBar.h"
#include "Engine/UI/Elements/UILabel.h"
#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Resources/ResourceFactory.h"

#include "Engine/Localise/Locale.h"

#include "Game/UI/Scenes/UIScene_Game.h"

#include "Game/Runner/Game.h"
#include "Engine/Engine/EngineOptions.h"
#include "Game/Runner/GameOptions.h"

UIScene_GraphicsOptions::UIScene_GraphicsOptions()
{
	Set_Layout("graphics_options");	
}

const char* UIScene_GraphicsOptions::Get_Name()
{
	return "UIScene_GraphicsOptions";
}

bool UIScene_GraphicsOptions::Should_Render_Lower_Scenes()
{
	return false;
}

bool UIScene_GraphicsOptions::Should_Display_Cursor()
{
	return true;
}

bool UIScene_GraphicsOptions::Is_Focusable()
{
	return true;
}

UIScene* UIScene_GraphicsOptions::Get_Background(UIManager* manager)
{
	if (manager->Get_Scene_By_Type<UIScene_Game*>() != NULL)
		return UIScene::Get_Background(manager);
	else
		return manager->Retrieve_Persistent_Scene("UIScene_MenuBackgroundSimple");
}	

void UIScene_GraphicsOptions::Enter(UIManager* manager)
{
	UIComboBox* resolution_box	= Find_Element<UIComboBox*>("resolution_box");
	UIComboBox* mode_box		= Find_Element<UIComboBox*>("mode_box");
	UISlider*   gib_slider		= Find_Element<UISlider*>("gib_slider");

	std::vector<GfxDisplayResolution> reses = GfxDisplay::Get()->Get_Available_Resolutions();
	for (std::vector<GfxDisplayResolution>::iterator iter = reses.begin(); iter != reses.end(); iter++)
	{
		GfxDisplayResolution& res = *iter;

		UIComboBoxItem item;
		item.Text	   = StringHelper::Format("%i x %i @ %i", res.Width, res.Height, res.Hertz);
		item.MetaData2 = res.Width;
		item.MetaData3 = res.Height;
		item.MetaData4 = res.Hertz;

		resolution_box->Add_Item(item);

		if (res.Width == *EngineOptions::render_display_width &&
			res.Height == *EngineOptions::render_display_height &&
			res.Hertz == *EngineOptions::render_display_hertz)
		{
			resolution_box->Set_Selected_Item_Index(resolution_box->Get_Items().size() - 1);
		}
	}

	mode_box->Add_Item("Fullscreen");
	mode_box->Add_Item("Fullscreen Windowed");
	mode_box->Add_Item("Windowed");
	mode_box->Set_Selected_Item_Index(*EngineOptions::render_display_mode);

	gib_slider->Set_Progress(*GameOptions::graphics_quality);
}	

void UIScene_GraphicsOptions::Exit(UIManager* manager)
{
	UIComboBox* resolution_box = Find_Element<UIComboBox*>("resolution_box");
	UIComboBox* mode_box = Find_Element<UIComboBox*>("mode_box");
	UISlider* gib_slider = Find_Element<UISlider*>("gib_slider");
	
	UIComboBoxItem resolution_item			= resolution_box->Get_Selected_Item();
	*EngineOptions::render_display_width	= resolution_item.MetaData2;
	*EngineOptions::render_display_height	= resolution_item.MetaData3;
	*EngineOptions::render_display_hertz	= resolution_item.MetaData4; 

	*EngineOptions::render_display_mode		= (GfxDisplayMode::Type)mode_box->Get_Selected_Item_Index();
	*GameOptions::graphics_quality			= gib_slider->Get_Progress();
}	

void UIScene_GraphicsOptions::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Tick(time, manager, scene_index);
}

void UIScene_GraphicsOptions::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Draw(time, manager, scene_index);
}

void UIScene_GraphicsOptions::Recieve_Event(UIManager* manager, UIEvent e)
{
	switch (e.Type)
	{
	case UIEventType::Element_Focus:
		{
			if (e.Source->Get_Name() == "resolution_box")
			{
				Find_Element<UILabel*>("description_label")->Set_Value(S("#menu_graphics_resolution_description"));
			}
			else if (e.Source->Get_Name() == "mode_box")
			{
				Find_Element<UILabel*>("description_label")->Set_Value(S("#menu_graphics_mode_description"));
			}
			else if (e.Source->Get_Name() == "gib_slider")
			{
				Find_Element<UILabel*>("description_label")->Set_Value(S("#menu_graphics_quality_description"));
			}
			else if (e.Source->Get_Name() == "advanced_button")
			{
				Find_Element<UILabel*>("description_label")->Set_Value(S("#menu_graphics_advanced_description"));
			}
			else if (e.Source->Get_Name() == "back_button")
			{
				Find_Element<UILabel*>("description_label")->Set_Value(S("#menu_back_description"));
			}
		}
		break;
	case UIEventType::Button_Click:
		{	
			if (e.Source->Get_Name() == "advanced_button")
			{
				manager->Go(UIAction::Push(new UIScene_AdvancedGraphicsOptions(), new UISlideInTransition()));
			}
			else if (e.Source->Get_Name() == "back_button")
			{
				manager->Go(UIAction::Pop(new UISlideInTransition()));
			}
		}
		break;
	}
}
