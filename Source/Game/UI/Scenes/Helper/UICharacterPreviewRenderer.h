// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_HELPER_UICHARACTERPREVIEWRENDERER_
#define _GAME_UI_SCENES_HELPER_UICHARACTERPREVIEWRENDERER_

#include "Engine/UI/UIScene.h"

#include "Engine/Renderer/Atlases/Atlas.h"
#include "Engine/Renderer/Atlases/AtlasRenderer.h"

#include "Engine/Renderer/Text/FontHandle.h"
#include "Engine/Renderer/Text/FontRenderer.h"

#include <vector>

struct Profile;
struct Item;

class UICharacterPreviewRenderer
{
	MEMORY_ALLOCATOR(UICharacterPreviewRenderer, "UI");

private:
	enum
	{
		DIRECTION_CHANGE_INTERVAL = 300,
	};

	float m_direction_change_timer;
	int m_direction;

	int m_last_update_frame;

	AtlasAnimation* m_leg_animations[8];
	AtlasAnimation* m_idle_animations[8];

	void Draw_Item(const FrameTime& time, Item* item, Rect2D item_box);
	void Draw_Body(const FrameTime& time, Item* item, Rect2D item_box);

public:
	UICharacterPreviewRenderer();

	int Get_Direction();
	void Draw(const FrameTime& time, UIManager* manager, Profile* profile, Rect2D box);
	void Draw_Head(const FrameTime& time, UIManager* manager, Profile* profile, Rect2D box);

};

#endif

