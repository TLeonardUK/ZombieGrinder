// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_LEADERBOARDS_
#define _GAME_UI_SCENES_UISCENE_LEADERBOARDS_

#include "Engine/UI/UIScene.h"

#include "Engine/UI/Elements/UIListView.h"

#include "Generic/Events/Event.h"
#include "Generic/Events/Delegate.h"

struct Leaderboard;

class UIScene_Leaderboards : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_Leaderboards, "UI");

private:
	Delegate<UIScene_Leaderboards, UIListViewGetItemData>* m_get_item_delegate;
	
	Leaderboard* m_current_board;
	//bool m_friends_only;
	bool m_initial_seek;

	void Refresh_Board();

	enum
	{
		refresh_range = 16
	};

protected:

public:
	UIScene_Leaderboards();

	const char* Get_Name();
	bool Should_Render_Lower_Scenes();
	bool Is_Focusable();
	bool Should_Display_Cursor();
	
	void Enter(UIManager* manager);
	void Exit(UIManager* manager);
	
	void Tick(const FrameTime& time, UIManager* manager, int scene_index);
	void Draw(const FrameTime& time, UIManager* manager, int scene_index);
		
	void On_Get_Item_Data(UIListViewGetItemData* data);

	void Recieve_Event(UIManager* manager, UIEvent e);

};

#endif

