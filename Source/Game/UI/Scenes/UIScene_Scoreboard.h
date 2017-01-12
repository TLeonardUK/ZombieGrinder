// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_SCOREBOARD_
#define _GAME_UI_SCENES_UISCENE_SCOREBOARD_

#include "Engine/UI/UIScene.h"
#include "Game/Runner/GameMode.h"

class NetUser;
class UISimpleListView;

struct ScoreboardNetUser
{
	NetUser* user;
	int		 sort_value;
	bool     is_dead;

	static bool Sort_Predicate(const ScoreboardNetUser& a, const ScoreboardNetUser& b)
	{
		return a.sort_value > b.sort_value;
	}
};

struct ScoreboardNetTeam
{
	GameModeTeam* team;
	int			  score;
	int			  team_index;

	static bool Sort_Predicate(const ScoreboardNetTeam& a, const ScoreboardNetTeam& b)
	{
		return a.score > b.score;
	}
};

class UIScene_Scoreboard : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_Scoreboard, "UI");

private:
	float m_update_timer;

	enum
	{
		update_interval = 500
	};

protected:

public:
	UIScene_Scoreboard();
	~UIScene_Scoreboard();

	static void Update_Scoreboard(UISimpleListView* list_view);

	void Update_Map_Details();

	const char* Get_Name();
	bool Should_Render_Lower_Scenes();	
	bool Should_Render_Lower_Scenes_Background();
	bool Should_Tick_When_Not_Top();
	bool Is_Focusable();
	bool Should_Display_Cursor();

	void Enter(UIManager* manager);
	void Exit(UIManager* manager);
	
	void Tick(const FrameTime& time, UIManager* manager, int scene_index);
	void Draw(const FrameTime& time, UIManager* manager, int scene_index);

};

#endif

