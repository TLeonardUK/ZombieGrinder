// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_UPGRADE_WEAPON_
#define _GAME_UI_SCENES_UISCENE_UPGRADE_WEAPON_

#include "Engine/Renderer/Text/FontRenderer.h"
#include "Engine/UI/UIScene.h"
#include "Engine/UI/UIFrame.h"
#include "Generic/Types/Color.h"

#include "Engine/UI/Elements/UIGridTree.h"

#include "Generic/Events/Event.h"
#include "Generic/Events/Delegate.h"

#include "Game/Profile/Profile.h"

struct Profile;
struct UIGridDrawItemData;
struct UIGridCanDragData;
struct Item;
class UICharacterPreviewRenderer;
struct ItemUpgradeTreeNode;

struct UIScene_UpgradeWeaponSocketState
{
	ItemUpgradeTreeNode* node;
	bool bPurchased;
	bool bAvailable;

	UIScene_UpgradeWeaponSocketState* UpNodeState;
	UIScene_UpgradeWeaponSocketState* DownNodeState;
	UIScene_UpgradeWeaponSocketState* LeftNodeState;
	UIScene_UpgradeWeaponSocketState* RightNodeState;

	UIGridTreeNode* GridNode;
};

class UIScene_UpgradeWeapon : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_UpgradeWeapon, "UI");

private:
	Delegate<UIScene_UpgradeWeapon, UIGridTreeDrawItemData>* m_draw_item_delegate;
	Delegate<UIScene_UpgradeWeapon, UIGridTreeCanDragData>* m_can_drag_delegate;

	Profile* m_profile;
	Item* m_item;

	ItemArchetype* m_required_gem;
	std::vector<ItemArchetype*> m_gem_series;

	std::vector<UIScene_UpgradeWeaponSocketState> m_states;

	bool m_auto_combine_active;
	ItemArchetype* m_auto_combine_gem;
	int m_auto_combine_upgrade_id;

	UIGridTreeNode* m_last_selected_node;

	bool m_init_enter;

	float m_info_alpha;
	
	bool m_is_max_level;

	enum
	{
		INFO_FADE_IN_DELAY = 500
	};

protected:
	void On_Draw_Item(UIGridTreeDrawItemData* data);
	void Can_Drag_Drop(UIGridTreeCanDragData* data);

	UIScene_UpgradeWeaponSocketState* Find_State_By_Grid_Node(UIGridTreeNode* node);

public:
	UIScene_UpgradeWeapon(Profile* profile, Item* item);
	~UIScene_UpgradeWeapon();

	void Update_Required_Gem(ItemArchetype* item);
	void Update_Item_States();

	const char* Get_Name();
	bool Should_Render_Lower_Scenes();
	bool Is_Focusable();
	bool Should_Display_Cursor();

	void Enter(UIManager* manager);
	void Exit(UIManager* manager);

	void Tick(const FrameTime& time, UIManager* manager, int scene_index);
	void Draw(const FrameTime& time, UIManager* manager, int scene_index);

	void Recieve_Event(UIManager* manager, UIEvent e);

};

#endif

