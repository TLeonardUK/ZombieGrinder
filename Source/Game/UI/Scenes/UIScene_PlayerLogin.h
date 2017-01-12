// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_PLAYER_LOGIN_
#define _GAME_UI_SCENES_UISCENE_PLAYER_LOGIN_

#include "Engine/UI/UIScene.h"

#include "Engine/Input/InputSource.h"

#include "Engine/Online/OnlinePlatform.h"

class UICharacterPreviewRenderer;

class UIScene_PlayerLogin : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_PlayerLogin, "UI");

private:
	int m_login_indexes[MAX_LOCAL_PLAYERS];
	bool m_accept_invite;

	float m_blink_timer;
	bool m_blink;

	InputSource m_select_profile_source;
	int m_select_profile_index;
	int m_selected_profile_index;
	
	UICharacterPreviewRenderer* m_char_renderer;

	enum
	{
		BLINK_INTERVAL = 400
	};

protected:
	void Login_Player_Index(int index, InputSource source, int profile_index);
	void Logout_Player_Index(int index);
	bool Player_Logged_In(int index);
	bool Any_Player_Logged_In();
	int Input_Source_To_Player_Index(InputSource source);
	int Get_Free_Player_Index();
	void Refresh_Screen();

	void Swap_Login(int from, int to);
	void Align_Logins_Array();

public:
	UIScene_PlayerLogin(bool accept_invite = false);
	~UIScene_PlayerLogin();

	bool Can_Accept_Invite();

	const char* Get_Name();
	bool Should_Render_Lower_Scenes();
	bool Should_Display_Cursor();
	bool Is_Focusable();
	UIScene* Get_Background(UIManager* manager);
	
	void Enter(UIManager* manager);
	void Exit(UIManager* manager);
	
	void Tick(const FrameTime& time, UIManager* manager, int scene_index);
	void Draw(const FrameTime& time, UIManager* manager, int scene_index);
	
	void Recieve_Event(UIManager* manager, UIEvent e);

};

#endif

