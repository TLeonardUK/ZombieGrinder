// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_FILE_BROWSER_
#define _GAME_UI_SCENES_UISCENE_FILE_BROWSER_

#include "Engine/UI/UIScene.h"

#include "Generic/Types/Color.h"

class UIScene_FileBrowser : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_FileBrowser, "UI");

private:
	bool m_save;
	std::string m_path;
	std::string m_overwrite_path;
	std::string m_selected_path;
	std::string m_extension;
	bool m_closing;

protected:
	void Selected_Path(UIManager* manager);
	void Return_Path(UIManager* manager, std::string path);
	
public:
	UIScene_FileBrowser(bool save, std::string path, std::string extension);

	std::string Get_Selected_Path();

	const char* Get_Name();
	bool Should_Render_Lower_Scenes();
	bool Is_Focusable();
	bool Should_Display_Cursor();
		
	bool Should_Display_Focus_Cursor();
	bool Should_Fade_Cursor();

	bool Can_Accept_Invite();
	
	void Refresh_Items();

	void Enter(UIManager* manager);
	void Exit(UIManager* manager);
	
	void Tick(const FrameTime& time, UIManager* manager, int scene_index);
	void Draw(const FrameTime& time, UIManager* manager, int scene_index);
		
	void Recieve_Event(UIManager* manager, UIEvent e);

};

#endif

