// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_UI_ELEMENTS_UISPLITCONTAINER_
#define _ENGINE_UI_ELEMENTS_UISPLITCONTAINER_

#include "Engine/UI/UIElement.h"
#include "Engine/UI/UIFrame.h"
#include "Engine/Renderer/Text/Font.h"
#include "Engine/Renderer/Text/FontRenderer.h"

#include "Engine/Renderer/Atlases/AtlasRenderer.h"

class AtlasHandle;
class UIManager;
struct AtlasFrame;

struct SplitContainerDirection
{
	enum Type
	{
		Horizontal,
		Vertical
	};
};

class UISplitContainer : public UIElement 
{
	MEMORY_ALLOCATOR(UISplitContainer, "UI");

protected:	
	friend class UILayout;

	bool							m_resizable;
	SplitContainerDirection::Type	m_direction;
	
	AtlasHandle*					m_atlas;
	UIManager*						m_manager;
	AtlasRenderer					m_atlas_renderer;

	float							m_split_seperator_width;
	float							m_split_offset;
	float							m_split_offset_min;
	float							m_split_offset_max;

	Rect2D							m_split_bar_rect;

	bool							m_moving_split;
	
	UIFrame							m_background_frame;
	
public:
	UISplitContainer();
	~UISplitContainer();
	
	void Refresh();
	void After_Refresh();

	void Tick(const FrameTime& time, UIManager* manager, UIScene* scene);
	void Draw(const FrameTime& time, UIManager* manager, UIScene* scene);

};

#endif

