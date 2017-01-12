// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Scripts/Runtime/CRuntime_Game_Mode.h"
#include "XScript/VirtualMachine/CVMBindingHelper.h"
#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMContext.h"
#include "XScript/VirtualMachine/CVMBinary.h"

#include "Game/Runner/Game.h"
#include "Game/Runner/GameMode.h"

#include "Game/Profile/Profile.h"

#include "Game/Network/GameNetUser.h"

#include "Game/Profile/ItemManager.h"

CVMObjectHandle CRuntime_Game_Mode::Get_Camera(CVirtualMachine* vm, CVMValue self, int index)
{
	GameMode* item = Game::Get()->Get_Game_Mode();
	return item->Get_Camera(index);
}

int CRuntime_Game_Mode::Get_Camera_Count(CVirtualMachine* vm, CVMValue self)
{
	GameMode* item = Game::Get()->Get_Game_Mode();
	return item->Get_Camera_Count();
}

CVMObjectHandle CRuntime_Game_Mode::Get_HUD(CVirtualMachine* vm, CVMValue self)
{
	GameMode* item = Game::Get()->Get_Game_Mode();
	return item->Get_HUD();
}

void CRuntime_Game_Mode::Reset_Game_Over(CVirtualMachine* vm, CVMValue self)
{
	GameMode* item = Game::Get()->Get_Game_Mode();
	item->Reset_Game_Over();
}

void CRuntime_Game_Mode::Add_Game_Over_Bonus(CVirtualMachine* vm, CVMValue self, CVMString name, int value)
{
	GameMode* item = Game::Get()->Get_Game_Mode();
	item->Add_Game_Over_Bonus(name.C_Str(), value);
}

int CRuntime_Game_Mode::Get_Game_Over_Score(CVirtualMachine* vm, CVMValue self)
{
	GameMode* item = Game::Get()->Get_Game_Mode();
	return item->Get_Game_Over_Score();
}

void CRuntime_Game_Mode::Show_Game_Over(CVirtualMachine* vm, CVMValue self, CVMString title, CVMString destination)
{
	GameMode* item = Game::Get()->Get_Game_Mode();
	item->Show_Game_Over(title.C_Str(), destination.C_Str());
}

int CRuntime_Game_Mode::Get_Scoreboard_Rank(CVirtualMachine* vm, CVMValue self, CVMObjectHandle user)
{
	NetUser* u = reinterpret_cast<NetUser*>(user.GetNullCheck(vm)->Get_Meta_Data());
	return dynamic_cast<GameNetUser*>(u)->Get_Scoreboard_Rank();
}

void CRuntime_Game_Mode::Bind(CVirtualMachine* vm)
{
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle,int>("Game_Mode", "Get_Camera", &Get_Camera);
	vm->Get_Bindings()->Bind_Method<int>("Game_Mode", "Get_Camera_Count", &Get_Camera_Count);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>("Game_Mode", "Get_HUD", &Get_HUD);
	vm->Get_Bindings()->Bind_Method<void>("Game_Mode", "Reset_Game_Over", &Reset_Game_Over);
	vm->Get_Bindings()->Bind_Method<void,CVMString,CVMString>("Game_Mode", "Show_Game_Over", &Show_Game_Over);
	vm->Get_Bindings()->Bind_Method<void,CVMString,int>("Game_Mode", "Add_Game_Over_Bonus", &Add_Game_Over_Bonus);
	vm->Get_Bindings()->Bind_Method<int>("Game_Mode", "Get_Game_Over_Score", &Get_Game_Over_Score);
	vm->Get_Bindings()->Bind_Method<int, CVMObjectHandle>("Game_Mode", "Get_Scoreboard_Rank", &Get_Scoreboard_Rank);
}
