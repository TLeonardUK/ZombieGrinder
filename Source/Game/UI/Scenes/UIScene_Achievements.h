// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_ACHIEVEMENTS_
#define _GAME_UI_SCENES_UISCENE_ACHIEVEMENTS_

#include "Engine/UI/UIScene.h"

#include "Engine/UI/Elements/UIListView.h"

#include "Generic/Events/Event.h"
#include "Generic/Events/Delegate.h"

class UIScene_Achievements : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_Achievements, "UI");

private:
	Delegate<UIScene_Achievements, UIListViewDrawItemData>* m_draw_item_delegate;
	
	AtlasHandle*	m_atlas;
	AtlasRenderer	m_atlas_renderer;
	
	FontHandle*		m_font;
	FontRenderer	m_font_renderer;
	MarkupFontRenderer	m_shadow_font_renderer;
	UIFrame			m_item_frame_active;
	UIFrame			m_item_frame_inactive;

protected:
	void Refresh_Items();

public:
	UIScene_Achievements();

	const char* Get_Name();
	bool Should_Render_Lower_Scenes();
	bool Is_Focusable();
	bool Should_Display_Cursor();
	
	void Refresh(UIManager* manager);

	void Enter(UIManager* manager);
	void Exit(UIManager* manager);
	
	void Tick(const FrameTime& time, UIManager* manager, int scene_index);
	void Draw(const FrameTime& time, UIManager* manager, int scene_index);
		
	void On_Draw_Item(UIListViewDrawItemData* data);

	void Recieve_Event(UIManager* manager, UIEvent e);

};

#endif

