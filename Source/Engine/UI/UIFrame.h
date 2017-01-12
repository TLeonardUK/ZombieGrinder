// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_UI_UIFRAME_
#define _ENGINE_UI_UIFRAME_

#include "Engine/Engine/FrameTime.h"
#include "Generic/Types/Rect2D.h"

#include "Engine/Renderer/Atlases/AtlasRenderer.h"

#include <vector>

class UIManager;
class AtlasHandle;

class UIFrame
{
	MEMORY_ALLOCATOR(UIFrame, "UI");

protected:
	static HashTable<UIFrame, int> m_hashed_frames;

	AtlasFrame* m_frames[9];

public:
	UIFrame();
	UIFrame(std::string name_pattern);

	void Draw_Frame(AtlasRenderer& renderer, Rect2D rect, Vector2 scale = Vector2(1.0f, 1.0f), Color color = Color::White);
	void Draw_Frame_Flat_Top(AtlasRenderer& renderer, Rect2D rect, Vector2 scale = Vector2(1.0f, 1.0f), Color color = Color::White);
	void Draw_Frame_Flat_Bottom(AtlasRenderer& renderer, Rect2D rect, Vector2 scale = Vector2(1.0f, 1.0f), Color color = Color::White);

};

#endif

