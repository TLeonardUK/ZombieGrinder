// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_PUBLISH_MAP_
#define _GAME_UI_SCENES_UISCENE_PUBLISH_MAP_

#include "Engine/UI/UIScene.h"

#include "Generic/Types/Color.h"

class UIScene_PublishMap : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_PublishMap, "UI");

private:
	bool m_uploading;
	bool m_requesting_mod_info;
	std::string m_original_map_path;
	bool m_creating_mod;
	bool m_creation_status;

	std::string Validate_Map();

	void Upload_Mod();
	void Begin_Upload();
	void Publish_Mod();

public:
	UIScene_PublishMap();

	const char* Get_Name();
	bool Should_Render_Lower_Scenes();
	bool Is_Focusable();
	bool Should_Display_Cursor();

	bool Should_Display_Focus_Cursor();
	bool Should_Fade_Cursor();

	bool Can_Accept_Invite();

	void Enter(UIManager* manager);
	void Exit(UIManager* manager);

	void Tick(const FrameTime& time, UIManager* manager, int scene_index);
	void Draw(const FrameTime& time, UIManager* manager, int scene_index);

	void Recieve_Event(UIManager* manager, UIEvent e);

};

#endif

