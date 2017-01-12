// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_LOADING_
#define _GAME_UI_SCENES_UISCENE_LOADING_

#include "Engine/UI/UIScene.h"
#include "Engine/Tasks/Task.h"
#include "Engine/Tasks/TaskManager.h"

class OnlineSubscribedMod;

class FinalizeLoadingTask : public Task
{
	MEMORY_ALLOCATOR(FinalizeLoadingTask, "Engine");

public:
	void Run();
	void LoadMod(OnlineSubscribedMod* mod);

};

class UIScene_Loading : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_Loading, "UI");

private:
	int m_load_sequence;

	FinalizeLoadingTask* m_finalize_task;
	TaskID m_finalize_task_id;

protected:

public:
	UIScene_Loading();

	const char* Get_Name();
	bool Should_Render_Lower_Scenes();
	UIScene* Get_Background(UIManager* manager);
	
	bool Can_Accept_Invite();

	void Enter(UIManager* manager);
	void Exit(UIManager* manager);
	
	void Tick(const FrameTime& time, UIManager* manager, int scene_index);
	void Draw(const FrameTime& time, UIManager* manager, int scene_index);

};

#endif

