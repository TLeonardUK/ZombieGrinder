// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_KEY_BIND_DIALOG_
#define _GAME_UI_SCENES_UISCENE_KEY_BIND_DIALOG_

#include "Engine/UI/UIScene.h"

#include "Engine/Engine/EngineOptions.h"
#include "Game/Runner/GameOptions.h"

class UIScene_KeyBindDialog : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_KeyBindDialog, "UI");

private:
	OutputBindings::Type m_output_binding;

protected:
	
public:
	UIScene_KeyBindDialog(OutputBindings::Type type);

	const char* Get_Name();
	bool Should_Render_Lower_Scenes();
	bool Is_Focusable();
	bool Should_Display_Cursor();
	
	void Refresh_Items();

	void Enter(UIManager* manager);
	void Exit(UIManager* manager);
	
	void Tick(const FrameTime& time, UIManager* manager, int scene_index);
	void Draw(const FrameTime& time, UIManager* manager, int scene_index);
		
	void Recieve_Event(UIManager* manager, UIEvent e);

	void Perform_Binding(InputBindings::Type type);

};

#endif

