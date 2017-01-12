// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_SELECT_PROFILE_
#define _GAME_UI_SCENES_UISCENE_SELECT_PROFILE_

#include "Engine/UI/UIScene.h"

#include "Engine/UI/Elements/UIListView.h"

#include "Generic/Events/Event.h"
#include "Generic/Events/Delegate.h"

class UICharacterPreviewRenderer;

class UIScene_SelectProfile : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_SelectProfile, "UI");

private:
	Delegate<UIScene_SelectProfile, UIListViewDrawItemData>* m_draw_item_delegate;
	
	AtlasHandle*	m_atlas;
	AtlasRenderer	m_atlas_renderer;
	
	FontHandle*		m_font;
	FontRenderer	m_font_renderer;

	UIFrame			m_item_frame_active;
	UIFrame			m_item_frame_inactive;

	bool			m_delete_confirm_dialog_open;
	int				m_delete_profile_index;

	bool			m_inventory_open;
	int				m_selected_profile_index;

	UICharacterPreviewRenderer*	m_char_renderer;

protected:	
	void Create_Profile();
	void Select_Profile(int index);
	void Delete_Profile(int index);

	void Refresh_Items();

public:
	UIScene_SelectProfile();
	~UIScene_SelectProfile();

	int Get_Selected_Index();
	void Cancel();

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

