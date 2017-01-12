// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_CHAT_
#define _GAME_UI_SCENES_UISCENE_CHAT_

#include "Engine/UI/UIScene.h"

class UIScene_Game;
class UITextPanel;

class UIScene_Chat : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_Chat, "UI");

private:
	bool m_as_overlay;
	UIScene_Game* m_game_scene;

	UITextPanel* m_chat_panel;

	int m_last_chat_messages_recieved;

protected:

public:
	UIScene_Chat(UIScene_Game* game, bool is_overlay);
	~UIScene_Chat();

	const char* Get_Name();
	bool Should_Render_Lower_Scenes();	
	bool Should_Render_Lower_Scenes_Background();
	bool Should_Tick_When_Not_Top();
	bool Is_Focusable();
	bool Should_Display_Cursor();
	bool Should_Display_Focus_Cursor();
	
	void Refresh(UIManager* manager);

	void Set_Offset(float offset);
	float Get_Offset();

	void Refresh_History();

	void Enter(UIManager* manager);
	void Exit(UIManager* manager);
	
	void Tick(const FrameTime& time, UIManager* manager, int scene_index);
	void Draw(const FrameTime& time, UIManager* manager, int scene_index);

	void Recieve_Event(UIManager* manager, UIEvent e);

};

#endif

