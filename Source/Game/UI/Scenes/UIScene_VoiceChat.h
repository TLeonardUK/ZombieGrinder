// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_VOICECHAT_
#define _GAME_UI_SCENES_UISCENE_VOICECHAT_

#include "Engine/UI/UIScene.h"

class UIScene_Game;
class UITextPanel;

class UIScene_VoiceChat : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_VoiceChat, "UI");

private:

protected:

public:
	UIScene_VoiceChat();
	~UIScene_VoiceChat();

	const char* Get_Name();
	bool Should_Render_Lower_Scenes();	
	bool Should_Render_Lower_Scenes_Background();
	bool Should_Tick_When_Not_Top();
	bool Is_Focusable();
	bool Should_Display_Cursor();
	bool Should_Display_Focus_Cursor();
	
	void Update_List();

	void Refresh(UIManager* manager);

	void Enter(UIManager* manager);
	void Exit(UIManager* manager);
	
	void Tick(const FrameTime& time, UIManager* manager, int scene_index);
	void Draw(const FrameTime& time, UIManager* manager, int scene_index);

	void Recieve_Event(UIManager* manager, UIEvent e);

};

#endif

