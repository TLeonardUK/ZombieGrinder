// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_GAMESETUPMENU_
#define _GAME_UI_SCENES_UISCENE_GAMESETUPMENU_

#include "Engine/UI/UIScene.h"
#include "Engine/UI/UIEvent.h"
#include "Engine/UI/Elements/UIPanel.h"
#include "Engine/UI/Elements/UIListView.h"

#include "Game/Online/ChallengeManager.h"
#include "Game/Online/RankingsManager.h"

struct Leaderboard;
struct Item;
struct Profile;
struct ItemArchetype;
struct SkillArchetype;

class UIScene_GameSetupMenu : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_GameSetupMenu, "UI");

private:
	ChallengeTimeframe::Type m_challenge_timeframe;
	float m_cycle_challenge_time;

	int m_direction;
	float m_direction_change_timer;

	float m_item_select_time;
	float m_info_alpha;

	Leaderboard* m_challenge_leaderboard;
	bool m_initial_seek;

	Delegate<UIScene_GameSetupMenu, UIPanelDrawItem>* m_draw_item_delegate;
	Delegate<UIScene_GameSetupMenu, UIListViewGetItemData>* m_get_item_delegate;

	bool m_initial_rank_refresh;
	std::vector<LeaderboardEntry> m_total_ranks;
	int m_our_user_rank;

	enum
	{
		DIRECTION_CHANGE_INTERVAL = 300,
		INFO_FADE_IN_DELAY = 500,
		TOTAL_RANKS_TO_SHOW = 100
	};

	int m_cycle_seed;

protected:
	void Update_Moderate();

	void Show_Challenge(ChallengeTimeframe::Type timeframe);
	void Update_Challenge_State();

	void Update_Leaderboard();

	void Update_Info_Box(const FrameTime& time, UIManager* manager);

	void On_Get_Item_Data(UIListViewGetItemData* data);
	void On_Draw_Item(UIPanelDrawItem* data);

	bool Get_Next_Broken_Item(Item*& item, Profile*& profile, int& index);

	bool Get_Unlocked_States(Profile*& outProfile, std::vector<ItemArchetype*>& outItemUnlocks, std::vector<SkillArchetype*>& outSkillUnlocks);

public:
	UIScene_GameSetupMenu();
	~UIScene_GameSetupMenu();

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

