// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_ITEMBROKENDIALOG_
#define _GAME_UI_SCENES_UISCENE_ITEMBROKENDIALOG_

#include "Engine/UI/UIScene.h"

#include "Engine/Online/OnlineInventory.h"
#include "Game/Profile/Profile.h"

struct ItemArchetype;

class UIScene_ItemBrokenDialog : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_ItemBrokenDialog, "UI");

private:
	Item* m_item;
	Profile* m_profile;

protected:

public:
	UIScene_ItemBrokenDialog(Item* item, Profile* profile);

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

