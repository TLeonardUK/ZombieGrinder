// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_CREDITS_
#define _GAME_UI_SCENES_UISCENE_CREDITS_

#include "Engine/UI/UIScene.h"

#include "Engine/Renderer/Text/MarkupFontRenderer.h"
#include "Game/UI/Scenes/Backgrounds/MenuTitleRenderer.h"

class FontHandle;

class UIScene_Credits : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_Credits, "UI");

private:
	float					 m_offset;
	std::string				 m_credits;
	std::vector<std::string> m_credits_lines;
	std::vector<Vector2>		 m_credits_lines_sizes;

	Vector2 m_text_scale;

	MenuTitleRenderer		 m_title_renderer;

	FontHandle*			m_font;
	MarkupFontRenderer	m_font_renderer;

	Vector2				m_credits_size;
	float				m_max_offset;

	float				m_fade_out_timer;

	enum
	{
		FADE_OUT_DELAY = 2000
	};

protected:

public:
	UIScene_Credits();
	~UIScene_Credits();

	const char* Get_Name();
	bool Should_Render_Lower_Scenes();
	bool Should_Render_Lower_Scenes_Background();
	
	void Enter(UIManager* manager);
	void Exit(UIManager* manager);
	
	void Tick(const FrameTime& time, UIManager* manager, int scene_index);
	void Draw(const FrameTime& time, UIManager* manager, int scene_index);

};

#endif

