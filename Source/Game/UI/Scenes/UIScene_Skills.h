// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_SKILLS_
#define _GAME_UI_SCENES_UISCENE_SKILLS_

#include "Engine/UI/UIScene.h"

#include "Generic/Events/Event.h"
#include "Generic/Events/Delegate.h"

struct Profile;
class UIGridTree;
struct UIGridTreeNode;
struct SkillArchetype;
struct UIGridTreeDrawItemData;
struct UIGridTreeCanDragData;
struct AtlasFrame;
struct SkillArchetype;

class UIScene_Skills : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_Skills, "UI");

private:
	Delegate<UIScene_Skills, UIGridTreeDrawItemData>* m_draw_item_delegate;
	Delegate<UIScene_Skills, UIGridTreeCanDragData>* m_can_drag_delegate;
	
	Profile* m_profile;
	
	float m_display_points;
	float m_point_sound_timer;

	AtlasFrame* m_lock_frame;

	float m_info_alpha;

	UIGridTreeNode* m_last_selected_node;

	SkillArchetype* m_purchase_item;

	bool m_rerolling_skills;
	
	int m_last_equip_slot_hover;
	float m_last_equip_slot_hover_timer;

	enum
	{
		POINT_SOUND_INTERVAL = 50,
		INFO_FADE_IN_DELAY = 500
	};

protected:	
	void Add_Tree_Item(UIGridTree* grid_view, SkillArchetype* skill, UIGridTreeNode* parent, std::vector<SkillArchetype*>& skills);
	void Update_Equip_Items();

public:
	UIScene_Skills(Profile* profile);
	~UIScene_Skills();

	const char* Get_Name();
	bool Should_Render_Lower_Scenes();
	bool Is_Focusable();
	bool Should_Display_Cursor();	

	void Enter(UIManager* manager);
	void Exit(UIManager* manager);
	
	void Tick(const FrameTime& time, UIManager* manager, int scene_index);
	void Draw(const FrameTime& time, UIManager* manager, int scene_index);
		
	void Recieve_Event(UIManager* manager, UIEvent e);
	
	void On_Draw_Item(UIGridTreeDrawItemData* data);
	void Can_Drag_Drop(UIGridTreeCanDragData* data);

};

#endif

