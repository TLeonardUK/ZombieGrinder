// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_RESOURCE_BROWSER_
#define _GAME_UI_SCENES_UISCENE_RESOURCE_BROWSER_

#include "Engine/UI/UIScene.h"

#include "Generic/Types/Color.h"

struct UIScene_ResourceBrowserMode
{
	enum Type
	{
		Audio,
		Image,
		Animation
	};
};

class UIScene_ResourceBrowser : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_ResourceBrowser, "UI");

private:
	UIScene_ResourceBrowserMode::Type m_mode;
	std::string m_filter;

	bool m_closing;
	std::string m_selected_resource;

protected:
	void Return_Resource(UIManager* manager, std::string path);

public:
	UIScene_ResourceBrowser(UIScene_ResourceBrowserMode::Type type, std::string filter = "");

	const char* Get_Name();
	bool Should_Render_Lower_Scenes();
	bool Is_Focusable();
	bool Should_Display_Cursor();
		
	bool Should_Display_Focus_Cursor();
	bool Should_Fade_Cursor();

	bool Can_Accept_Invite();

	std::string Get_Selected_Resource();

	void Enter(UIManager* manager);
	void Exit(UIManager* manager);
	
	void Tick(const FrameTime& time, UIManager* manager, int scene_index);
	void Draw(const FrameTime& time, UIManager* manager, int scene_index);
		
	void Recieve_Event(UIManager* manager, UIEvent e);

};

#endif

