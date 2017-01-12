// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_RUNTIME_GAME_MODE_
#define _GAME_RUNTIME_GAME_MODE_

#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMValue.h"

class CRuntime_Game_Mode
{
public:
	static CVMObjectHandle Get_Camera(CVirtualMachine* vm, CVMValue self, int index);
	static int Get_Camera_Count(CVirtualMachine* vm, CVMValue self);
	static CVMObjectHandle Get_HUD(CVirtualMachine* vm, CVMValue self);

	static int Get_Scoreboard_Rank(CVirtualMachine* vm, CVMValue self, CVMObjectHandle user);

	static void Reset_Game_Over(CVirtualMachine* vm, CVMValue self);
	static void Add_Game_Over_Bonus(CVirtualMachine* vm, CVMValue self, CVMString name, int value);
	static void Show_Game_Over(CVirtualMachine* vm, CVMValue self, CVMString title, CVMString destination);
	static int Get_Game_Over_Score(CVirtualMachine* vm, CVMValue self);	

	static void Bind(CVirtualMachine* machine);
};

#endif
