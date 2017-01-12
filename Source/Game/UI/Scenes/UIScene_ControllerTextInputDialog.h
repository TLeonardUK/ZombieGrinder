// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_CONTROLLERTEXTINPUTDIALOG_
#define _GAME_UI_SCENES_UISCENE_CONTROLLERTEXTINPUTDIALOG_

#include "Engine/UI/UIScene.h"

class UIScene_ControllerTextInputDialog : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_ControllerTextInputDialog, "UI");

private:
	std::string m_text;
	int m_max_length;
	bool m_uppercase;
	JoystickState* m_joystick;
	bool m_using_native_dialog;

protected:
	void SwitchCase();

public:
	UIScene_ControllerTextInputDialog(std::string text, std::string label_text = "", int max_length = 0, JoystickState* joystick = NULL);

	std::string Get_Input();

	const char* Get_Name();
	bool Should_Render_Lower_Scenes();
	bool Is_Focusable();
	bool Should_Display_Cursor();
	bool Should_Display_Focus_Cursor();

	bool Can_Accept_Invite();

	void Refresh_Items();

	void Enter(UIManager* manager);
	void Exit(UIManager* manager);

	void Tick(const FrameTime& time, UIManager* manager, int scene_index);
	void Draw(const FrameTime& time, UIManager* manager, int scene_index);

	void Recieve_Event(UIManager* manager, UIEvent e);

};

#endif

