// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_INVENTORY_
#define _GAME_UI_SCENES_UISCENE_INVENTORY_

#include "Engine/UI/UIScene.h"

struct Profile;
class UICharacterPreviewRenderer;

class UIScene_Inventory : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_Inventory, "UI");

private:
	Profile* m_profile;
	int m_profile_index;
	bool m_creating;
	
	bool m_showing_name_input_dialog;

	UICharacterPreviewRenderer* m_char_renderer;

protected:
	void Update_Profile(bool final_update);

public:
	UIScene_Inventory(Profile* profile, bool creating, int profile_index = -1);
	~UIScene_Inventory();

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

