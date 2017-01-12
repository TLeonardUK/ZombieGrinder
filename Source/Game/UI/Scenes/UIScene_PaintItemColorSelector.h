// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_PAINT_ITEM_COLOR_SELECTOR_
#define _GAME_UI_SCENES_UISCENE_PAINT_ITEM_COLOR_SELECTOR_

#include "Engine/UI/UIScene.h"

#include "Generic/Types/Color.h"

struct Item;

class UIScene_PaintItemColorSelector : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_PaintItemColorSelector, "UI");

private:
	Color* m_color;
	Item* m_preview_item;

	int m_direction;
	float m_direction_timer;

	int m_selected_option;

protected:
	void Update_Preview();

public:
	UIScene_PaintItemColorSelector(Color* color, Item* preview_item);

	int Get_Selected_Index();

	const char* Get_Name();
	bool Should_Render_Lower_Scenes();
	bool Is_Focusable();
	bool Should_Display_Cursor();

	bool Can_Accept_Invite();

	void Refresh_Items();

	void Enter(UIManager* manager);
	void Exit(UIManager* manager);

	void Tick(const FrameTime& time, UIManager* manager, int scene_index);
	void Draw(const FrameTime& time, UIManager* manager, int scene_index);

	void Recieve_Event(UIManager* manager, UIEvent e);

};

#endif

