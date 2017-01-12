// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_INPUT_OPTIONS_
#define _GAME_UI_SCENES_UISCENE_INPUT_OPTIONS_

#include "Engine/UI/UIScene.h"

#include "Engine/Engine/EngineOptions.h"
#include "Game/Runner/GameOptions.h"

class UIScene_InputOptions : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_InputOptions, "UI");

private:

protected:
	InputBindings::Type Get_Keyboard_For_Bind(OutputBindings::Type type);
	InputBindings::Type Get_Joystick_For_Bind(OutputBindings::Type type);

public:
	UIScene_InputOptions();

	const char* Get_Name();
	bool Should_Render_Lower_Scenes();
	bool Is_Focusable();
	bool Should_Display_Cursor();
	UIScene* Get_Background(UIManager* manager);
	
	void Refresh_Items();

	void Enter(UIManager* manager);
	void Exit(UIManager* manager);
	
	void Tick(const FrameTime& time, UIManager* manager, int scene_index);
	void Draw(const FrameTime& time, UIManager* manager, int scene_index);
		
	void Recieve_Event(UIManager* manager, UIEvent e);

};

#endif

