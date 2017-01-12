// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_INPUT_OPTIONS_
#define _GAME_UI_SCENES_UISCENE_INPUT_OPTIONS_

#include "Engine/UI/UIScene.h"
#include "Engine/UI/Elements/UITextPanel.h"

#include "Engine/Engine/EngineOptions.h"
#include "Game/Runner/GameOptions.h"

#include "Engine/Online/OnlineGlobalChat.h"

class UIScene_GlobalChat : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_GlobalChat, "UI");

private:
	std::vector<GlobalChatMessage> m_messages;
	int m_last_chat_messages_recieved;

	int m_messages_recieve_count;

	UITextPanel* m_chat_panel;

protected:

public:
	UIScene_GlobalChat();

	const char* Get_Name();
	bool Should_Render_Lower_Scenes();
	bool Is_Focusable();
	bool Should_Display_Cursor();
	UIScene* Get_Background(UIManager* manager);

	void Refresh(UIManager* manager);
	void Refresh_History();

	void Enter(UIManager* manager);
	void Exit(UIManager* manager);

	void Tick(const FrameTime& time, UIManager* manager, int scene_index);
	void Draw(const FrameTime& time, UIManager* manager, int scene_index);

	void Recieve_Event(UIManager* manager, UIEvent e);

};

#endif

