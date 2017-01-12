// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_GAMEOVER_
#define _GAME_UI_SCENES_UISCENE_GAMEOVER_

#include "Engine/UI/UIScene.h"
#include "Engine/UI/Elements/UIListView.h"

#include "Engine/Renderer/Text/MarkupFontRenderer.h"

#include "Generic/Events/Event.h"
#include "Generic/Events/Delegate.h"

struct Leaderboard;

struct GameOverBonus
{
	std::string name;
	std::string value_format;
	int			value;
	float		scale;

	GameOverBonus()
		: name("")
		, value_format("%s")
		, value(0)
		, scale(1.0f)
	{
	}
};

class UIScene_GameOver : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_GameOver, "UI");

private:
	std::vector<GameOverBonus> m_bonuses;
	float m_bgm_volume;

	std::string m_destination;

	MarkupFontRenderer m_font_renderer;

	int m_bonus_display_index;
	float m_bonus_display_time;

	bool m_initial_seek;

	Delegate<UIScene_GameOver, UIListViewGetItemData>* m_get_item_delegate;
	Leaderboard* m_leaderboard;

	float m_restart_timer;

	bool m_has_restarted_map;

	bool m_is_pvp;

	bool m_show_black_bg;

	enum 
	{
		bonus_display_time = 250,
		refresh_range = 16,
		restart_countdown = 15000
	};

protected:
	void Refresh_Board();
	//void Update_Scoreboard();

public:
	UIScene_GameOver(std::vector<GameOverBonus>& bonuses, std::string title, std::string destination, bool show_black_bg = false);
	~UIScene_GameOver();

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

	void On_Get_Item_Data(UIListViewGetItemData* data);

};

#endif

