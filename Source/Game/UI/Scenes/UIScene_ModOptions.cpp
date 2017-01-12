// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_ModOptions.h"
#include "Game/UI/Scenes/UIScene_Dialog.h"
#include "Game/UI/Scenes/UIScene_RestrictedModeWarning.h"
#include "Engine/UI/Transitions/UISlideInTransition.h"
#include "Engine/UI/Transitions/UIFadeInTransition.h"
#include "Engine/UI/Transitions/UIFadeOutTransition.h"
#include "Engine/UI/Layouts/UILayoutFactory.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Display/GfxDisplay.h"
#include "Engine/Input/Input.h"
#include "Engine/UI/UIManager.h"
#include "Engine/UI/Elements/UIPanel.h"
#include "Engine/UI/Elements/UISlider.h"
#include "Engine/UI/Elements/UICheckBox.h"
#include "Engine/UI/Elements/UIListView.h"
#include "Engine/UI/Elements/UIComboBox.h"
#include "Engine/UI/Elements/UIProgressBar.h"
#include "Engine/UI/Elements/UIButton.h"
#include "Engine/UI/Elements/UILabel.h"
#include "Engine/UI/Elements/UIDropDownMenu.h"
#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Resources/ResourceFactory.h"

#include "Engine/GeoIP/GeoIPManager.h"

#include "Engine/Online/OnlineMatching.h"
#include "Engine/Online/OnlinePlatform.h"
#include "Engine/Online/OnlineMods.h"

#include "Engine/Localise/Locale.h"

#include "Game/UI/Scenes/UIScene_Game.h"

#include "Game/Runner/Game.h"
#include "Engine/Engine/EngineOptions.h"
#include "Game/Runner/GameOptions.h"

struct ModCategory
{
	enum Type
	{
		Subscribed,
		Favorites,
		Maps,
		SoundPacks,
		SkinPacks,
		Items,
		Weapons,
		Hats,
		All
	};
};

UIScene_ModOptions::UIScene_ModOptions()
	: m_mods_changed(false)
{
	Set_Layout("mod_options");	

	m_draw_item_delegate = new Delegate<UIScene_ModOptions, UIListViewDrawItemData>(this, &UIScene_ModOptions::On_Draw_Item);

	UIListView* listview = Find_Element<UIListView*>("server_listview");
	listview->Clear_Columns();
	listview->On_Draw_Item += m_draw_item_delegate;

	UICheckBox* view_unapproved = Find_Element<UICheckBox*>("view_unapproved_checkbox");
	view_unapproved->Set_Checked(*GameOptions::workshop_show_unapproved);

	UIComboBox* category_box = Find_Element<UIComboBox*>("source_box");
	category_box->Set_Selected_Item_Index((int)ModCategory::All);

	// Notice: Keep these in sync with ModCategory above.
	category_box->Add_Item("Subscribed");
	category_box->Add_Item("Favorites");
	category_box->Add_Item("Maps");
	category_box->Add_Item("Sound Packs");
	category_box->Add_Item("Skin Packs");
	category_box->Add_Item("Items");
	category_box->Add_Item("Weapons");
	category_box->Add_Item("Hats");
	category_box->Add_Item("All");

	Refresh_Items(false);
}

const char* UIScene_ModOptions::Get_Name()
{
	return "UIScene_ModOptions";
}

UIScene* UIScene_ModOptions::Get_Background(UIManager* manager)
{
	if (manager->Get_Scene_By_Type<UIScene_Game*>() != NULL)
		return UIScene::Get_Background(manager);
	else
		return manager->Retrieve_Persistent_Scene("UIScene_MenuBackgroundSimple");
}

bool UIScene_ModOptions::Should_Render_Lower_Scenes()
{
	return false;
}

bool UIScene_ModOptions::Should_Display_Cursor()
{
	return true;
}

bool UIScene_ModOptions::Is_Focusable()
{
	return true;
}

void UIScene_ModOptions::Refresh(UIManager* manager)
{
	m_atlas = manager->Get_Atlas();
	m_atlas_renderer = AtlasRenderer(m_atlas);

	m_font = manager->Get_Font();
	m_font_renderer = FontRenderer(m_font, false, false);
	m_markup_font_renderer = MarkupFontRenderer(m_font, false, false, false);

	m_item_frame_active = UIFrame("screen_main_item_border_active_#");
	m_item_frame_inactive = UIFrame("screen_main_item_border_inactive_#");

	UIScene::Refresh(manager);
}

void UIScene_ModOptions::Enter(UIManager* manager)
{
	if (!(*GameOptions::workshop_warning_shown))
	{
		*GameOptions::workshop_warning_shown = true;
		manager->Go(UIAction::Push(new UIScene_RestrictedModeWarning(false), new UIFadeInTransition()));
	}
}	

void UIScene_ModOptions::Exit(UIManager* manager)
{
	if (m_mods_changed)
	{
		manager->Go(UIAction::Push(new UIScene_Dialog("#menu_mod_restart_dialog"), new UIFadeInTransition()));
	}
}	

void UIScene_ModOptions::Refresh_Items(bool bResetSliderToStart)
{
	std::vector<OnlineSubscribedMod*> mods;

	UIComboBox* category_box = Find_Element<UIComboBox*>("source_box");
	ModCategory::Type category = (ModCategory::Type)category_box->Get_Selected_Item_Index();

	std::vector<std::string> tags;

	switch (category)
	{
	case ModCategory::Subscribed:
		{
			mods = OnlineMods::Get()->Get_Subscribed_Mods();
			break;
		}
	case ModCategory::Favorites:
		{
			mods = OnlineMods::Get()->Get_Favorited_Mods();
			break;
		}
	case ModCategory::Maps:
		{
			tags.push_back("Map");
			mods = OnlineMods::Get()->Get_Tagged_Items(tags);
			break;
		}
	case ModCategory::SoundPacks:
		{
			tags.push_back("Sound");
			mods = OnlineMods::Get()->Get_Tagged_Items(tags);
			break;
		}
	case ModCategory::SkinPacks:
		{
			tags.push_back("Skin");
			mods = OnlineMods::Get()->Get_Tagged_Items(tags);
			break;
		}
	case ModCategory::Items:
		{
			tags.push_back("Item");
			mods = OnlineMods::Get()->Get_Tagged_Items(tags);
			break;
		}
	case ModCategory::Weapons:
		{
			tags.push_back("Weapon");
			mods = OnlineMods::Get()->Get_Tagged_Items(tags);
			break;
		}
	case ModCategory::Hats:
		{
			tags.push_back("Hat");
			mods = OnlineMods::Get()->Get_Tagged_Items(tags);
			break;
		}
	case ModCategory::All:
		{
			mods = OnlineMods::Get()->Get_Tagged_Items(tags);
			break;
		}
	}

	UIListView* listview = Find_Element<UIListView*>("server_listview");
	int previous_scroll_offset = listview->Get_Scroll_Offset();

	listview->Clear_Items();

	if (mods.size() == 0)
	{
		listview->Set_Item_Height(12.0f);
		listview->Add_Item("", NULL);
	}
	else
	{
		listview->Set_Item_Height(48.0f);

		for (std::vector<OnlineSubscribedMod*>::iterator iter = mods.begin(); iter != mods.end(); iter++)
		{
			OnlineSubscribedMod* mod = *iter;
			if ((mod->IsWhitelisted || *GameOptions::workshop_show_unapproved) || category == ModCategory::Subscribed)
			{
				listview->Add_Item(" ", mod);
			}
		}
	}

	// Reset slider to zero if we change category.
	if (bResetSliderToStart)
	{
		listview->Get_Slider()->Set_Progress(0.0f);
	}

	// Otherwise adjust slider progress proportionally to the number of items that have been added.
	else
	{
		listview->Scroll_To(previous_scroll_offset);
	}
}

void UIScene_ModOptions::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	if (OnlineMods::Get()->Get_Item_Lists_Dirty())
	{
		Refresh_Items(false);
	}
	else
	{
		UIListView* item_list = Find_Element<UIListView*>("server_listview");
		if (item_list->Get_Slider()->Get_Progress() >= 0.95f)
		{
			UIComboBox* category_box = Find_Element<UIComboBox*>("source_box");
			ModCategory::Type category = (ModCategory::Type)category_box->Get_Selected_Item_Index();

			std::vector<std::string> tags;

			switch (category)
			{
			case ModCategory::Maps:
				{
					tags.push_back("Map");
					OnlineMods::Get()->Page_Tagged_Items(tags);
					break;
				}
			case ModCategory::SoundPacks:
				{
					tags.push_back("Sound");
					OnlineMods::Get()->Page_Tagged_Items(tags);
					break;
				}
			case ModCategory::SkinPacks:
				{
					tags.push_back("Skin");
					OnlineMods::Get()->Page_Tagged_Items(tags);
					break;
				}
			case ModCategory::Items:
				{
					tags.push_back("Item");
					OnlineMods::Get()->Page_Tagged_Items(tags);
					break;
				}
			case ModCategory::Weapons:
				{
					tags.push_back("Weapon");
					 OnlineMods::Get()->Page_Tagged_Items(tags);
					break;
				}
			case ModCategory::Hats:
				{
					tags.push_back("Hat");
					OnlineMods::Get()->Page_Tagged_Items(tags);
					break;
				}
			case ModCategory::All:
				{
					OnlineMods::Get()->Page_Tagged_Items(tags);
					break;
				}
			}
		}
	}

	UIScene::Tick(time, manager, scene_index);
}

void UIScene_ModOptions::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{

	UIScene::Draw(time, manager, scene_index);
}

void UIScene_ModOptions::On_Draw_Item(UIListViewDrawItemData* data)
{
	OnlineSubscribedMod* mod = (OnlineSubscribedMod*)data->item->MetaData;
	if (mod == NULL)
	{
		return;
	}

	RenderPipeline* pipeline = RenderPipeline::Get();
	PrimitiveRenderer pr;
	AtlasRenderer ar;
	FontRenderer fr(data->manager->Get_Font());
	Vector2 ui_scale = data->manager->Get_UI_Scale();

	// Draw outline.
	if (data->selected)
		m_item_frame_active.Draw_Frame(m_atlas_renderer, data->item_bounds, data->ui_scale, Color::White);
	else if (data->hovering)
		m_item_frame_active.Draw_Frame(m_atlas_renderer, data->item_bounds, data->ui_scale, Color(128, 128, 128, 255));
	else
		m_item_frame_inactive.Draw_Frame(m_atlas_renderer, data->item_bounds, data->ui_scale, Color::White);

	const float spacing = 2.0f * data->ui_scale.Y;
	const float text_height = 9.0f * data->ui_scale.Y;

	Rect2D preview_bounds = Rect2D(
		data->item_bounds.X + (spacing),
		data->item_bounds.Y + (spacing),
		(data->item_bounds.Height * 1.333f) - (spacing * 2.0f),
		data->item_bounds.Height - (spacing * 2.0f)
	);
	Rect2D preview_inner_bounds = Rect2D(
		preview_bounds.X + (spacing),
		preview_bounds.Y + (spacing),
		preview_bounds.Width - (spacing * 2.0f),
		preview_bounds.Height - (spacing * 2.0f)
	);
	Rect2D votes_bounds = Rect2D(
		preview_bounds.X,
		preview_bounds.Y + preview_bounds.Height * 0.75f,
		preview_bounds.Width * 0.5f,
		preview_bounds.Height * 0.25f
	);
	Rect2D whitelist_bounds = Rect2D(
		preview_bounds.X,
		preview_bounds.Y,
		preview_bounds.Width * 1.0f,
		preview_bounds.Height * 0.25f
	);
	Rect2D title_bounds = Rect2D(
		preview_bounds.X + preview_bounds.Width + (spacing),
		preview_bounds.Y,
		(data->item_bounds.Width - preview_bounds.Width) - (spacing * 4.0f),
		data->item_bounds.Height * 0.25f
	);
	Rect2D description_bounds = Rect2D(
		title_bounds.X,
		title_bounds.Y + title_bounds.Height + spacing,
		title_bounds.Width,
		data->item_bounds.Height - title_bounds.Height - (spacing * 3)
	);

	// Draw preview.
	Pixelmap* preview = NULL;
	Texture* previewTexture = NULL;
	OnlineMods::Get()->Get_Mod_Preview(mod->ID, preview, previewTexture);

	m_item_frame_inactive.Draw_Frame(m_atlas_renderer, preview_bounds, data->ui_scale, Color(200, 200, 200, 255));
	if (preview != NULL && previewTexture != NULL)
	{
		m_atlas_renderer.Draw(previewTexture, Rect2D(0, 0, 1, 1), Vector2(0, 0), preview_inner_bounds, 0.0f, Color::White);
	}

	// Draw whitelist message.
	bool bWhitelisted = mod->IsWhitelisted;
	if (!bWhitelisted)
	{
		pr.Draw_Solid_Quad(whitelist_bounds, Color(0, 0, 0, 170));
		m_markup_font_renderer.Draw_String(S("#mod_not_whitelisted"), whitelist_bounds, 16.0f * 0.5f, Color(255, 128, 128, 225), TextAlignment::Center, TextAlignment::ScaleToFit, ui_scale);
	}

	// Draw votes.
	pr.Draw_Solid_Quad(votes_bounds, Color(0, 0, 0, 170));

	std::string vote_text = SF("#menu_mod_vote_text", mod->UpVotes, mod->DownVotes);
	m_markup_font_renderer.Draw_String(vote_text.c_str(), votes_bounds, 16.0f * 0.5f, Color(255, 255, 255, 225), TextAlignment::Center, TextAlignment::ScaleToFit, ui_scale);

	std::string description = m_font_renderer.Word_Wrap(mod->Description.c_str(), description_bounds, 16.0f * 0.25f, ui_scale);

	// Draw title.
	m_font_renderer.Draw_String(mod->Title.c_str(), title_bounds, 16.0f * 0.5f, Color::White, TextAlignment::Left, TextAlignment::ScaleToFit, ui_scale);
	m_font_renderer.Draw_String(mod->Author.c_str(), title_bounds, 16.0f * 0.5f, Color::White, TextAlignment::Right, TextAlignment::ScaleToFit, ui_scale);
	m_font_renderer.Draw_String(description.c_str(), description_bounds, 16.0f * 0.25f, Color::White, TextAlignment::Left, TextAlignment::Top, ui_scale);
}

void UIScene_ModOptions::Recieve_Event(UIManager* manager, UIEvent e)
{
	switch (e.Type)
	{
	case UIEventType::ComboBox_SelectedIndexChanged:
		{
			if (e.Source->Get_Name() == "source_box")
			{
				Refresh_Items(true);
			}
		}
		break;

	case UIEventType::ListView_Item_Click:
		{
			if (e.Source->Get_Name() == "server_listview")
			{
				UIListView* view = dynamic_cast<UIListView*>(e.Source);
				int index = view->Get_Selected_Item_Index();

				UIDropDownMenu* drop_down_menu = Find_Element<UIDropDownMenu*>("mod_option_menu");
				UIListView* item_list = Find_Element<UIListView*>("server_listview");

				OnlineSubscribedMod* mod = (OnlineSubscribedMod*)item_list->Get_Selected_Item().MetaData;

				Find_Element<UILabel*>("subscribe_button")->Set_Value(mod->Subscribed ? "#menu_mod_unsubscribe" : "#menu_mod_subscribe");
				Find_Element<UILabel*>("favorite_button")->Set_Value(mod->Favorited ? "#menu_mod_unfavorite" : "#menu_mod_favorite");
				Find_Element<UIButton*>("vote_up_button")->Set_Enabled(!mod->HasVoted || !mod->VotedUp);
				Find_Element<UIButton*>("vote_down_button")->Set_Enabled(!mod->HasVoted || mod->VotedUp);

				Rect2D selection_box = item_list->Get_Selected_Item_Rectangle();

				drop_down_menu->Show(Rect2D(selection_box.X, selection_box.Y + selection_box.Height, selection_box.Width, drop_down_menu->Get_Screen_Box().Height));
			}
		}
		break;

	case UIEventType::CheckBox_Click:
		{
			if (e.Source->Get_Name() == "view_unapproved_checkbox")
			{
				*GameOptions::workshop_show_unapproved = dynamic_cast<UICheckBox*>(e.Source)->Get_Checked();
				Refresh_Items(true);
			}
			break;
		}

	case UIEventType::Button_Click:
		{
			UIDropDownMenu* drop_down_menu = Find_Element<UIDropDownMenu*>("mod_option_menu");
			UIListView* item_list = Find_Element<UIListView*>("server_listview");

			if (e.Source->Get_Name() == "view_all_button")
			{
				OnlinePlatform::Get()->Show_Mod_Gallery();
			}
			else if (e.Source->Get_Name() == "back_button")
			{
				manager->Go(UIAction::Pop(new UISlideInTransition()));
			}
			else if (e.Source->Get_Name() == "subscribe_button")
			{
				OnlineSubscribedMod* mod = (OnlineSubscribedMod*)item_list->Get_Selected_Item().MetaData;
				if (mod->Subscribed)
				{
					OnlineMods::Get()->Unsubscribe(mod->ID);
					mod->Subscribed = false;
				}
				else
				{
					OnlineMods::Get()->Subscribe(mod->ID);
					mod->Subscribed = true;
				}

				drop_down_menu->Hide();

				m_mods_changed = true;

				Refresh_Items(false);
			}
			else if (e.Source->Get_Name() == "favorite_button")
			{
				OnlineSubscribedMod* mod = (OnlineSubscribedMod*)item_list->Get_Selected_Item().MetaData;
				if (mod->Favorited)
				{
					OnlineMods::Get()->Unfavorite(mod->ID);
					mod->Favorited = false;
				}
				else
				{
					OnlineMods::Get()->Favorite(mod->ID);
					mod->Favorited = true;
				}
				
				drop_down_menu->Hide();

				Refresh_Items(false);
			}
			else if (e.Source->Get_Name() == "view_on_steam_button")
			{
				OnlineSubscribedMod* mod = (OnlineSubscribedMod*)item_list->Get_Selected_Item().MetaData;
				OnlinePlatform::Get()->Show_Web_Browser(StringHelper::Format("http://steamcommunity.com/sharedfiles/filedetails/?id=%llu", mod->ID).c_str());

				drop_down_menu->Hide();
			}
			else if (e.Source->Get_Name() == "vote_up_button")
			{
				OnlineSubscribedMod* mod = (OnlineSubscribedMod*)item_list->Get_Selected_Item().MetaData;
				OnlineMods::Get()->VoteUp(mod->ID);

				drop_down_menu->Hide();
			}
			else if (e.Source->Get_Name() == "vote_down_button")
			{
				OnlineSubscribedMod* mod = (OnlineSubscribedMod*)item_list->Get_Selected_Item().MetaData;
				OnlineMods::Get()->VoteDown(mod->ID);

				drop_down_menu->Hide();
			}
			else if (e.Source->Get_Name() == "close_button")
			{
				drop_down_menu->Hide();
			}
		}
		break;
	}
}
