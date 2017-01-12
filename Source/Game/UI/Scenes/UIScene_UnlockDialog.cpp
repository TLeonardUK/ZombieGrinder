// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_UnlockDialog.h"
#include "Engine/UI/Transitions/UISlideInTransition.h"
#include "Engine/UI/Transitions/UIFadeOutTransition.h"
#include "Engine/UI/Layouts/UILayoutFactory.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Display/GfxDisplay.h"
#include "Engine/Input/Input.h"
#include "Engine/UI/UIManager.h"
#include "Engine/UI/Elements/UIPanel.h"
#include "Engine/UI/Elements/UISlider.h"
#include "Engine/UI/Elements/UIListView.h"
#include "Engine/UI/Elements/UIProgressBar.h"
#include "Engine/UI/Elements/UIListView.h"
#include "Engine/UI/Elements/UILabel.h"
#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Resources/ResourceFactory.h"

#include "Game/Scene/Actors/ScriptedActor.h"

#include "Game/Profile/ItemManager.h"
#include "Game/Profile/SkillManager.h"
#include "Game/Profile/ProfileManager.h"

#include "Engine/Localise/Locale.h"

#include "Game/UI/Scenes/UIScene_Game.h"

#include "Game/Runner/Game.h"
#include "Engine/Engine/EngineOptions.h"
#include "Game/Runner/GameOptions.h"

UIScene_UnlockDialog::UIScene_UnlockDialog(std::vector<UnlockDialogEntry> events)
	: m_events(events)
	, m_direction(0)
	, m_direction_change_timer(0.0f)
{
	Set_Layout("unlock_dialog");

	m_draw_item_delegate = new Delegate<UIScene_UnlockDialog, UIListViewDrawItemData>(this, &UIScene_UnlockDialog::On_Draw_Item);

	UIListView* listview = Find_Element<UIListView*>("listview");
	listview->On_Draw_Item += m_draw_item_delegate;
	listview->Set_Faked_Item_Count(events.size());

	for (unsigned int i = 0; i < events.size(); i++)
	{
		listview->Add_Item("", reinterpret_cast<void*>(i));
	}

	ProfileManager::Get()->Set_Sync_Enabled(false);
}

UIScene_UnlockDialog::~UIScene_UnlockDialog()
{
	for (unsigned int i = 0; i < m_events.size(); i++)
	{
		UnlockDialogEntry& entry = m_events[i];
		if (entry.Type == UnlockDialogEntryType::Item_Broken)
		{
			entry.User_Profile->Unequip_Item(entry.Item_Instance);
			entry.User_Profile->Ensure_Valid_Equips();
		}
	}
	Game::Get()->Queue_Save();
	ProfileManager::Get()->Set_Sync_Enabled(true);
}

bool UIScene_UnlockDialog::Can_Accept_Invite()
{
	return false;
}

const char* UIScene_UnlockDialog::Get_Name()
{
	return "UIScene_UnlockDialog";
}

bool UIScene_UnlockDialog::Should_Render_Lower_Scenes()
{
	return true;
}

bool UIScene_UnlockDialog::Should_Display_Cursor()
{
	return true;
}

bool UIScene_UnlockDialog::Should_Display_Focus_Cursor()
{
	return true;
}

bool UIScene_UnlockDialog::Is_Focusable()
{
	return true;
}

void UIScene_UnlockDialog::Enter(UIManager* manager)
{
}

void UIScene_UnlockDialog::Exit(UIManager* manager)
{
}

void UIScene_UnlockDialog::Refresh(UIManager* manager)
{
	m_atlas = manager->Get_Atlas();
	m_atlas_renderer = AtlasRenderer(m_atlas);

	m_font = manager->Get_Font();
	m_font_renderer = MarkupFontRenderer(m_font, false, false, false);

	m_item_frame_active = UIFrame("screen_main_item_border_active_#");
	m_item_frame_inactive = UIFrame("screen_main_item_border_inactive_#");

	UIScene::Refresh(manager);
}

void UIScene_UnlockDialog::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Tick(time, manager, scene_index);

	m_direction_change_timer += time.Get_Frame_Time();
	if (m_direction_change_timer >= DIRECTION_CHANGE_INTERVAL)
	{
		m_direction = (m_direction + 1) % 8;
		m_direction_change_timer = 0.0f;
	}
}

void UIScene_UnlockDialog::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Draw(time, manager, scene_index);
}

void UIScene_UnlockDialog::Recieve_Event(UIManager* manager, UIEvent e)
{
	switch (e.Type)
	{
	case UIEventType::Button_Click:
	{
		if (e.Source->Get_Name() == "back_button")
		{
			manager->Go(UIAction::Pop(new UIFadeOutTransition()));
		}
	}
	break;
	}
}

void UIScene_UnlockDialog::On_Draw_Item(UIListViewDrawItemData* data)
{
	PrimitiveRenderer prim_renderer;

	UIListView* profile_list = Find_Element<UIListView*>("listview");
	UnlockDialogEntry& entry = m_events[reinterpret_cast<int>(data->item->MetaData)];

	const float spacing = 2.0f * data->ui_scale.Y;
	const float text_height = 9.0f * data->ui_scale.Y;

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
		name_bounds.X,
		name_bounds.Y + name_bounds.Height + spacing,
		name_bounds.Width,
		(avatar_bounds.Y + avatar_bounds.Height) - (name_bounds.Y + name_bounds.Height + spacing)
	);

	// Draw outline.
	if (data->selected)
		m_item_frame_active.Draw_Frame(m_atlas_renderer, data->item_bounds, data->ui_scale, Color::White);
	else if (data->hovering)
		m_item_frame_active.Draw_Frame(m_atlas_renderer, data->item_bounds, data->ui_scale, Color(128, 128, 128, 255));
	else
		m_item_frame_inactive.Draw_Frame(m_atlas_renderer, data->item_bounds, data->ui_scale, Color::White);

	std::string name = "";
	std::string description = "";
	std::string action = "";
	AtlasFrame* fore_image = NULL;
	AtlasFrame* back_image = NULL;
	Color fore_color = Color::White;
	Vector2 image_offset(0.0f, 0.0f);
	bool fill_image = false;

	Vector2 ui_scale = data->ui_scale;
	Vector2 icon_scale = ui_scale * Vector2(0.75f, 0.75f);

	switch (entry.Type)
	{
		case UnlockDialogEntryType::Online_Item_Drop:
		{
			ItemArchetype* arch = ItemManager::Get()->Find_Archetype_By_InventoryId(entry.Online_Item.type_id);

			name = SF("#unlock_dialog_title_online", arch->Get_Indestructable_Display_Name().c_str());
			action = S("#unlock_dialog_action_online");
			description = arch->Get_Description(true);

			image_offset = arch->icon_offset * icon_scale;
			fore_color = arch->default_tint;
			if (arch->is_icon_direction_based)
			{
				back_image = arch->icon_animations[m_direction]->Frames[0];
				fore_image = arch->is_tintable ? arch->icon_tint_animations[m_direction]->Frames[0] : NULL;
			}
			else
			{
				back_image = arch->icon_animations[0]->Frames[0];
				fore_image = arch->is_tintable ? arch->icon_tint_animations[0]->Frames[0] : NULL;
			}
			break;
		}
		case UnlockDialogEntryType::Item_Unpacked:
		{
			switch (entry.Unpack_Event.type)
			{
				case ProfileUnpackEventType::Coins:
				{
					name = SF("#unlock_dialog_title_unpack_coins", entry.Unpack_Event.amount);
					action = S("#unlock_dialog_action_unpack_coins");
					description = S("#unlock_dialog_description_unpack_coins");

					image_offset = Vector2(0.0f, 0.0f);
					fore_color = Color::White;
					back_image = ResourceFactory::Get()->Get_Atlas_Frame("item_accessory_coins_drop_0");
					fore_image = NULL;
					fill_image = true;
					icon_scale = Vector2(1.0f, 1.0f);
					break;
				}
				case ProfileUnpackEventType::XP:
				{
					name = SF("#unlock_dialog_title_unpack_xp", entry.Unpack_Event.amount);
					action = S("#unlock_dialog_action_unpack_xp");
					description = S("#unlock_dialog_description_unpack_xp");

					image_offset = Vector2(0.0f, 0.0f);
					fore_color = Color::White;
					back_image = ResourceFactory::Get()->Get_Atlas_Frame("item_accessory_xp_drop_0");
					fore_image = NULL;
					fill_image = true;
					icon_scale = Vector2(1.0f, 1.0f);
					break;
				}
				case ProfileUnpackEventType::Item:
				{
					ItemArchetype* arch = entry.Unpack_Event.item;

					name = SF("#unlock_dialog_title_unpack_item", arch->Get_Indestructable_Display_Name().c_str());
					action = S("#unlock_dialog_action_unpack_item");
					description = arch->Get_Description(false);

					image_offset = arch->icon_offset * icon_scale;
					fore_color = arch->default_tint;
					if (arch->is_icon_direction_based)
					{
						back_image = arch->icon_animations[m_direction]->Frames[0];
						fore_image = arch->is_tintable ? arch->icon_tint_animations[m_direction]->Frames[0] : NULL;
					}
					else
					{
						back_image = arch->icon_animations[0]->Frames[0];
						fore_image = arch->is_tintable ? arch->icon_tint_animations[0]->Frames[0] : NULL;
					}
					break;
				}
			}
			break;
		}
		case UnlockDialogEntryType::Skill_Unlocked:
		{
			name = SF("#unlock_dialog_title_skill_unlocked", entry.Archetype_For_Skill->Get_Display_Name().c_str());
			action = S("#unlock_dialog_action_skill_unlocked");
			description = SF("#unlock_dialog_description_skill_unlocked", S(entry.Archetype_For_Skill->description.c_str()));

			image_offset = Vector2(0.0f, 0.0f);
			fore_color = Color::White;
			back_image = entry.Archetype_For_Skill->icon;
			fore_image = NULL;
			fill_image = true;
			icon_scale = Vector2(1.0f, 1.0f);
			break;
		}
		case UnlockDialogEntryType::Item_Unlocked:
		{
			name = SF("#unlock_dialog_title_item_unlocked", entry.Archetype_For_Item->Get_Base_Display_Name().c_str());
			action = S("#unlock_dialog_action_item_unlocked");
			description = SF("#unlock_dialog_description_item_unlocked", entry.Archetype_For_Item->Get_Description(false).c_str());

			image_offset = entry.Archetype_For_Item->icon_offset * icon_scale;
			fore_color = entry.Archetype_For_Item->default_tint;
			if (entry.Archetype_For_Item->is_icon_direction_based)
			{
				back_image = entry.Archetype_For_Item->icon_animations[m_direction]->Frames[0];
				fore_image = entry.Archetype_For_Item->is_tintable ? entry.Archetype_For_Item->icon_tint_animations[m_direction]->Frames[0] : NULL;
			}
			else
			{
				back_image = entry.Archetype_For_Item->icon_animations[0]->Frames[0];
				fore_image = entry.Archetype_For_Item->is_tintable ? entry.Archetype_For_Item->icon_tint_animations[0]->Frames[0] : NULL;
			}
			break;
		}
		case UnlockDialogEntryType::Challenge_Reward:
		case UnlockDialogEntryType::Item_Broken:
		{
			if (entry.Type == UnlockDialogEntryType::Challenge_Reward)
			{
				name = SF("#unlock_dialog_title_challenge", entry.Item_Instance->archetype->Get_Fragile_Display_Name().c_str());
				action = S("#unlock_dialog_action_challenge");
				description = entry.Item_Instance->archetype->Get_Description(entry.Item_Instance->indestructable);
			}
			else
			{
				name = SF("#unlock_dialog_title_item_broken", entry.Item_Instance->archetype->Get_Fragile_Display_Name().c_str());
				action = S("#unlock_dialog_action_item_broken");
				description = S("#unlock_dialog_description_item_broken");
			}

			image_offset = entry.Item_Instance->archetype->icon_offset * icon_scale;
			fore_color = entry.Item_Instance->primary_color;
			if (entry.Item_Instance->archetype->is_icon_direction_based)
			{
				back_image = entry.Item_Instance->archetype->icon_animations[m_direction]->Frames[0];
				fore_image = entry.Item_Instance->archetype->is_tintable ? entry.Item_Instance->archetype->icon_tint_animations[m_direction]->Frames[0] : NULL;
			}
			else
			{
				back_image = entry.Item_Instance->archetype->icon_animations[0]->Frames[0];
				fore_image = entry.Item_Instance->archetype->is_tintable ? entry.Item_Instance->archetype->icon_tint_animations[0]->Frames[0] : NULL;
			}
			break;
		}
	}

	// Draw character.
	m_item_frame_inactive.Draw_Frame(m_atlas_renderer, avatar_bounds, data->ui_scale, Color(200, 200, 200, 255));

	// Draw image.
	if (fill_image)
	{
		if (back_image != NULL)
		{
			m_atlas_renderer.Draw_Frame(back_image, avatar_bounds.Inflate(-spacing, -spacing), 0.0f, Color::White, false, false, RendererOption::E_Src_Alpha_One_Minus_Src_Alpha, icon_scale);
		}
	}
	else
	{
		if (back_image != NULL)
		{
			m_atlas_renderer.Draw_Frame(back_image, avatar_bounds.Center() - image_offset, 0.0f, Color::White, false, false, RendererOption::E_Src_Alpha_One_Minus_Src_Alpha, icon_scale);
		}
		if (fore_image != NULL)
		{
			m_atlas_renderer.Draw_Frame(fore_image, avatar_bounds.Center() - image_offset, 0.0f, fore_color, false, false, RendererOption::E_Src_Alpha_One_Minus_Src_Alpha, icon_scale);
		}
	}

	// Draw title.
	m_font_renderer.Draw_String(
		name.c_str(),
		name_bounds,
		16.0f,
		Color(200, 200, 200, 255),
		TextAlignment::Left,
		TextAlignment::Top,
		data->ui_scale * 0.5f);

	// Draw action.
	m_font_renderer.Draw_String(
		action.c_str(),
		name_bounds,
		16.0f,
		Color(200, 32, 32, 255),
		TextAlignment::Right,
		TextAlignment::Top,
		data->ui_scale * 0.5f);

	// Draw description.
	m_font_renderer.Draw_String(
		m_font_renderer.Word_Wrap(description.c_str(), level_bounds, 16.0f, data->ui_scale * 0.4f).c_str(),
		level_bounds,
		16.0f,
		Color(200, 200, 200, 255),
		TextAlignment::Left,
		TextAlignment::Top,
		data->ui_scale * 0.4f);

}