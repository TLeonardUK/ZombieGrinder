// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_UI_ELEMENTS_UICHECKBOX_
#define _ENGINE_UI_ELEMENTS_UICHECKBOX_

#include "Engine/UI/UIElement.h"
#include "Engine/UI/UIFrame.h"
#include "Engine/Renderer/Text/Font.h"
#include "Engine/Renderer/Text/FontRenderer.h"

#include "Engine/Renderer/Atlases/Atlas.h"
#include "Engine/Renderer/Atlases/AtlasRenderer.h"

#include "Engine/Renderer/PrimitiveRenderer.h"

#include "Engine/Input/OutputBindings.h"

class AtlasHandle;
class UIManager;
struct AtlasFrame;

class UICheckBox : public UIElement 
{
	MEMORY_ALLOCATOR(UICheckBox, "UI");

protected:	
	friend class UILayout;

	AtlasHandle*		m_atlas;
	AtlasRenderer		m_atlas_renderer;

	FontHandle*			m_font;
	FontRenderer		m_font_renderer;

	AtlasFrame*			m_checked_frame;
	AtlasFrame*			m_unchecked_frame;

	Color				m_frame_color;

	bool				m_checked;

public:
	UICheckBox();
	~UICheckBox();

	bool Get_Checked();
	void Set_Checked(bool checked);

	void Refresh();
	void After_Refresh();
	bool Is_Focusable();

	void Tick(const FrameTime& time, UIManager* manager, UIScene* scene);
	void Draw(const FrameTime& time, UIManager* manager, UIScene* scene);

};

#endif

