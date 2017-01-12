// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_GAME_
#define _GAME_UI_SCENES_UISCENE_GAME_

#include "Engine/UI/UIScene.h"

class UIScene_Chat;
class UIScene_VoiceChat;
class UIScene_VoteOverlay;
class UIScene_TimeoutOverlay;
class Canvas;

class UIScene_Game : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_Game, "UI");

private:
	UIScene_Chat*				m_chat_overlay;
	UIScene_VoiceChat*			m_voice_chat_overlay;
	UIScene_VoteOverlay*		m_vote_overlay;
	UIScene_TimeoutOverlay*		m_timeout_overlay;
	//Canvas*					m_hud_canvas;

	bool						m_closing;

	float						m_hud_alpha;

	bool						m_overlay_enabled;

	u64							m_demo_spectate_platform_id;
	Rect2D						m_demo_spectate_view;
	float						m_demo_freecam_acceleration;
	Rect2D						m_demo_target_view;
	bool						m_demo_initial_camera_snap;

	bool						m_demo_finish_dialog_open;

protected:
	void Draw_Demo_Replay_Screen(const FrameTime& time, Canvas* canvas);
	void Draw_Demo_Replay_Camera(const FrameTime& time, Canvas* canvas);

public:
	UIScene_Game();
	~UIScene_Game();

	const char* Get_Name();
	bool Should_Render_Lower_Scenes();	
	bool Should_Render_Lower_Scenes_Background();
	bool Should_Tick_When_Not_Top();
	UIScene* Get_Background(UIManager* manager);

	UIScene_Chat* Get_Chat_Overlay();

	void Enter(UIManager* manager);
	void Exit(UIManager* manager);
	
	void Recieve_Event(UIManager* manager, UIEvent e);

	void Tick(const FrameTime& time, UIManager* manager, int scene_index);
	void Draw(const FrameTime& time, UIManager* manager, int scene_index);
	void Draw_Overlays(const FrameTime& time, UIManager* manager, int scene_index);

	void Set_Overlay_Enabled(bool enabled);

};

#endif

