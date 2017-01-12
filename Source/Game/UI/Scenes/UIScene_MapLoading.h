// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_MAP_LOADING_
#define _GAME_UI_SCENES_UISCENE_MAP_LOADING_

#include "Engine/UI/UIScene.h"

#include "Engine/Tasks/TaskManager.h"
#include "Engine/Tasks/Task.h"

#include "Game/Scene/Map/MapLoadTask.h"

class MapFileHandle;

struct MapLoadingState
{
	enum Type
	{
		WaitingForServer,
		LoadingMap,
		SendingClientInfo,
		SyncWithServer,
		WaitingForStart,
		Starting
	};
};

class UIScene_MapLoading : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_MapLoading, "UI");

private:
	bool m_set_style;
	
	std::vector<const char*> m_tips;
	int m_tip_count;
	const char* m_tip;
	
	std::string m_progress_text;

	float m_tip_timer;

	MapLoadingState::Type m_loading_state;

	MapFileHandle* m_map_handle;

	MapLoadTask* m_load_task;
	TaskID m_load_task_id;

	float m_duration_timer;

	bool m_bgm_pushed;	
	bool m_load_success;

	enum
	{
		tip_interval = 5 * 1000,
		tip_fade_interval = 500,
		pip_increment_interval = 200,
		pip_count = 3,
		minimum_display_time = 2000
	};

protected:
	void Next_Tip();

	void Set_Progress(const char* text, float progress);
	void Set_Map_Details(const char* name, const char* description);

	bool Update_Loading(const FrameTime& time, UIManager* uimanager);

public:
	UIScene_MapLoading();

	const char* Get_Name();
	bool Should_Render_Lower_Scenes();
	bool Should_Render_Lower_Scenes_Background();
	UIScene* Get_Background(UIManager* manager);
	
	bool Can_Accept_Invite();

	void Refresh(UIManager* manager);

	void Enter(UIManager* manager);
	void Exit(UIManager* manager);
	
	void Tick(const FrameTime& time, UIManager* manager, int scene_index);
	void Draw(const FrameTime& time, UIManager* manager, int scene_index);

};

#endif

