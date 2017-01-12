// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_VIDEO_
#define _GAME_UI_SCENES_UISCENE_VIDEO_

#include "Engine/UI/UIScene.h"
#include "Engine/UI/UIManager.h"

class VideoHandle;
class VideoRenderer;

class UIScene_Video : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_Video, "UI");

private:
	VideoHandle* m_video;
	VideoRenderer* m_video_renderer;

	float m_original_bgm_multiplier;

	UIAction m_next_screen;

	float m_skip_timer;
	float m_last_skip_timer;
	float m_skip_alpha;

	enum
	{
		SKIP_DURATION = 2
	};

protected:

public:
	UIScene_Video(VideoHandle* handle, UIAction next_screen);
	~UIScene_Video();

	const char* Get_Name();
	bool Should_Render_Lower_Scenes();
	bool Should_Render_Lower_Scenes_Background();
	UIScene* Get_Background(UIManager* manager);

	bool Can_Accept_Invite();

	void Enter(UIManager* manager);
	void Exit(UIManager* manager);

	void Tick(const FrameTime& time, UIManager* manager, int scene_index);
	void Draw(const FrameTime& time, UIManager* manager, int scene_index);

};

#endif

