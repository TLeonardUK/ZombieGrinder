// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_UpgradeWeapon.h"
#include "Game/UI/Scenes/UIScene_Dialog.h"
#include "Game/UI/Scenes/UIScene_ConfirmDialog.h"
#include "Game/UI/Scenes/UIScene_InventoryDropDialog.h"
#include "Game/UI/Scenes/UIScene_PaintItemColorSelector.h"
#include "Game/UI/Scenes/UIScene_UnlockDialog.h"
#include "Game/UI/Scenes/UIScene_Equipment.h"
#include "Game/UI/Scenes/Helper/UICharacterPreviewRenderer.h"
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
#include "Engine/UI/Elements/UIGrid.h"
#include "Engine/UI/Elements/UIGridTree.h"
#include "Engine/UI/Elements/UIProgressBar.h"
#include "Engine/UI/Elements/UIDropDownMenu.h"
#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Resources/ResourceFactory.h"

#include "Engine/Online/OnlineInventory.h"

#include "Engine/Audio/AudioRenderer.h"

#include "Engine/Localise/Locale.h"
#include "Engine/Engine/GameEngine.h"

#include "Game/UI/Scenes/UIScene_Game.h"

#include "Generic/Math/Random.h"

#include "Game/Runner/Game.h"
#include "Engine/Engine/EngineOptions.h"
#include "Game/Runner/GameOptions.h"

#include "Game/Profile/ProfileManager.h"
#include "Game/Profile/ItemManager.h"

#include "XScript/VirtualMachine/CSymbol.h"
#include "XScript/VirtualMachine/CVMBinary.h"

#include "Generic/Helper/PersistentLogHelper.h"

UIScene_UpgradeWeapon::UIScene_UpgradeWeapon(Profile* profile, Item* item)
	: m_profile(profile)
	, m_item(item)
	, m_required_gem(NULL)
	, m_auto_combine_active(false)
	, m_init_enter(false)
	, m_info_alpha(0.0f)
	, m_last_selected_node(NULL)
	, m_is_max_level(true)
{
	Set_Layout("upgrade_weapon");

	m_draw_item_delegate = new Delegate<UIScene_UpgradeWeapon, UIGridTreeDrawItemData>(this, &UIScene_UpgradeWeapon::On_Draw_Item);
	m_can_drag_delegate = new Delegate<UIScene_UpgradeWeapon, UIGridTreeCanDragData>(this, &UIScene_UpgradeWeapon::Can_Drag_Drop);

	UIGridTree* grid_view = Find_Element<UIGridTree*>("upgrade_view");
	grid_view->On_Draw_Item += m_draw_item_delegate;
	grid_view->On_Can_Drag += m_can_drag_delegate;
}

UIScene_UpgradeWeapon::~UIScene_UpgradeWeapon()
{
	SAFE_DELETE(m_draw_item_delegate);
	SAFE_DELETE(m_can_drag_delegate);
}

const char* UIScene_UpgradeWeapon::Get_Name()
{
	return "UIScene_UpgradeWeapon";
}

bool UIScene_UpgradeWeapon::Should_Render_Lower_Scenes()
{
	return false;
}

bool UIScene_UpgradeWeapon::Should_Display_Cursor()
{
	return true;
}

bool UIScene_UpgradeWeapon::Is_Focusable()
{
	return true;
}

void UIScene_UpgradeWeapon::Enter(UIManager* manager)
{
	if (m_init_enter)
	{
		return;
	}
	m_init_enter = true;

	UILabel* name_label = Find_Element<UILabel*>("name_box");
	name_label->Set_Value(m_item->Get_Display_Name().c_str());

	UIGridTree* grid_panel = Find_Element<UIGridTree*>("upgrade_view");
	grid_panel->Clear_Nodes();

	ItemUpgradeTree* tree = m_item->archetype->Get_Upgrade_Tree();
	std::vector<ItemUpgradeTreeNode*> nodes;
	tree->Get_Nodes(nodes);

	/*
	if (m_profile->Get_Items().size() < 600)
	{
		std::vector<ItemArchetype*> archetypes = ItemManager::Get()->Get_Archetypes();
		for (std::vector<ItemArchetype*>::iterator iter = archetypes.begin(); iter != archetypes.end(); iter++)
		{
			Item* item = m_profile->Add_Item(*iter);
			item->Fully_Upgrade();
		}
	}
	*/

	/*
	for (int i = 0; i < 64; i++)
	{
		m_profile->Add_Item(ItemManager::Get()->Find_Archetype("Item_Aqua_Gem_1"));
		m_profile->Add_Item(ItemManager::Get()->Find_Archetype("Item_Blue_Gem_1"));
		m_profile->Add_Item(ItemManager::Get()->Find_Archetype("Item_Green_Gem_1"));
		m_profile->Add_Item(ItemManager::Get()->Find_Archetype("Item_Orange_Gem_1"));
		m_profile->Add_Item(ItemManager::Get()->Find_Archetype("Item_Pink_Gem_1"));
		m_profile->Add_Item(ItemManager::Get()->Find_Archetype("Item_Purple_Gem_1"));
		m_profile->Add_Item(ItemManager::Get()->Find_Archetype("Item_Red_Gem_1"));
		m_profile->Add_Item(ItemManager::Get()->Find_Archetype("Item_Yellow_Gem_1"));
	}*/

	/*
	static bool bTest = false;
	if (!bTest)
	{
		m_profile->Add_Item(ItemManager::Get()->Find_Archetype("Item_Pink_Gem_1"));
		m_profile->Add_Item(ItemManager::Get()->Find_Archetype("Item_Green_Gem_1"));
		m_profile->Add_Item(ItemManager::Get()->Find_Archetype("Item_Blue_Gem_1"));
		m_profile->Add_Item(ItemManager::Get()->Find_Archetype("Item_Purple_Gem_1"));
		bTest = true;
	}
	*/

	m_states.reserve(nodes.size());

	UIGridTreeNode* root = NULL;

	for (std::vector<ItemUpgradeTreeNode*>::iterator iter = nodes.begin(); iter != nodes.end(); iter++)
	{
		ItemUpgradeTreeNode* node = *iter;
		if (node->gem_type == NULL)
		{
			UIScene_UpgradeWeaponSocketState state;
			state.node = node;
			state.bAvailable = false;
			state.bPurchased = false;
			m_states.push_back(state);

			UIScene_UpgradeWeaponSocketState* state_ptr = &m_states[m_states.size() - 1];

			root = grid_panel->Add_Node(state_ptr, NULL, false, node->x, node->y, node->bConnectLeft, node->bConnectUp, node->bConnectRight, node->bConnectDown);

			state_ptr->GridNode = root;
		}
	}

	for (std::vector<ItemUpgradeTreeNode*>::iterator iter = nodes.begin(); iter != nodes.end(); iter++)
	{
		ItemUpgradeTreeNode* node = *iter;
		if (node->gem_type != NULL)
		{
			UIScene_UpgradeWeaponSocketState state;
			state.node = node;
			state.bAvailable = false;
			state.bPurchased = false;
			m_states.push_back(state);

			UIScene_UpgradeWeaponSocketState* state_ptr = &m_states[m_states.size() - 1];

			root = grid_panel->Add_Node(state_ptr, root, false, node->x, node->y, node->bConnectLeft, node->bConnectUp, node->bConnectRight, node->bConnectDown);

			state_ptr->GridNode = root;
		}
	}

	grid_panel->Refresh_Tree();
	Update_Item_States();
}

void UIScene_UpgradeWeapon::Exit(UIManager* manager)
{
}

UIScene_UpgradeWeaponSocketState* UIScene_UpgradeWeapon::Find_State_By_Grid_Node(UIGridTreeNode* node)
{
	for (std::vector<UIScene_UpgradeWeaponSocketState>::iterator iter = m_states.begin(); iter != m_states.end(); iter++)
	{
		UIScene_UpgradeWeaponSocketState& state = *iter;
		if (state.GridNode == node)
		{
			return &state;
		}
	}

	return NULL;
}

void UIScene_UpgradeWeapon::Update_Item_States()
{
	UIGridTree* grid_panel = Find_Element<UIGridTree*>("upgrade_view");

	std::vector<int> upgrade_ids;
	m_item->Get_Upgrade_Ids(upgrade_ids);

	for (std::vector<UIScene_UpgradeWeaponSocketState>::iterator iter = m_states.begin(); iter != m_states.end(); iter++)
	{
		UIScene_UpgradeWeaponSocketState& state = *iter;	
		state.bPurchased = (std::find(upgrade_ids.begin(), upgrade_ids.end(), state.node->id) != upgrade_ids.end());
		if (state.node->gem_type == NULL)
		{
			state.bPurchased = true; // root node is always purchased.
		}

		state.bAvailable = false;

		UIGridTreeNode* grid_node = NULL;
		
		if (state.node->bConnectDown)
		{
			grid_panel->Get_Below_Node(state.GridNode->x, state.GridNode->y, grid_node, state.GridNode);
			state.DownNodeState = Find_State_By_Grid_Node(grid_node);
		}
		else
		{
			state.DownNodeState = NULL;
		}

		if (state.node->bConnectUp)
		{
			grid_panel->Get_Above_Node(state.GridNode->x, state.GridNode->y, grid_node, state.GridNode);
			state.UpNodeState = Find_State_By_Grid_Node(grid_node);
		}
		else
		{
			state.UpNodeState = NULL;
		}

		if (state.node->bConnectLeft)
		{
			grid_panel->Get_Left_Node(state.GridNode->x, state.GridNode->y, grid_node, state.GridNode);
			state.LeftNodeState = Find_State_By_Grid_Node(grid_node);
		}
		else
		{
			state.LeftNodeState = NULL;
		}

		if (state.node->bConnectRight)
		{
			grid_panel->Get_Right_Node(state.GridNode->x, state.GridNode->y, grid_node, state.GridNode);
			state.RightNodeState = Find_State_By_Grid_Node(grid_node);
		}
		else
		{
			state.RightNodeState = NULL;
		}
	}

	for (std::vector<UIScene_UpgradeWeaponSocketState>::iterator iter = m_states.begin(); iter != m_states.end(); iter++)
	{
		UIScene_UpgradeWeaponSocketState& state = *iter;
		if (!state.node->gem_type)
		{
			state.bAvailable = true;
			state.bPurchased = true;
		}
		else if (state.bPurchased)
		{
			if (state.UpNodeState)
			{
				state.UpNodeState->bAvailable = true;
			}
			if (state.DownNodeState)
			{
				state.DownNodeState->bAvailable = true;
			}
			if (state.LeftNodeState)
			{
				state.LeftNodeState->bAvailable = true;
			}
			if (state.RightNodeState)
			{
				state.RightNodeState->bAvailable = true;
			}
		}
	}

	for (std::vector<UIScene_UpgradeWeaponSocketState>::iterator iter = m_states.begin(); iter != m_states.end(); iter++)
	{
		UIScene_UpgradeWeaponSocketState& state = *iter;
		
		if ((state.UpNodeState != NULL && state.UpNodeState->bPurchased) ||
			(state.DownNodeState != NULL && state.DownNodeState->bPurchased) ||
			(state.LeftNodeState != NULL && state.LeftNodeState->bPurchased) ||
			(state.RightNodeState != NULL && state.RightNodeState->bPurchased))
		{
			state.bAvailable = true;
		}

		state.GridNode->connect_left_enabled = 
			(state.LeftNodeState != NULL && state.LeftNodeState->bPurchased) ||
			((state.LeftNodeState != NULL && state.LeftNodeState->bAvailable) && state.bAvailable);

		state.GridNode->connect_right_enabled = state.bPurchased;

		state.GridNode->connect_top_enabled = (state.UpNodeState != NULL && state.UpNodeState->bAvailable) && state.bAvailable;
		state.GridNode->connect_down_enabled = (state.DownNodeState != NULL && state.DownNodeState->bAvailable) && state.bAvailable;
	}

	GameEngine::Get()->Get_UIManager()->Get_Scene_By_Type<UIScene_Equipment*>()->Refresh_Item_List();
}

void UIScene_UpgradeWeapon::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIPanel* info_panel = Find_Element<UIPanel*>("info_panel");
	UILabel* info_name_label = Find_Element<UILabel*>("info_name_label");
	UILabel* info_price_label = Find_Element<UILabel*>("info_price_label");
	UILabel* info_description_label = Find_Element<UILabel*>("info_description_label");
	UIProgressBar* upgrade_progress_bar = Find_Element<UIProgressBar*>("upgrade_box");
	UILabel* upgrade_progress_label = Find_Element<UILabel*>("upgrade_text");

	UILabel* name_label = Find_Element<UILabel*>("name_box");
	name_label->Set_Value(m_item->Get_Display_Name().c_str());

	UIGridTree* grid = Find_Element<UIGridTree*>("upgrade_view");
 	UIGridTreeNode* selected_node = grid->Get_Selected_Node();
	if (selected_node)
	{
		UIScene_UpgradeWeaponSocketState* upgrade_node = static_cast<UIScene_UpgradeWeaponSocketState*>(selected_node->meta_data);
		ItemArchetype* type = ItemManager::Get()->Find_Archetype(upgrade_node->node->gem_type);
		if (type)
		{
			Update_Required_Gem(type);
		}
		else
		{
			m_gem_series.clear();
		}
	}
	else
	{
		m_gem_series.clear();
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
	Vector2 ui_scale = manager->Get_UI_Scale();

	float screen_width = (float)GfxDisplay::Get()->Get_Width();
	float screen_height = (float)GfxDisplay::Get()->Get_Height();

	Rect2D item_box = grid->Get_Selected_Item_Box();
	Rect2D info_box = info_panel->Get_Screen_Box();
	Rect2D grid_box = grid->Get_Screen_Box();

	bool left = (item_box.X + (item_box.Width * 0.5f)) <= (screen_width * 0.5f);
	bool top = (item_box.Y + (item_box.Height * 0.5f)) <= (screen_height * 0.5f);

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

	// Show info.
	if (selected_node != m_last_selected_node && selected_node != NULL)
	{
		UIScene_UpgradeWeaponSocketState* node = reinterpret_cast<UIScene_UpgradeWeaponSocketState*>(selected_node->meta_data);
		if (node->node->gem_type)
		{
			ItemArchetype* gem_archetype = ItemManager::Get()->Find_Archetype(node->node->gem_type);

			info_name_label->Set_Value(SF("#menu_upgrade_socket_name", gem_archetype->Get_Base_Display_Name().c_str()));

			std::string description = "";
			std::vector<ItemUpgradeTreeNodeModifier*> modifiers;
			node->node->Get_Modifiers(modifiers);

			for (std::vector<ItemUpgradeTreeNodeModifier*>::iterator iter = modifiers.begin(); iter != modifiers.end(); iter++)
			{
				ItemUpgradeTreeNodeModifier* mod = *iter;
				description += mod->To_Description_String() + "\n";
			}

			if (node->node->cost > 0)
			{
				description += SF("#menu_upgrade_socket_cost", StringHelper::Format_Number((float)node->node->cost, true).c_str());
			}
			else
			{
				description += SF("#menu_upgrade_socket_cost_negative", StringHelper::Format_Number((float)abs(node->node->cost), true).c_str());
			}
			description += "\n";

			info_description_label->Set_Value(description);

			if (!node->bAvailable)
			{
				info_price_label->Set_Value(S("#menu_inventory_locked"));
			}
			else 
			{
				info_price_label->Set_Value("");
			}
		}
		else
		{
			std::string name = m_item->Get_Display_Name();

			// Figure out description based on cumulative upgrade tree scales.
			std::string description = m_item->archetype->Get_Description(m_item->indestructable) + "\n\n";
			std::vector<ItemUpgradeTreeNodeModifier> modifiers;
			std::vector<int> upgrade_ids;

			m_item->Get_Upgrade_Ids(upgrade_ids);
			m_item->archetype->Get_Upgrade_Tree()->Get_Active_Compound_Modifiers(modifiers, upgrade_ids);

			for (std::vector<ItemUpgradeTreeNodeModifier>::iterator iter = modifiers.begin(); iter != modifiers.end(); iter++)
			{
				ItemUpgradeTreeNodeModifier& mod = *iter;
				description += mod.To_Description_String() + "\n";
			}

			info_name_label->Set_Value(name);
			info_description_label->Set_Value(description.c_str());
			info_price_label->Set_Value("");
		}
	}

	// Show level information.
	const int DEFAULT_MAX_LEVEL = 10;

	int current_level = 0;
	int max_level = DEFAULT_MAX_LEVEL;

	std::vector<int> upgrade_ids;
	m_item->Get_Upgrade_Ids(upgrade_ids);

	std::vector<ItemUpgradeTreeNode*> upgrade_nodes;
	m_item->archetype->Get_Upgrade_Tree()->Get_Nodes(upgrade_nodes);
	
	for (std::vector<int>::iterator iter = upgrade_ids.begin(); iter != upgrade_ids.end(); iter++)
	{
		int id = *iter;
		ItemUpgradeTreeNode* node = m_item->archetype->Get_Upgrade_Tree()->Get_Node(id);
		if (node)
		{
			if (node->cost < 0)
			{
				max_level -= node->cost;
			}
			else
			{
				current_level += node->cost;
			}
		}
	}

	int new_current_level = current_level;
	int new_max_level = max_level;

	UIScene_UpgradeWeaponSocketState* node = reinterpret_cast<UIScene_UpgradeWeaponSocketState*>(selected_node->meta_data);
	if (node->node->gem_type)
	{
		if (std::find(upgrade_ids.begin(), upgrade_ids.end(), node->node->id) == upgrade_ids.end())
		{
			if (node->node->cost < 0)
			{
				new_max_level -= node->node->cost;
			}
			else
			{
				new_current_level += node->node->cost;
			}			
		}
	}

	max_level = Min(max_level, (int)upgrade_nodes.size() - 1);
	current_level = Min(current_level, (int)upgrade_nodes.size() - 1);
	new_max_level = Min(new_max_level, (int)upgrade_nodes.size() - 1);
	new_current_level = Min(new_current_level, (int)upgrade_nodes.size() - 1);

	upgrade_progress_bar->Set_Progress((float)current_level / (float)max_level);
	upgrade_progress_bar->Set_Pending_Progress((float)new_current_level / (float)new_max_level);
	upgrade_progress_label->Set_Value(SF("#menu_upgrade_level_text", current_level, max_level));

	m_is_max_level = (current_level >= max_level);

	m_last_selected_node = selected_node;

	UIScene::Tick(time, manager, scene_index);
}

void UIScene_UpgradeWeapon::Update_Required_Gem(ItemArchetype* item)
{
	m_required_gem = item;
	m_gem_series.clear();

	std::string base_name = item->script_object.Get().Get()->Get_Symbol()->symbol->name;
	base_name = base_name.substr(0, base_name.find_last_of('_'));

	m_gem_series.push_back(ItemManager::Get()->Find_Archetype(StringHelper::Format("%s_1", base_name.c_str()).c_str()));
	m_gem_series.push_back(ItemManager::Get()->Find_Archetype(StringHelper::Format("%s_2", base_name.c_str()).c_str()));
	m_gem_series.push_back(ItemManager::Get()->Find_Archetype(StringHelper::Format("%s_3", base_name.c_str()).c_str()));
	m_gem_series.push_back(ItemManager::Get()->Find_Archetype(StringHelper::Format("%s_4", base_name.c_str()).c_str()));
	m_gem_series.push_back(ItemManager::Get()->Find_Archetype(StringHelper::Format("%s_5", base_name.c_str()).c_str()));
}

void UIScene_UpgradeWeapon::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Draw(time, manager, scene_index);

	UIPanel* gem_panel = Find_Element<UIPanel*>("gem_box");
	Rect2D gem_area = gem_panel->Get_Screen_Box();

	float ui_scale = manager->Get_UI_Scale().Y;

	float padding = 2.0f * ui_scale;
	float text_padding = 1.0f * ui_scale;
	float size = gem_area.Height - (padding * 2.0f);

	int total_items = 5;

	float x_offset = gem_area.X + gem_area.Width - size - padding;
	float y_offset = gem_area.Y + padding;

	PrimitiveRenderer pr;
	FontRenderer fr(manager->Get_Font(), false, true);
	AtlasRenderer ar;

	for (unsigned int i = 0; i < m_gem_series.size(); i++)
	{
		ItemArchetype* archetype = m_gem_series[m_gem_series.size() - (i + 1)];
		int total_items_avail = m_profile->Get_Item_Count(archetype);

		std::string value = StringHelper::Format("x %i", total_items_avail);

		Rect2D area(x_offset, y_offset, size, size);
		ar.Draw_Frame(archetype->icon_animations[0]->Frames[0], area, 0.0f, total_items_avail <= 0.0f ? Color::Gray : Color::White);

		Rect2D text_bounds(area.X + text_padding, area.Y + text_padding, area.Width - (text_padding * 2), area.Height - (text_padding * 2));

		fr.Draw_String(		
			value.c_str(), 
			text_bounds,
			8.0f,
			Color::White,
			TextAlignment::Right,
			TextAlignment::Bottom, 
			manager->Get_UI_Scale() * 0.5f);

		x_offset -= (size + padding);
	}
}

void UIScene_UpgradeWeapon::Recieve_Event(UIManager* manager, UIEvent e)
{
	switch (e.Type)
	{
	case UIEventType::Dialog_Close:
		{
			UIScene_ConfirmDialog* src = static_cast<UIScene_ConfirmDialog*>(e.Scene);
			if (m_auto_combine_active == true)
			{
				if (src->Get_Selected_Index() == 0)
				{
					Item* energy = m_profile->Combo_Create(m_auto_combine_gem);
					m_profile->Remove_Item(energy, true);

					m_item->Add_Upgrade_Id(m_auto_combine_upgrade_id);

					Update_Item_States();
				}

				manager->Get_Scene_By_Type<UIScene_Equipment*>()->Refresh_Item_List();

				m_auto_combine_active = false;
			}
			break;
		}
	case UIEventType::GridTree_Item_Click:
		{
			UIGridTree* grid = reinterpret_cast<UIGridTree*>(e.Source);
			UIGridTreeNode* selected_node = grid->Get_Selected_Node();
			UIScene_UpgradeWeaponSocketState* node = reinterpret_cast<UIScene_UpgradeWeaponSocketState*>(selected_node->meta_data);

			DBG_LOG("Selected node '%s'.", node->node->name.c_str());

			ItemArchetype* gem_archetype = ItemManager::Get()->Find_Archetype(node->node->gem_type);
			bool bHasEnergy = node->node->gem_type ? m_profile->Has_Item(gem_archetype) : false;
			bool bHasEnergyCombo = m_profile->Can_Combo_Create(gem_archetype);

			if (!node->node->gem_type)
			{
				manager->Play_UI_Sound(UISoundType::Back);
			}
			else if (node->bPurchased)
			{
				manager->Go(UIAction::Push(new UIScene_Dialog(S("#menu_upgrade_multi_purchase")), new UIFadeInTransition()));
				manager->Play_UI_Sound(UISoundType::Back);
			}

			else if (!node->bAvailable)
			{
				manager->Go(UIAction::Push(new UIScene_Dialog(S("#menu_upgrade_not_available")), new UIFadeInTransition()));
				manager->Play_UI_Sound(UISoundType::Back);
			}
			else if (m_is_max_level && node->node->cost > 0)
			{
				manager->Go(UIAction::Push(new UIScene_Dialog(S("#menu_upgrade_max_level")), new UIFadeInTransition()));
				manager->Play_UI_Sound(UISoundType::Back);
			}
			else if (!bHasEnergy)
			{
				if (!bHasEnergyCombo)
				{
					manager->Go(UIAction::Push(new UIScene_Dialog(S("#menu_upgrade_no_energy")), new UIFadeInTransition()));
				}
				else
				{
					m_auto_combine_active = true;
					m_auto_combine_gem = gem_archetype;
					m_auto_combine_upgrade_id = node->node->id;
					manager->Go(UIAction::Push(new UIScene_ConfirmDialog(S("#menu_upgrade_no_energy_can_combo"), S("#menu_yes"), S("#menu_no")), new UIFadeInTransition()));
				}
				manager->Play_UI_Sound(UISoundType::Back);
			}
			else
			{
				manager->Play_UI_Sound(UISoundType::Thud);

				Item* energy = m_profile->Get_Item(gem_archetype);
				m_profile->Remove_Item(energy, true);

				m_item->Add_Upgrade_Id(node->node->id);
			}

			manager->Get_Scene_By_Type<UIScene_Equipment*>()->Refresh_Item_List();
			Update_Item_States();

			break;
		}
	case UIEventType::Button_Click:
		{
			if (e.Source->Get_Name() == "back_button")
			{
				manager->Go(UIAction::Pop(new UISlideInTopLevelTransition()));
			}
			break;
		}
	}
}

void UIScene_UpgradeWeapon::Can_Drag_Drop(UIGridTreeCanDragData* data)
{
	data->allow_drag = false;
	return;
}

void UIScene_UpgradeWeapon::On_Draw_Item(UIGridTreeDrawItemData* data)
{
	Vector2 ui_scale = data->ui_scale;
	Vector2 icon_scale = data->ui_scale * Vector2(0.66f, 0.66f);

	UIScene_UpgradeWeaponSocketState* node = reinterpret_cast<UIScene_UpgradeWeaponSocketState*>(data->node->meta_data);

	// Draw background.
	AtlasRenderer ar;
	Rect2D icon_area = data->item_bounds.Inflate(-8.0f * ui_scale.X, -8.0f * ui_scale.Y);

	// Root node.
	if (node->node->gem_type == NULL)
	{
		// Draw background.
		ar.Draw_Frame("screen_main_root_item_upgrade_socket", data->item_bounds, 0.0f, Color::White);

		Vector2 icon_position
		(
			data->item_bounds.X + ((data->item_bounds.Width * 0.5f) - (m_item->archetype->icon_animations[0]->Frames[0]->Rect.Width * icon_scale.X * 0.5f)),
			data->item_bounds.Y + ((data->item_bounds.Height * 0.5f) - (m_item->archetype->icon_animations[0]->Frames[0]->Rect.Height * icon_scale.Y * 0.5f))
		);

		if (m_item->archetype->is_tintable)
		{
			ar.Draw_Frame(m_item->archetype->icon_animations[0]->Frames[0], icon_position, 0.0f, m_item->primary_color, false, false, RendererOption::E_Src_Alpha_One_Minus_Src_Alpha, icon_scale);
			ar.Draw_Frame(m_item->archetype->icon_tint_animations[0]->Frames[0], icon_position, 0.0f, m_item->primary_color, false, false, RendererOption::E_Src_Alpha_One_Minus_Src_Alpha, icon_scale);
		}
		else
		{
			ar.Draw_Frame(m_item->archetype->icon_animations[0]->Frames[0], icon_position, 0.0f, m_item->primary_color, false, false, RendererOption::E_Src_Alpha_One_Minus_Src_Alpha, icon_scale);
		}
	}

	// Gem socket.
	else
	{
		ItemArchetype* archetype = ItemManager::Get()->Find_Archetype(node->node->gem_type);

		Color bg_color = Color::White;
		Color fore_color = Color::White;

		bg_color = archetype->default_tint;

		if (!node->bAvailable)
		{
			bg_color = Color(bg_color.R * 0.1f, bg_color.G * 0.1f, bg_color.B * 0.1f, bg_color.A * 1.0f);
			fore_color = Color(fore_color.R * 0.1f, fore_color.G * 0.1f, fore_color.B * 0.1f, fore_color.A * 1.0f);
		}
		else if (!node->bPurchased)
		{
			bg_color = Color(bg_color.R * 0.5f, bg_color.G * 0.5f, bg_color.B * 0.5f, bg_color.A * 1.0f);
			fore_color = Color(fore_color.R * 0.1f, fore_color.G * 0.1f, fore_color.B * 0.1f, fore_color.A * 1.0f);
		}

		// Draw gem type.
		ar.Draw_Frame("screen_main_item_upgrade_socket", data->item_bounds, 0.0f, bg_color);

		Vector2 icon_position
		(
			data->item_bounds.X + ((data->item_bounds.Width * 0.5f) - (archetype->icon_animations[0]->Frames[0]->Rect.Width * icon_scale.X * 0.5f)),
			data->item_bounds.Y + ((data->item_bounds.Height * 0.5f) - (archetype->icon_animations[0]->Frames[0]->Rect.Height * icon_scale.Y * 0.5f))
			);

		ar.Draw_Frame(archetype->icon_animations[0]->Frames[0], icon_position, 0.0f, fore_color, false, false, RendererOption::E_Src_Alpha_One_Minus_Src_Alpha, icon_scale);
	}

}
