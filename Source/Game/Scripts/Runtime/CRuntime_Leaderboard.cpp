// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Scripts/Runtime/CRuntime_Leaderboard.h"
#include "XScript/VirtualMachine/CVMBindingHelper.h"
#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMContext.h"
#include "XScript/VirtualMachine/CVMBinary.h"

#include "Engine/Resources/ResourceFactory.h"

#include "Game/Online/RankingsManager.h"

void CRuntime_Leaderboard::Set_ID(CVirtualMachine* vm, CVMValue self, CVMString string)
{
	Leaderboard* leaderboard = reinterpret_cast<Leaderboard*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	leaderboard->id = string.C_Str();
}

CVMString CRuntime_Leaderboard::Get_ID(CVirtualMachine* vm, CVMValue self)
{
	Leaderboard* leaderboard = reinterpret_cast<Leaderboard*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return leaderboard->id.c_str();
}

void CRuntime_Leaderboard::Set_Name(CVirtualMachine* vm, CVMValue self, CVMString string)
{
	Leaderboard* leaderboard = reinterpret_cast<Leaderboard*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	leaderboard->name = string.C_Str();
}

CVMString CRuntime_Leaderboard::Get_Name(CVirtualMachine* vm, CVMValue self)
{
	Leaderboard* leaderboard = reinterpret_cast<Leaderboard*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return leaderboard->name.c_str();
}

void CRuntime_Leaderboard::Set_Replace_Score(CVirtualMachine* vm, CVMValue self, int value)
{
	Leaderboard* leaderboard = reinterpret_cast<Leaderboard*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	leaderboard->replace_scores = value != 0;
}

int CRuntime_Leaderboard::Get_Replace_Score(CVirtualMachine* vm, CVMValue self)
{
	Leaderboard* leaderboard = reinterpret_cast<Leaderboard*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return leaderboard->replace_scores;
}

void CRuntime_Leaderboard::Set_Ascending(CVirtualMachine* vm, CVMValue self, int value)
{
	Leaderboard* leaderboard = reinterpret_cast<Leaderboard*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	leaderboard->ascending = value != 0;
}

int CRuntime_Leaderboard::Get_Ascending(CVirtualMachine* vm, CVMValue self)
{
	Leaderboard* leaderboard = reinterpret_cast<Leaderboard*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return leaderboard->ascending;
}


void CRuntime_Leaderboard::Submit(CVirtualMachine* vm, CVMValue self, int value)
{
	Leaderboard* leaderboard = reinterpret_cast<Leaderboard*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	RankingsManager::Get()->Submit(leaderboard, value);
}

void CRuntime_Leaderboard::Bind(CVirtualMachine* vm)
{
	vm->Get_Bindings()->Bind_Method<void,CVMString>	("Leaderboard", "Set_ID",						&Set_ID);
	vm->Get_Bindings()->Bind_Method<CVMString>		("Leaderboard", "Get_ID",						&Get_ID);
	vm->Get_Bindings()->Bind_Method<void,CVMString>	("Leaderboard", "Set_Name",						&Set_Name);
	vm->Get_Bindings()->Bind_Method<CVMString>		("Leaderboard", "Get_Name",						&Get_Name);
	vm->Get_Bindings()->Bind_Method<void,int>		("Leaderboard", "Set_Replace_Score",			&Set_Replace_Score);
	vm->Get_Bindings()->Bind_Method<int>			("Leaderboard", "Get_Replace_Score",			&Get_Replace_Score);
	vm->Get_Bindings()->Bind_Method<void,int>		("Leaderboard", "Set_Ascending",				&Set_Ascending);
	vm->Get_Bindings()->Bind_Method<int>			("Leaderboard", "Get_Ascending",				&Get_Ascending);
	vm->Get_Bindings()->Bind_Method<void,int>		("Leaderboard", "Submit",						&Submit);
}

