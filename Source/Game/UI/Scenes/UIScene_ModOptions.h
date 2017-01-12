// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_MOD_OPTIONS_
#define _GAME_UI_SCENES_UISCENE_MOD_OPTIONS_

#include "Engine/UI/UIScene.h"

#include "Engine/UI/Elements/UIListView.h"
#include "Engine/Online/OnlineMatching.h"

class UIScene_ModOptions : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_ModOptions, "UI");

private:
	Delegate<UIScene_ModOptions, UIListViewDrawItemData>* m_draw_item_delegate;

	AtlasHandle*	m_atlas;
	AtlasRenderer	m_atlas_renderer;

	FontHandle*		m_font;
	FontRenderer	m_font_renderer;
	MarkupFontRenderer	m_markup_font_renderer;

	UIFrame			m_item_frame_active;
	UIFrame			m_item_frame_inactive;

	bool			m_mods_changed;

protected:

public:
	UIScene_ModOptions();

	const char* Get_Name();
	bool Should_Render_Lower_Scenes();
	bool Is_Focusable();
	bool Should_Display_Cursor();

	UIScene* Get_Background(UIManager* manager);

	void Refresh_Items(bool bResetSliderToStart);

	void Refresh(UIManager* manager);

	void Enter(UIManager* manager);
	void Exit(UIManager* manager);

	void Tick(const FrameTime& time, UIManager* manager, int scene_index);
	void Draw(const FrameTime& time, UIManager* manager, int scene_index);

	void On_Draw_Item(UIListViewDrawItemData* data);

	void Recieve_Event(UIManager* manager, UIEvent e);

};

#endif

