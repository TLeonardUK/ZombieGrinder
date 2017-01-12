// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_Skills.h"
#include "Game/UI/Scenes/UIScene_ConfirmDialog.h"
#include "Game/UI/Scenes/UIScene_Dialog.h"
#include "Engine/UI/Transitions/UISlideInTransition.h"
#include "Engine/UI/Transitions/UISlideInTopLevelTransition.h"
#include "Engine/UI/Transitions/UIFadeInTransition.h"
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
#include "Engine/UI/Elements/UIButton.h"
#include "Engine/UI/Elements/UITextBox.h"
#include "Engine/UI/Elements/UIGridTree.h"
#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Resources/ResourceFactory.h"

#include "Engine/Audio/AudioRenderer.h"

#include "Engine/Localise/Locale.h"
#include "Engine/Engine/GameEngine.h"

#include "Game/UI/Scenes/UIScene_Game.h"

#include "Game/Runner/Game.h"
#include "Engine/Engine/EngineOptions.h"
#include "Game/Runner/GameOptions.h"

#include "Game/Profile/ProfileManager.h"
#include "Game/Profile/ItemManager.h"
#include "Game/Profile/SkillManager.h"

UIScene_Skills::UIScene_Skills(Profile* profile)
	: m_profile(profile)
	, m_lock_frame(NULL)
	, m_info_alpha(0.0f)
	, m_last_selected_node(NULL)
	, m_purchase_item(NULL)
	, m_rerolling_skills(false)
	, m_last_equip_slot_hover(-1)
	, m_last_equip_slot_hover_timer(0.0f)
{
	DBG_ASSERT(profile != NULL);
	DBG_LOG("Modifying skills for: %s", profile->Name.c_str());
	
	m_display_points = (float)m_profile->Skill_Points;

	Set_Layout("skills");	
	
	m_draw_item_delegate = new Delegate<UIScene_Skills, UIGridTreeDrawItemData>(this, &UIScene_Skills::On_Draw_Item);
	m_can_drag_delegate = new Delegate<UIScene_Skills, UIGridTreeCanDragData>(this, &UIScene_Skills::Can_Drag_Drop);
	
	// Add all items
	UIGridTree* grid_view = Find_Element<UIGridTree*>("grid_view");
	grid_view->On_Draw_Item += m_draw_item_delegate;
	grid_view->On_Can_Drag += m_can_drag_delegate;
	grid_view->Clear_Nodes();

	std::vector<SkillArchetype*> items = SkillManager::Get()->Get_Archetypes();	
	for (std::vector<SkillArchetype*>::iterator iter = items.begin(); iter != items.end(); iter++)
	{
		SkillArchetype* item = *iter;
		if (item->parent == NULL)
		{
			Add_Tree_Item(grid_view, item, NULL, items);
		}
	}

	m_lock_frame = ResourceFactory::Get()->Get_Atlas_Frame("game_hud_lock");

	grid_view->Refresh_Tree();
}

UIScene_Skills::~UIScene_Skills()
{
	SAFE_DELETE(m_draw_item_delegate);
	SAFE_DELETE(m_can_drag_delegate);
}

void UIScene_Skills::Add_Tree_Item(UIGridTree* grid_view, SkillArchetype* skill, UIGridTreeNode* parent, std::vector<SkillArchetype*>& skills)
{
	UIGridTreeNode* node = grid_view->Add_Node(skill, parent, skill->are_children_mutex);
	for (std::vector<SkillArchetype*>::iterator iter = skills.begin(); iter != skills.end(); iter++)
	{
		SkillArchetype* item = *iter;
		if (item->parent == skill)
		{
			Add_Tree_Item(grid_view, item, node, skills);
		}
	}
}

const char* UIScene_Skills::Get_Name()
{
	return "UIScene_Skills";
}

bool UIScene_Skills::Should_Render_Lower_Scenes()
{
	return false;
}

bool UIScene_Skills::Should_Display_Cursor()
{
	return true;
}

bool UIScene_Skills::Is_Focusable()
{
	return true;
}

void UIScene_Skills::Enter(UIManager* manager)
{	
	UILabel* gold_label = Find_Element<UILabel*>("point_label");
	gold_label->Set_Value(SF("#menu_inventory_skill_points", StringHelper::Format_Number((float)m_display_points).c_str()));
	
	Update_Equip_Items();
}	

void UIScene_Skills::Exit(UIManager* manager)
{
}	

void UIScene_Skills::Update_Equip_Items()
{
	UIManager* manager = GameEngine::Get()->Get_UIManager();
	UIPanel* equip_slot_1 = Find_Element<UIPanel*>("equip_slot_1");
	UIPanel* equip_slot_2 = Find_Element<UIPanel*>("equip_slot_2");
	UIPanel* equip_slot_3 = Find_Element<UIPanel*>("equip_slot_3");
	
	Skill* skill_1 = m_profile->Get_Skill_Slot(0);
	Skill* skill_2 = m_profile->Get_Skill_Slot(1);
	Skill* skill_3 = m_profile->Get_Skill_Slot(2);

	equip_slot_1->Set_Foreground_Image(skill_1 != NULL ? skill_1->archetype->icon : NULL);
	equip_slot_2->Set_Foreground_Image(skill_2 != NULL ? skill_2->archetype->icon : NULL);
	equip_slot_3->Set_Foreground_Image(skill_3 != NULL ? skill_3->archetype->icon : NULL);
}

void UIScene_Skills::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	Vector2 ui_scale = manager->Get_UI_Scale();

	UILabel* point_label = Find_Element<UILabel*>("point_label");
	UIGridTree* grid = Find_Element<UIGridTree*>("grid_view");
	UIPanel* info_panel = Find_Element<UIPanel*>("info_panel");
	UILabel* info_name_label = Find_Element<UILabel*>("info_name_label");
	UILabel* info_price_label = Find_Element<UILabel*>("info_price_label");
//	UILabel* info_energy_source_label = Find_Element<UILabel*>("info_energy_source_label");
	UILabel* info_description_label = Find_Element<UILabel*>("info_description_label");	
	UIPanel* equip_slot_1 = Find_Element<UIPanel*>("equip_slot_1");
	UIPanel* equip_slot_2 = Find_Element<UIPanel*>("equip_slot_2");
	UIPanel* equip_slot_3 = Find_Element<UIPanel*>("equip_slot_3");

	UIGridTreeNode* selected_node = grid->Get_Selected_Node();

	Input* input = Input::Get();
	MouseState* mouse = input->Get_Mouse_State();
	Vector2 mouse_position = mouse->Get_Position();

	// Hover over an equip box long enough then zoom to item.
	m_last_equip_slot_hover_timer += time.Get_Frame_Time();

	if (equip_slot_1->Get_Screen_Box().Intersects(mouse_position))
	{
		if (m_last_equip_slot_hover == -1)
		{
			m_last_equip_slot_hover_timer = 0.0f;
		}
		m_last_equip_slot_hover = 0;
	}
	else if (equip_slot_2->Get_Screen_Box().Intersects(mouse_position))
	{		
		if (m_last_equip_slot_hover == -1)
		{
			m_last_equip_slot_hover_timer = 0.0f;
		}
		m_last_equip_slot_hover = 1;
	}
	else if (equip_slot_3->Get_Screen_Box().Intersects(mouse_position))
	{
		if (m_last_equip_slot_hover == -1)
		{
			m_last_equip_slot_hover_timer = 0.0f;
		}
		m_last_equip_slot_hover = 2;
	}
	else
	{
		m_last_equip_slot_hover = -1;
		m_last_equip_slot_hover_timer = 0.0f;
	}
	
	// Select skill slots by button-press.
	if (selected_node != NULL)
	{
		SkillArchetype* item = reinterpret_cast<SkillArchetype*>(selected_node->meta_data);
		Skill* skill = m_profile->Get_Skill(item);
		if (skill != NULL && item->is_passive == false)
		{
			if (input->Was_Pressed(OutputBindings::SkillSlot1))
			{
				m_profile->Equip_Skill(skill, 0);
				Update_Equip_Items();
				manager->Play_UI_Sound(UISoundType::Select);
			}
			else if (input->Was_Pressed(OutputBindings::SkillSlot2))
			{
				m_profile->Equip_Skill(skill, 1);
				Update_Equip_Items();
				manager->Play_UI_Sound(UISoundType::Select);
			}
			else if (input->Was_Pressed(OutputBindings::SkillSlot3))
			{
				m_profile->Equip_Skill(skill, 2);
				Update_Equip_Items();
				manager->Play_UI_Sound(UISoundType::Select);
			}
		}
	}

	if (mouse->Is_Button_Down(InputBindings::Mouse_Left) && m_last_equip_slot_hover >= 0 && !grid->Is_Dragging_Item())
	{
		Skill* skill = m_profile->Get_Skill_Slot(m_last_equip_slot_hover);
		if (skill != NULL)
		{
			m_profile->Unequip_Skill(skill);
			grid->Drag_Node_By_Meta_Data(skill->archetype);

			Update_Equip_Items();
			
			manager->Play_UI_Sound(UISoundType::Click);
		}
	}
	if (m_last_equip_slot_hover >= 0 && m_last_equip_slot_hover_timer > INFO_FADE_IN_DELAY)
	{
		Skill* skill = m_profile->Get_Skill_Slot(m_last_equip_slot_hover);
		if (skill != NULL)
		{
			grid->Select_Node_By_Meta_Data(skill->archetype);
		}
	}

	// Lerp the display coins if we have bought/sold anything.	
	if (abs(m_display_points - m_profile->Skill_Points) >= 1.0f)
	{
		m_display_points = Math::Lerp((float)m_display_points, (float)m_profile->Skill_Points, 0.1f);

		m_point_sound_timer += time.Get_Frame_Time();
		if (m_point_sound_timer >= POINT_SOUND_INTERVAL)
		{
			manager->Play_UI_Sound(UISoundType::Coin_Increase);
			m_point_sound_timer = 0.0f;
		}
	}
	else
	{
		m_display_points = (float)m_profile->Skill_Points;
	}

	// Show info dialog if we hover long enough.
	if (grid->Is_Focused(manager) && grid->Get_Selection_Time() >= INFO_FADE_IN_DELAY && selected_node != NULL)
	{
		m_info_alpha = Math::Lerp(m_info_alpha, 1.0f, 0.05f);
	}
	else
	{
		m_info_alpha = 0.0f;
	}

	// Position info next to the info box.
	float screen_width  = (float)GfxDisplay::Get()->Get_Width();
	float screen_height = (float)GfxDisplay::Get()->Get_Height();
		
	Rect2D item_box = grid->Get_Selected_Item_Box();
	Rect2D info_box = info_panel->Get_Screen_Box();
	Rect2D grid_box = grid->Get_Screen_Box();

	bool left = (item_box.X + (item_box.Width * 0.5f)) <= (screen_width * 0.5f);
	bool top  = (item_box.Y + (item_box.Height * 0.5f)) <= (screen_height * 0.5f);

	float spacing = 5.0f * ui_scale.Y;

	Rect2D box;

	// Top-Left
	if (left == true && top == true)
	{
		box = Rect2D(item_box.X, item_box.Y + item_box.Height + spacing, info_box.Width, info_box.Height);
	}
	// Top-Right
	else if (left == false && top == true)
	{
		box = Rect2D(item_box.X + item_box.Width - info_box.Width, item_box.Y + item_box.Height + spacing, info_box.Width, info_box.Height);
	}
	// Bottom-Left
	else if (left == true && top == false)
	{
		box = Rect2D(item_box.X, (item_box.Y - info_box.Height) - spacing, info_box.Width, info_box.Height);
	}
	// Bottom-Right
	else if (left == false && top == false)
	{
		box = Rect2D(item_box.X + item_box.Width - info_box.Width, (item_box.Y - info_box.Height) - spacing, info_box.Width, info_box.Height);
	}

	box.X = Min(Max(box.X, grid_box.X), (grid_box.X + grid_box.Width) - info_box.Width);
	box.Y = Min(Max(box.Y, grid_box.Y), (grid_box.Y + grid_box.Height) - info_box.Height);
	info_panel->Reposition(box);

	info_panel->Set_Frame_Color(Color(255.0f, 255.0f, 255.0f, 220.0f * m_info_alpha));
	info_name_label->Set_Color(Color(255.0f, 255.0f, 255.0f, 220.0f * m_info_alpha));
	info_price_label->Set_Color(Color(255.0f, 255.0f, 255.0f, 220.0f * m_info_alpha));
	info_description_label->Set_Color(Color(255.0f, 255.0f, 255.0f, 220.0f * m_info_alpha));
//	info_energy_source_label->Set_Color(Color(255.0f, 255.0f, 255.0f, 220.0f * m_info_alpha));

	// Show info.
	if (selected_node != m_last_selected_node && selected_node != NULL)
	{
		SkillArchetype* item = reinterpret_cast<SkillArchetype*>(selected_node->meta_data);
		info_name_label->Set_Value(item->Get_Display_Name());

		std::string description = S(item->description);
		if (!item->is_passive)
		{
			description += "\n\n";
			description += S("#menu_inventory_not_available_in_pvp");
		}

		bool isUnlocked = m_profile->Is_Skill_Unlocked(item);

		if (!isUnlocked)
		{
			SkillArchetype* unlock_item = NULL;
			if (item->unlock_critera_item != NULL)
			{
				unlock_item = SkillManager::Get()->Find_Archetype(item->unlock_critera_item);
			}

			switch (item->unlock_critera)
			{
				case SkillUnlockCriteria::Required_Rank:
				{
					description += StringHelper::Format("\n\n%s\n%s", S("#menu_shop_unlock_criteria"), SF("#menu_shop_unlock_required_rank", item->unlock_critera_threshold));
					break;
				}
				case SkillUnlockCriteria::Ailments_From_Weapon:
				{
					description += StringHelper::Format("\n\n%s\n%s", S("#menu_shop_unlock_criteria"), SF("#menu_shop_unlock_ailments_from_weapon", item->unlock_critera_threshold, S(unlock_item->name.c_str()) ));
					break;
				}
				case SkillUnlockCriteria::Damage_With_Weapon:
				{
					if (item->unlock_critera_threshold < 0)
					{
						description += StringHelper::Format("\n\n%s\n%s", S("#menu_shop_unlock_criteria"), SF("#menu_shop_unlock_heal_from_weapon", abs(item->unlock_critera_threshold), S(unlock_item->name.c_str()) ));
					}
					else
					{
						description += StringHelper::Format("\n\n%s\n%s", S("#menu_shop_unlock_criteria"), SF("#menu_shop_unlock_damage_from_weapon", item->unlock_critera_threshold, S(unlock_item->name.c_str()) ));
					}
					break;
				}
				case SkillUnlockCriteria::Kills_With_Weapon:
				{
					description += StringHelper::Format("\n\n%s\n%s", S("#menu_shop_unlock_criteria"), SF("#menu_shop_unlock_kills_from_weapon", item->unlock_critera_threshold, S(unlock_item->name.c_str()) ));
					break;
				}
			}
		}

		info_description_label->Set_Value(description);

		if (!isUnlocked)
		{
			info_price_label->Set_Value(S("#menu_inventory_locked"));
		}
		else if (item->cost == 0)
		{
			info_price_label->Set_Value("");
		}
		else
		{
			info_price_label->Set_Value(SF("#menu_inventory_skill_points", StringHelper::Format_Number((float)item->cost).c_str()));
		}
	}

	point_label->Set_Value(SF("#menu_inventory_skill_points", StringHelper::Format_Number((float)ceilf(m_display_points)).c_str()));

	m_last_selected_node = selected_node;

	UIScene::Tick(time, manager, scene_index);
}

void UIScene_Skills::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Draw(time, manager, scene_index);
}

void UIScene_Skills::Can_Drag_Drop(UIGridTreeCanDragData* data)
{
	SkillArchetype* item = reinterpret_cast<SkillArchetype*>(data->node->meta_data);
	data->allow_drag = (item != NULL && m_profile->Has_Skill(item) && item->is_passive == false);
}

void UIScene_Skills::Recieve_Event(UIManager* manager, UIEvent e)
{
	switch (e.Type)
	{
	case UIEventType::GridTree_Drop:
		{
			UIGridTree* grid = reinterpret_cast<UIGridTree*>(e.Source);
			UIGridTreeNode* selected_node = grid->Get_Selected_Node();
			SkillArchetype* item = reinterpret_cast<SkillArchetype*>(selected_node->meta_data);
			
			DBG_LOG("Drag-dropped skill '%s'.", item->name.c_str());		
			
			UIPanel* equip_slot_1 = Find_Element<UIPanel*>("equip_slot_1");
			UIPanel* equip_slot_2 = Find_Element<UIPanel*>("equip_slot_2");
			UIPanel* equip_slot_3 = Find_Element<UIPanel*>("equip_slot_3");

			MouseState* mouse = Input::Get(0)->Get_Mouse_State();
			if (equip_slot_1->Get_Screen_Box().Intersects(mouse->Get_Position()))
			{
				DBG_LOG("Dropped onto equip slot 1.");
				m_profile->Equip_Skill(m_profile->Get_Skill(item), 0); 
				manager->Play_UI_Sound(UISoundType::Paint);
			}
			else if (equip_slot_2->Get_Screen_Box().Intersects(mouse->Get_Position()))
			{
				DBG_LOG("Dropped onto equip slot 2.");
				m_profile->Equip_Skill(m_profile->Get_Skill(item), 1);
				manager->Play_UI_Sound(UISoundType::Paint);
			}
			else if (equip_slot_3->Get_Screen_Box().Intersects(mouse->Get_Position()))
			{
				DBG_LOG("Dropped onto equip slot 3.");
				m_profile->Equip_Skill(m_profile->Get_Skill(item), 2);
				manager->Play_UI_Sound(UISoundType::Paint);
			}

			Update_Equip_Items();

			break;
		}
	case UIEventType::GridTree_Item_Click:
		{
			UIGridTree* grid = reinterpret_cast<UIGridTree*>(e.Source);
			UIGridTreeNode* selected_node = grid->Get_Selected_Node();
			SkillArchetype* item = reinterpret_cast<SkillArchetype*>(selected_node->meta_data);

			DBG_LOG("Selected skill '%s'.", item->name.c_str());

			// Not unlocked yet?
			if (!m_profile->Is_Skill_Unlocked(item))
			{
				manager->Go(UIAction::Push(new UIScene_Dialog(S("#menu_skills_purchase_skill_locked")), new UIFadeInTransition()));
			}

			// Already purchased, should we equip?
			else if (m_profile->Has_Skill(item) || item->cost == 0)
			{
				Skill* skill = m_profile->Get_Skill(item);

				if (skill != NULL && skill->was_rolled_back == true)
				{
					DBG_LOG("Attempt to purchase rolled back skill '%s'.", item->name.c_str());
					
					manager->Go(UIAction::Push(new UIScene_Dialog(S("#menu_skills_reroll_cannot_multi_purchase"), S("#menu_continue")), new UIFadeInTransition()));
				}
				else if (item->is_passive == false && item->cost != 0)
				{
					DBG_LOG("Equipping active skill '%s'.", item->name.c_str());

					if (skill->equip_slot < 0)
					{
						m_profile->Equip_Skill(skill, m_profile->Get_Next_Equip_Slot());
					}
				}
				else
				{
					DBG_LOG("Pointlessly selecting skill '%s', ignoring?", item->name.c_str());
				}
			}

			// Have we not unlocked parent?
			else if (item->parent != NULL && item->parent->cost != 0 && !m_profile->Has_Skill(item->parent))
			{
				std::string parent_name = item->parent->Get_Display_Name();
				std::string name = item->Get_Display_Name();

				manager->Go(UIAction::Push(new UIScene_Dialog(SF("#menu_skills_purchase_parent_not_unlocked", parent_name.c_str(), name.c_str()), S("#menu_continue")), new UIFadeInTransition()));
			}

			// Do we have enough points.
			else if (m_profile->Skill_Points < item->cost)
			{
				int required_points = item->cost - m_profile->Skill_Points;
				std::string name = item->Get_Display_Name();
				std::string price = StringHelper::Format_Number((float)abs(required_points));

				manager->Go(UIAction::Push(new UIScene_Dialog(SF("#menu_skills_purchase_not_enough", price.c_str(), name.c_str()), S("#menu_continue")), new UIFadeInTransition()));
			}

			// Is item mutex?
			else if (item->parent != NULL && item->parent->are_children_mutex)
			{
				std::string name = item->Get_Display_Name();
				std::string price = StringHelper::Format_Number((float)item->cost);
				std::string sibling_name = "";

				bool has_unlocked_siblings = false;

				for (std::vector<SkillArchetype*>::iterator iter = item->parent->children.begin(); iter != item->parent->children.end(); iter++)
				{
					SkillArchetype* sibling_item = *iter;
					Skill* skill = m_profile->Get_Skill(sibling_item);
					if (skill != NULL && skill->was_rolled_back == false)
					{
						has_unlocked_siblings = true;
						sibling_name = sibling_item->Get_Display_Name();
						break;
					}
				}
				
				if (has_unlocked_siblings)
				{
					DBG_LOG("Attempted to unlock mutex item '%s' when siblings are already unlocked.", item->name.c_str());				
					manager->Go(UIAction::Push(new UIScene_Dialog(SF("#menu_skills_purchase_sibling_unlocked_mutex", sibling_name.c_str()), S("#menu_continue")), new UIFadeInTransition()));
				}
				else
				{
					DBG_LOG("Showing confirm dialog for mutex item '%s'.", item->name.c_str());				
					manager->Go(UIAction::Push(new UIScene_ConfirmDialog(SF("#menu_skills_purchase_confirm_mutex", name.c_str(), price.c_str()), S("#menu_no"), S("#menu_yes")), new UIFadeInTransition()));				
					m_purchase_item = item;
				}
			}

			// Time to purchase!
			else
			{
				std::string name = item->Get_Display_Name();
				std::string price = StringHelper::Format_Number((float)item->cost);

				DBG_LOG("Showing confirm dialog for normal item '%s'.", item->name.c_str());				
				manager->Go(UIAction::Push(new UIScene_ConfirmDialog(SF("#menu_skills_purchase_confirm", name.c_str(), price.c_str()), S("#menu_no"), S("#menu_yes")), new UIFadeInTransition()));				
			
				m_purchase_item = item;
			}

			Update_Equip_Items();
		}
		break;
	case UIEventType::Dialog_Close:
		{
			if (dynamic_cast<UIScene_ConfirmDialog*>(e.Scene)->Get_Selected_Index() == 1)
			{
				if (m_purchase_item != NULL)
				{
					DBG_LOG("Purchasing item '%s'.", m_purchase_item->name.c_str());	

					m_profile->Unlock_Skill(m_purchase_item);
					m_profile->Skill_Points -= m_purchase_item->cost;

					if (m_purchase_item->is_passive == false)
					{
						m_profile->Equip_Skill(m_profile->Get_Skill(m_purchase_item), m_profile->Get_Next_Equip_Slot());
					}

					m_purchase_item = NULL;
				}
				else if (m_rerolling_skills == true)
				{
					m_profile->Reroll_Skills();
					m_rerolling_skills = false;
				}
			}
			break;
		}
	case UIEventType::Button_Click:
		{	
			if (e.Source->Get_Name() == "back_button")
			{	
				manager->Go(UIAction::Pop(new UISlideInTopLevelTransition()));
			}
			else if (e.Source->Get_Name() == "reroll_button")
			{	
				int total_cost = m_profile->Get_Reroll_Cost();
				if (total_cost > m_profile->Coins)
				{					
					int required_points = total_cost - m_profile->Coins;
					std::string total_price = StringHelper::Format_Number((float)abs(total_cost));
					std::string price = StringHelper::Format_Number((float)abs(required_points));
					
					DBG_LOG("Showing reroll not enough coins dialog.");	
					manager->Go(UIAction::Push(new UIScene_Dialog(SF("#menu_skills_reroll_not_enough", total_price.c_str(), price.c_str()), S("#menu_continue")), new UIFadeInTransition()));
				}
				else
				{	
					std::string total_price = StringHelper::Format_Number((float)abs(total_cost));

					DBG_LOG("Showing reroll confirm dialog.");	
					manager->Go(UIAction::Push(new UIScene_ConfirmDialog(SF("#menu_skills_reroll_confirm", total_price.c_str()), S("#menu_no"), S("#menu_yes")), new UIFadeInTransition()));	
					m_rerolling_skills = true;
				}			
			}
		}
		break;
	}
}

void UIScene_Skills::On_Draw_Item(UIGridTreeDrawItemData* data)
{
	Vector2 ui_scale = data->manager->Get_UI_Scale();
	SkillArchetype* item = reinterpret_cast<SkillArchetype*>(data->node->meta_data);

	ui_scale.X *= 0.75f;
	ui_scale.Y *= 0.75f;
	
	Skill* skill = m_profile->Get_Skill(item);
	bool unlocked = (skill != NULL && skill->was_rolled_back == false) || item->cost == 0;

	if (!m_profile->Is_Skill_Unlocked(item))
	{
		unlocked = false;
	}

	AtlasRenderer atlas_renderer(NULL);
	if (unlocked == true)
	{
		atlas_renderer.Draw_Frame(item->icon, data->item_bounds, 0.0f, Color::White, false, false, RendererOption::E_Src_Alpha_One_Minus_Src_Alpha, Vector2(1.0f, 1.0f));			
	}
	else
	{
		float padding = 3.0f * ui_scale.X;
		atlas_renderer.Draw_Frame(item->icon, data->item_bounds, 0.0f, Color(64, 64, 64, 255), false, false, RendererOption::E_Src_Alpha_One_Minus_Src_Alpha, Vector2(1.0f, 1.0f));			
	}

	float w = data->item_bounds.Width * 0.66f;
	float h = data->item_bounds.Height * 0.66f;
	float p = -2.0f * ui_scale.X;
	Rect2D lock_area = Rect2D(data->item_bounds.X + data->item_bounds.Width - (w + p), data->item_bounds.Y + data->item_bounds.Height - (h + p), w, h);//data->item_bounds.Inflate(-5, -5);

	if (!unlocked)
	{
		// Draw lock!
		atlas_renderer.Draw_Frame("achievement_locked_clear", lock_area, 0.0f, Color(255, 255, 255, 128));
	}
}