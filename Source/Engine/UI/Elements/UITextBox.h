// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_UI_ELEMENTS_UITEXTBOX_
#define _ENGINE_UI_ELEMENTS_UITEXTBOX_

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

class UITextBox : public UIElement 
{
	MEMORY_ALLOCATOR(UITextBox, "UI");

protected:	
	friend class UILayout;

	AtlasHandle*		m_atlas;
	AtlasRenderer		m_atlas_renderer;

	FontHandle*			m_font;
	FontRenderer		m_font_renderer;

	UIFrame				m_game_background_frame;
	UIFrame				m_game_background_active_frame;
	
	UIFrame				m_background_frame;
	UIFrame				m_background_active_frame;

	Color				m_frame_color;

	Vector2				m_text_size;
	float				m_text_offset;

	Rect2D				m_padded_box;
	Rect2D				m_viewport_box;

	bool				m_draw_carrot;
	float				m_carrot_timer;

	int					m_carrot_offset;

	bool				m_always_active;

	int					m_max_length;

	bool				m_controller_input_pending;

	bool				m_game_style;
	bool				m_draw_border;

	bool				m_was_entered;

	enum
	{
		CARROT_BLINK_INTERVAL = 300
	};

private:
	void Text_Changed();

public:
	UITextBox();
	~UITextBox();

	void Set_Value(std::string value);
	void Set_Max_Length(int len);
	void Set_Draw_Border(bool value);

	bool Was_Entered();

	void Focus();
	
	void Set_Game_Style(bool value) { m_game_style = value; }

	bool Focus_Left(UIManager* manager);
	bool Focus_Right(UIManager* manager);

	void Refresh();
	void After_Refresh();
	bool Is_Focusable();

	void Tick(const FrameTime& time, UIManager* manager, UIScene* scene);
	void Draw(const FrameTime& time, UIManager* manager, UIScene* scene);

};

#endif

