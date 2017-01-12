// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_UI_ELEMENTS_UIPANEL_
#define _ENGINE_UI_ELEMENTS_UIPANEL_

#include "Engine/UI/UIElement.h"
#include "Engine/UI/UIFrame.h"
#include "Engine/Renderer/Text/Font.h"
#include "Engine/Renderer/Text/FontRenderer.h"

#include "Engine/Renderer/Atlases/Atlas.h"
#include "Engine/Renderer/Atlases/AtlasRenderer.h"

#include "Engine/Renderer/PrimitiveRenderer.h"

#include "Generic/Events/Event.h"

class AtlasHandle;
class UIManager;
struct AtlasFrame;
class UIPanel;

struct UIPanelBGScaleMode
{
	enum Type
	{
		Zoom,
		Scale,
		Center,
		FullWidth,
		FullHeight
	};
};

struct UIPanelDrawItem
{
public:
	UIPanel*		view;
	Rect2D			view_bounds;
	FrameTime		time;
	UIManager*		manager;
	UIScene*		scene;
	Vector2			ui_scale;
};

class UIPanel : public UIElement 
{
	MEMORY_ALLOCATOR(UIPanel, "UI");

protected:	
	friend class UILayout;

	Color						m_backcolor;
	Color						m_forecolor;
	AtlasFrame*					m_backimage;
	AtlasFrame*					m_foreimage;
	Texture*					m_backtexture;
	TextureHandle*				m_backtexturehandle;
	UIPanelBGScaleMode::Type	m_bg_scale_mode;
	std::string					m_backimage_name;
	std::string					m_foreimage_name;
		
	PrimitiveRenderer			m_primitive_renderer;
	AtlasRenderer				m_atlas_renderer;
	
	Color						m_frame_color;
		
	std::string					m_frame_name;
	UIFrame						m_frame;

	bool						m_flat_bottom;

public:
	Event<UIPanelDrawItem>		On_Draw;

public:
	UIPanel();
	~UIPanel();
	
	void Set_Foreground_Image(AtlasFrame* frame);
	void Set_Background_Image(AtlasFrame* frame);
	void Set_Background_Image(TextureHandle* texture);
	void Set_Background_Image(Texture* texture);

	void Set_Background_Color(Color color);
	void Set_Foreground_Color(Color color);

	void Set_Frame_Color(Color color);

	void Refresh();
	void After_Refresh();

	void Tick(const FrameTime& time, UIManager* manager, UIScene* scene);
	void Draw(const FrameTime& time, UIManager* manager, UIScene* scene);

};

#endif

