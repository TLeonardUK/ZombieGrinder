// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_UI_ELEMENTS_UITOOLBAR_
#define _ENGINE_UI_ELEMENTS_UITOOLBAR_

#include "Engine/UI/UIElement.h"
#include "Engine/UI/UIFrame.h"
#include "Engine/Renderer/Text/Font.h"
#include "Engine/Renderer/Text/FontRenderer.h"

#include "Engine/Renderer/Atlases/AtlasRenderer.h"

class AtlasHandle;
class UIManager;
struct AtlasFrame;

struct UIToolbarAlignment
{
	enum Type
	{
		Near,
		Far,
	};
};

class UIToolbar : public UIElement 
{
	MEMORY_ALLOCATOR(UIToolbar, "UI");

protected:	
	friend class UILayout;

	AtlasHandle*	m_atlas;
	AtlasRenderer	m_atlas_renderer;
	FontHandle*		m_font;
	FontRenderer	m_font_renderer;

	UIManager*		m_manager;
	UIFrame			m_background_frame;

	Rect2D			m_item_padding;

	UIToolbarAlignment::Type	m_alignment;

public:
	UIToolbar();
	~UIToolbar();

	void Set_Alignment(UIToolbarAlignment::Type type);
	
	void Refresh();
	void After_Refresh();

	void Tick(const FrameTime& time, UIManager* manager, UIScene* scene);
	void Draw(const FrameTime& time, UIManager* manager, UIScene* scene);

};

#endif

