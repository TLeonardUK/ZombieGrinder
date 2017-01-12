// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_LOBBY_
#define _GAME_UI_SCENES_UISCENE_LOBBY_

#include "Engine/UI/UIScene.h"

#include "Engine/Online/OnlineMatching.h"
#include "Engine/Online/OnlineGameServerFinder.h"

#include "Generic/Events/Delegate.h"
#include "Generic/Events/Event.h"

class UIScene_Lobby : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_Lobby, "UI");

private:	
	Delegate<UIScene_Lobby, OnlineMatching_LobbyMemberJoinedEventData>*		m_lobby_join_event_delegate;
	Delegate<UIScene_Lobby, OnlineMatching_LobbyMemberLeftEventData>*		m_lobby_left_event_delegate;
	Delegate<UIScene_Lobby, OnlineMatching_LobbyChatEventData>*				m_lobby_chat_event_delegate;
	Delegate<UIScene_Lobby, OnlineMatching_LobbyMemberDataUpdateEventData>*	m_lobby_member_data_update_event_delegate;

	OnlineGameServerFinder													m_game_server_finder;

	OnlineGameServerFinder_State::Type										m_game_server_finder_old_state;

	bool m_connecting;

	bool m_auto_continue;
	bool m_start_editor;

	bool m_first_enter;

protected:

	void Refresh_Preview(UIManager* manager);
	void Refresh_Member_List(UIManager* manager);
	
	void Callback_Lobby_Member_Joined(OnlineMatching_LobbyMemberJoinedEventData* data);
	void Callback_Lobby_Member_Left(OnlineMatching_LobbyMemberLeftEventData* data);
	void Callback_Lobby_Chat(OnlineMatching_LobbyChatEventData* data);
	void Callback_Lobby_Member_Data_Update(OnlineMatching_LobbyMemberDataUpdateEventData* data);

public:
	UIScene_Lobby(bool auto_continue = false, bool start_editor = false);
	~UIScene_Lobby();

	const char* Get_Name();
	bool Should_Render_Lower_Scenes();
	bool Is_Focusable();
	bool Should_Display_Cursor();
	UIScene* Get_Background(UIManager* manager);

	void Refresh(UIManager* manager);
	
	void Enter(UIManager* manager);
	void Exit(UIManager* manager);
	
	void Tick(const FrameTime& time, UIManager* manager, int scene_index);
	void Draw(const FrameTime& time, UIManager* manager, int scene_index);
		
	void Recieve_Event(UIManager* manager, UIEvent e);

};

#endif

