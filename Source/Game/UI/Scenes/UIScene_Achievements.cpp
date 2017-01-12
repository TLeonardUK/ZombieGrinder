// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_Achievements.h"
#include "Engine/UI/Transitions/UISlideInTransition.h"
#include "Engine/UI/Transitions/UISlideInTopLevelTransition.h"
#include "Engine/UI/Layouts/UILayoutFactory.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Display/GfxDisplay.h"
#include "Engine/Input/Input.h"
#include "Engine/UI/UIManager.h"
#include "Engine/UI/Elements/UIPanel.h"
#include "Engine/UI/Elements/UIProgressBar.h"
#include "Engine/UI/Elements/UILabel.h"
#include "Engine/UI/Elements/UISlider.h"
#include "Engine/UI/Elements/UICheckBox.h"
#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Resources/ResourceFactory.h"

#include "Engine/Audio/AudioRenderer.h"

#include "Engine/Localise/Locale.h"

#include "Game/UI/Scenes/UIScene_Game.h"

#include "Game/Online/AchievementManager.h"

#include "Game/Runner/Game.h"
#include "Engine/Engine/EngineOptions.h"
#include "Game/Runner/GameOptions.h"

UIScene_Achievements::UIScene_Achievements()
{
	Set_Layout("achievements");	
	m_draw_item_delegate = new Delegate<UIScene_Achievements, UIListViewDrawItemData>(this, &UIScene_Achievements::On_Draw_Item);

	UIListView* listview = Find_Element<UIListView*>("achievement_listview");
	listview->On_Draw_Item += m_draw_item_delegate;
}

const char* UIScene_Achievements::Get_Name()
{
	return "UIScene_Achievements";
}

bool UIScene_Achievements::Should_Render_Lower_Scenes()
{
	return false;
}

bool UIScene_Achievements::Should_Display_Cursor()
{
	return true;
}

bool UIScene_Achievements::Is_Focusable()
{
	return true;
}

void UIScene_Achievements::Refresh_Items()
{	
	AchievementManager* achievement_manager = AchievementManager::Get();
	
	UIListView* listview = Find_Element<UIListView*>("achievement_listview");
	listview->Clear_Items();
	
	for (int i = 0; i < achievement_manager->Get_Achievement_Count(); i++)
	{
		Achievement* achievement = achievement_manager->Get_Achievement(i);
		listview->Add_Item("", achievement);
	}

}

void UIScene_Achievements::Enter(UIManager* manager)
{
	Refresh_Items();
}	

void UIScene_Achievements::Exit(UIManager* manager)
{
}	

void UIScene_Achievements::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Tick(time, manager, scene_index);
}

void UIScene_Achievements::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Draw(time, manager, scene_index);
}

void UIScene_Achievements::Refresh(UIManager* manager)
{
	m_atlas					= manager->Get_Atlas();
	m_atlas_renderer		= AtlasRenderer(m_atlas);

	m_font					= manager->Get_Font();
	m_font_renderer			= FontRenderer(m_font, false, false);
	m_shadow_font_renderer	= MarkupFontRenderer(m_font, false, true);

	m_item_frame_active = UIFrame("screen_main_item_border_active_#");
	m_item_frame_inactive = UIFrame("screen_main_item_border_inactive_#");

	UIScene::Refresh(manager);
}

void UIScene_Achievements::On_Draw_Item(UIListViewDrawItemData* data)
{
	PrimitiveRenderer prim_renderer;

	// Draw outline.
	if (data->selected)
		m_item_frame_active.Draw_Frame(m_atlas_renderer, data->item_bounds, data->ui_scale, Color::White);
	else if (data->hovering)
		m_item_frame_active.Draw_Frame(m_atlas_renderer, data->item_bounds, data->ui_scale, Color(128, 128, 128, 255));
	else
		m_item_frame_inactive.Draw_Frame(m_atlas_renderer, data->item_bounds, data->ui_scale, Color::White);

	// Create new-char or profile.
	UIListView* listview = Find_Element<UIListView*>("achievement_listview");
	Achievement* achievement = reinterpret_cast<Achievement*>(data->item->MetaData);

	const float spacing = 0.5f * data->ui_scale.Y;
	const float text_height = 9.5f * data->ui_scale.Y;
	const float avatar_spacing = 1.5f * data->ui_scale.Y;

	Rect2D avatar_bounds = Rect2D(
		data->item_bounds.X + (spacing),
		data->item_bounds.Y + (spacing),
		data->item_bounds.Height - (spacing * 2.0f),
		data->item_bounds.Height - (spacing * 2.0f)
	);
	Rect2D name_bounds = Rect2D(
		avatar_bounds.X + avatar_bounds.Width + (spacing * 2),
		avatar_bounds.Y,
		data->item_bounds.Width - (spacing * 3) - data->item_bounds.Height,
		text_height
	);
	Rect2D level_bounds = Rect2D(
		name_bounds.X ,
		name_bounds.Y + name_bounds.Height + spacing,
		name_bounds.Width,
		name_bounds.Height 
	);
	Rect2D prog_bounds = Rect2D(
		name_bounds.X,
		level_bounds.Y + level_bounds.Height + spacing + 2,
		name_bounds.Width - (2.0f * data->ui_scale.Y),
		name_bounds.Height - spacing - 2 - (2.0f * data->ui_scale.Y)
	);
	Rect2D prog_bar_rect = Rect2D(
		prog_bounds.X + 1,
		prog_bounds.Y + 1, 
		prog_bounds.Width - (2), 
		prog_bounds.Height - (2)
	);

	// Draw character.
	AtlasFrame* frame = achievement->locked_frame;
	Color title_color = Color::White;
	if (achievement->unlocked)
	{
		frame = achievement->unlocked_frame;
		title_color = Color(0, 131, 47, 255);
	}
	if (frame != NULL)
	{
		m_atlas_renderer.Draw_Frame(frame, Rect2D(avatar_bounds.X + avatar_spacing, avatar_bounds.Y + avatar_spacing, avatar_bounds.Width - (avatar_spacing * 2), avatar_bounds.Height - (avatar_spacing * 2)) , 0.0f, Color::White);
	}
	//m_item_frame_inactive.Draw_Frame(m_atlas_renderer, avatar_bounds, data->ui_scale, Color(200, 200, 200, 255));

	// Draw name.
	m_shadow_font_renderer.Draw_String(
		S(achievement->name.c_str()), 
		name_bounds, 
		16.0f,
		title_color, 
		TextAlignment::Left,
		TextAlignment::Top,
		data->ui_scale * 0.5f);

	// Draw level.
	m_font_renderer.Draw_String(
		S(achievement->description.c_str()), 
		level_bounds, 
		16.0f,
		Color(200, 200, 200, 255), 
		TextAlignment::Left,
		TextAlignment::Top,
		data->ui_scale * 0.4f);

	// Draw progress.
	float progress = Clamp((float)achievement->progress / (float)achievement->max_progress, 0.0f, 1.0f);
	Color bg_color = Color(32, 32, 32, 255);
	Color color = Color(255, 194, 14, 255);
	prim_renderer.Draw_Solid_Quad(prog_bar_rect, bg_color);
	prim_renderer.Draw_Solid_Quad(Rect2D(prog_bar_rect.X, prog_bar_rect.Y, (prog_bar_rect.Width * progress), prog_bar_rect.Height), color);
}

void UIScene_Achievements::Recieve_Event(UIManager* manager, UIEvent e)
{
	switch (e.Type)
	{
	case UIEventType::Button_Click:
		{	
			if (e.Source->Get_Name() == "back_button")
			{
				manager->Go(UIAction::Pop(new UISlideInTopLevelTransition()));
			}
		}
		break;
	}
}
