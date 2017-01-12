// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_UI_ELEMENTS_UIDROPDOWNMENU_
#define _ENGINE_UI_ELEMENTS_UIDROPDOWNMENU_

#include "Engine/UI/UIElement.h"
#include "Engine/UI/UIFrame.h"
#include "Engine/Renderer/Text/Font.h"
#include "Engine/Renderer/Text/FontRenderer.h"

#include "Engine/Renderer/Atlases/Atlas.h"
#include "Engine/Renderer/Atlases/AtlasRenderer.h"

#include "Engine/Renderer/PrimitiveRenderer.h"

class AtlasHandle;
class UIManager;
struct AtlasFrame;

class UIDropDownMenu : public UIElement 
{
	MEMORY_ALLOCATOR(UIDropDownMenu, "UI");

protected:	
	friend class UILayout;
		
	PrimitiveRenderer			m_primitive_renderer;
	AtlasRenderer				m_atlas_renderer;
	
	Color						m_frame_color;
		
	std::string					m_frame_name;
	UIFrame						m_frame;

	bool						m_visible;
	bool						m_visible_last_frame;

	UIElement*					m_previous_focused_element;

public:
	UIDropDownMenu();
	~UIDropDownMenu();
	
	void Refresh();
	void After_Refresh();

	void Tick(const FrameTime& time, UIManager* manager, UIScene* scene);
	void Draw(const FrameTime& time, UIManager* manager, UIScene* scene);

	void Show(Rect2D position);
	void Hide();

};

#endif

