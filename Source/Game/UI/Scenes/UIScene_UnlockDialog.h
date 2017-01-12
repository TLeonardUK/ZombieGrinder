// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_ITEMUNLOCKDIALG_
#define _GAME_UI_SCENES_UISCENE_ITEMUNLOCKDIALG_

#include "Engine/UI/UIScene.h"

#include "Engine/Online/OnlineInventory.h"
#include "Game/Profile/Profile.h"

#include "Engine/UI/Elements/UIListView.h"

#include "Generic/Events/Event.h"
#include "Generic/Events/Delegate.h"

struct ItemArchetype;

struct UnlockDialogEntryType
{
	enum Type
	{
		Online_Item_Drop,
		Item_Unpacked,
		Challenge_Reward,
		Item_Unlocked,
		Skill_Unlocked,
		Item_Broken,
	};
};

struct UnlockDialogEntry
{
	UnlockDialogEntryType::Type Type;

	ProfileUnpackEvent Unpack_Event; 
	OnlineInventoryItem Online_Item;

	Item* Item_Instance;

	ItemArchetype* Archetype_For_Item;
	SkillArchetype* Archetype_For_Skill;

	Profile* User_Profile;
};

class UIScene_UnlockDialog : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_UnlockDialog, "UI");

private:
	Delegate<UIScene_UnlockDialog, UIListViewDrawItemData>* m_draw_item_delegate;

	AtlasHandle*	m_atlas;
	AtlasRenderer	m_atlas_renderer;

	FontHandle*		m_font;
	MarkupFontRenderer	m_font_renderer;

	UIFrame			m_item_frame_active;
	UIFrame			m_item_frame_inactive;

	int m_direction;
	float m_direction_change_timer;

	std::vector<UnlockDialogEntry> m_events;

	enum
	{
		DIRECTION_CHANGE_INTERVAL = 300
	};

protected:

public:
	UIScene_UnlockDialog(std::vector<UnlockDialogEntry> events);
	~UIScene_UnlockDialog();

	const char* Get_Name();
	bool Should_Render_Lower_Scenes();
	bool Is_Focusable();
	bool Should_Display_Cursor();
	bool Should_Display_Focus_Cursor();

	bool Can_Accept_Invite();

	void Refresh_Items();
	
	void Refresh(UIManager* manager);

	void Enter(UIManager* manager);
	void Exit(UIManager* manager);

	void Tick(const FrameTime& time, UIManager* manager, int scene_index);
	void Draw(const FrameTime& time, UIManager* manager, int scene_index);

	void On_Draw_Item(UIListViewDrawItemData* data);

	void Recieve_Event(UIManager* manager, UIEvent e);

};

#endif

