// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_INVENTORYDROPDIALOG_
#define _GAME_UI_SCENES_UISCENE_INVENTORYDROPDIALOG_

#include "Engine/UI/UIScene.h"

#include "Engine/Online/OnlineInventory.h"
#include "Game/Profile/Profile.h"

struct ItemArchetype;

class UIScene_InventoryDropDialog : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_InventoryDropDialog, "UI");

private:
	OnlineInventoryItem m_item;
	ItemArchetype* m_archetype;
	bool m_use_online_item;
	bool m_use_unpack_event;
	ProfileUnpackEvent m_unpack_event;

protected:

public:
	UIScene_InventoryDropDialog(OnlineInventoryItem item);
	UIScene_InventoryDropDialog(ItemArchetype* type);
	UIScene_InventoryDropDialog(ProfileUnpackEvent evt);
	
	const char* Get_Name();
	bool Should_Render_Lower_Scenes();
	bool Is_Focusable();
	bool Should_Display_Cursor();
	bool Should_Display_Focus_Cursor();

	bool Can_Accept_Invite();

	void Refresh_Items();

	void Enter(UIManager* manager);
	void Exit(UIManager* manager);

	void Tick(const FrameTime& time, UIManager* manager, int scene_index);
	void Draw(const FrameTime& time, UIManager* manager, int scene_index);

	void Recieve_Event(UIManager* manager, UIEvent e);

};

#endif

