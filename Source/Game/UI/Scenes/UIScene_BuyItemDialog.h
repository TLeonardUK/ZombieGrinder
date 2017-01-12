// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_BUY_ITEM_DIALOG_
#define _GAME_UI_SCENES_UISCENE_BUY_ITEM_DIALOG_

#include "Engine/UI/UIScene.h"

#include "Engine/Engine/EngineOptions.h"
#include "Game/Runner/GameOptions.h"

struct ItemArchetype;
struct Profile;

class UIScene_BuyItemDialog : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_BuyItemDialog, "UI");

private:
	ItemArchetype* m_archetype;
	Profile* m_profile;
	bool m_pending_close;

protected:

public:
	UIScene_BuyItemDialog(ItemArchetype* archetype, Profile* profile);

	const char* Get_Name();
	bool Should_Render_Lower_Scenes();
	bool Is_Focusable();
	bool Should_Display_Cursor();
	bool Should_Display_Focus_Cursor();
	bool Should_Fade_Cursor();

	bool Can_Accept_Invite();

	void Refresh_Items();

	void Enter(UIManager* manager);
	void Exit(UIManager* manager);

	void Tick(const FrameTime& time, UIManager* manager, int scene_index);
	void Draw(const FrameTime& time, UIManager* manager, int scene_index);

	void Recieve_Event(UIManager* manager, UIEvent e);

};

#endif

