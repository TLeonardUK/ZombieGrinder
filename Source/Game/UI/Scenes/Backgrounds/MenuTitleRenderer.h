// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_TITLE_RENDERER_
#define _GAME_UI_SCENES_UISCENE_TITLE_RENDERER_

#include "Engine/UI/UIScene.h"

#include "Engine/Renderer/Atlases/Atlas.h"
#include "Engine/Renderer/Atlases/AtlasRenderer.h"

#include "Engine/Renderer/Text/FontHandle.h"
#include "Engine/Renderer/Text/FontRenderer.h"

#include <vector>

class MenuBackgroundTitle
{
	MEMORY_ALLOCATOR(MenuBackgroundTitle, "UI");

private:
	float	m_x, m_y;

	bool	m_active;
	float	m_timer;
	float	m_interval;

public:
	void Tick(const FrameTime& time);
	void Draw(const FrameTime& time, float logo_x, float logo_y, AtlasRenderer& renderer, AtlasFrame* logo_frame);

	void Deactivate();
	void Activate(float delta = 1.0f);

	bool Is_Active();
	
	float Get_Delta();
	float Get_X_Offset();
	float Get_Y_Offset();

};

class MenuTitleRenderer
{
	MEMORY_ALLOCATOR(MenuTitleRenderer, "UI");

private:
	static const int			TITLE_COUNT = 16;
	MenuBackgroundTitle			m_titles[TITLE_COUNT];

	std::vector<const char*>	m_taglines;
	const char*					m_tagline;
	int							m_tagline_count;

	int							m_tagline_index;

	float						m_menu_timer;
	float						m_tagline_flicker_change_timer;
	float						m_tagline_change_timer;

	AtlasRenderer				m_atlas_renderer;
	AtlasFrame*					m_logo_frame;
	
	FontHandle*					m_font;
	FontRenderer				m_font_renderer;
	
	float						m_scale_timer;
	bool						m_scale_direction;
	float						m_scale;
	float						m_scalar;
	float						m_resolution_scale;

	Color						m_tag_color;

	bool						m_ticked;

protected:

public:
	MenuTitleRenderer();

	void Reset();

	void Tick(const FrameTime& time);
	void Draw(const FrameTime& time, Vector2 logo_position, Vector2 tagline_position, bool render_tag = true, float rotation = 0, float scale = 1.0f, float alpha = 1.0f);

};

#endif

