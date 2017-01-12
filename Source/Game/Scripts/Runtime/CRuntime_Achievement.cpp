// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Scripts/Runtime/CRuntime_Achievement.h"
#include "XScript/VirtualMachine/CVMBindingHelper.h"
#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMContext.h"
#include "XScript/VirtualMachine/CVMBinary.h"

#include "Engine/Resources/ResourceFactory.h"

#include "Game/Online/AchievementManager.h"

void CRuntime_Achievement::Set_ID(CVirtualMachine* vm, CVMValue self, CVMString string)
{
	Achievement* achievement = reinterpret_cast<Achievement*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	achievement->id = string.C_Str();
}

CVMString CRuntime_Achievement::Get_ID(CVirtualMachine* vm, CVMValue self)
{
	Achievement* achievement = reinterpret_cast<Achievement*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return achievement->id.c_str();
}

void CRuntime_Achievement::Set_Name(CVirtualMachine* vm, CVMValue self, CVMString string)
{
	Achievement* achievement = reinterpret_cast<Achievement*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	achievement->name = string.C_Str();
}

CVMString CRuntime_Achievement::Get_Name(CVirtualMachine* vm, CVMValue self)
{
	Achievement* achievement = reinterpret_cast<Achievement*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return achievement->name.c_str();
}

void CRuntime_Achievement::Set_Description(CVirtualMachine* vm, CVMValue self, CVMString string)
{
	Achievement* achievement = reinterpret_cast<Achievement*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	achievement->description = string.C_Str();
}

CVMString CRuntime_Achievement::Get_Description(CVirtualMachine* vm, CVMValue self)
{
	Achievement* achievement = reinterpret_cast<Achievement*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return achievement->description.c_str();
}

void CRuntime_Achievement::Set_Progress(CVirtualMachine* vm, CVMValue self, float value)
{
	Achievement* achievement = reinterpret_cast<Achievement*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	achievement->progress = value;
}

float CRuntime_Achievement::Get_Progress(CVirtualMachine* vm, CVMValue self)
{
	Achievement* achievement = reinterpret_cast<Achievement*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return achievement->progress;
}

void CRuntime_Achievement::Set_Max_Progress(CVirtualMachine* vm, CVMValue self, float value)
{
	Achievement* achievement = reinterpret_cast<Achievement*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	achievement->max_progress = value;
}

float CRuntime_Achievement::Get_Max_Progress(CVirtualMachine* vm, CVMValue self)
{
	Achievement* achievement = reinterpret_cast<Achievement*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return achievement->max_progress;
}

void CRuntime_Achievement::Set_Locked_Frame(CVirtualMachine* vm, CVMValue self, CVMString string)
{
	Achievement* achievement = reinterpret_cast<Achievement*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	achievement->locked_frame_name = string.C_Str();
	achievement->locked_frame = ResourceFactory::Get()->Get_Atlas_Frame(achievement->locked_frame_name.c_str());
}

CVMString CRuntime_Achievement::Get_Locked_Frame(CVirtualMachine* vm, CVMValue self)
{
	Achievement* achievement = reinterpret_cast<Achievement*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return achievement->locked_frame_name.c_str();
}

void CRuntime_Achievement::Set_Unlocked_Frame(CVirtualMachine* vm, CVMValue self, CVMString string)
{
	Achievement* achievement = reinterpret_cast<Achievement*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	achievement->unlocked_frame_name = string.C_Str();
	achievement->unlocked_frame = ResourceFactory::Get()->Get_Atlas_Frame(achievement->unlocked_frame_name.c_str());
}

CVMString CRuntime_Achievement::Get_Unlocked_Frame(CVirtualMachine* vm, CVMValue self)
{
	Achievement* achievement = reinterpret_cast<Achievement*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return achievement->unlocked_frame_name.c_str();
}

void CRuntime_Achievement::Set_Unlocked(CVirtualMachine* vm, CVMValue self, int value)
{
	Achievement* achievement = reinterpret_cast<Achievement*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	achievement->unlocked = value != 0;
}

int CRuntime_Achievement::Get_Unlocked(CVirtualMachine* vm, CVMValue self)
{
	Achievement* achievement = reinterpret_cast<Achievement*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return achievement->unlocked;
}

void CRuntime_Achievement::Set_Track_Statistic(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value)
{
	Achievement* achievement = reinterpret_cast<Achievement*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	CVMLinkedSymbol* class_symbol = reinterpret_cast<CVMLinkedSymbol*>(value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));	
	achievement->track_stat_type = class_symbol;
}

CVMObjectHandle CRuntime_Achievement::Get_Track_Statistic(CVirtualMachine* vm, CVMValue self)
{
	Achievement* achievement = reinterpret_cast<Achievement*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return achievement->track_stat_type == NULL ? NULL : achievement->track_stat_type->type_instance.Get();
}

void CRuntime_Achievement::Set_Use_Statistic_Progress(CVirtualMachine* vm, CVMValue self, int value)
{
	Achievement* achievement = reinterpret_cast<Achievement*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	achievement->use_stat_progress = !!value;
}

int CRuntime_Achievement::Get_Use_Statistic_Progress(CVirtualMachine* vm, CVMValue self)
{
	Achievement* achievement = reinterpret_cast<Achievement*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return achievement->use_stat_progress ? 1 : 0;
}

void CRuntime_Achievement::Bind(CVirtualMachine* vm)
{
	vm->Get_Bindings()->Bind_Method<void,CVMString>	("Achievement", "Set_ID",					&Set_ID);
	vm->Get_Bindings()->Bind_Method<CVMString>		("Achievement", "Get_ID",					&Get_ID);
	vm->Get_Bindings()->Bind_Method<void,CVMString>	("Achievement", "Set_Name",				&Set_Name);
	vm->Get_Bindings()->Bind_Method<CVMString>		("Achievement", "Get_Name",				&Get_Name);
	vm->Get_Bindings()->Bind_Method<void,CVMString>	("Achievement", "Set_Description",		&Set_Description);
	vm->Get_Bindings()->Bind_Method<CVMString>		("Achievement", "Get_Description",		&Get_Description);
	vm->Get_Bindings()->Bind_Method<void,float>		("Achievement", "Set_Progress",			&Set_Progress);
	vm->Get_Bindings()->Bind_Method<float>			("Achievement", "Get_Progress",			&Get_Progress);
	vm->Get_Bindings()->Bind_Method<void,float>		("Achievement", "Set_Max_Progress",		&Set_Max_Progress);
	vm->Get_Bindings()->Bind_Method<float>			("Achievement", "Get_Max_Progress",		&Get_Max_Progress);
	vm->Get_Bindings()->Bind_Method<void,CVMString>	("Achievement", "Set_Locked_Frame",		&Set_Locked_Frame);
	vm->Get_Bindings()->Bind_Method<CVMString>		("Achievement", "Get_Locked_Frame",		&Get_Locked_Frame);
	vm->Get_Bindings()->Bind_Method<void,CVMString>	("Achievement", "Set_Unlocked_Frame",		&Set_Unlocked_Frame);
	vm->Get_Bindings()->Bind_Method<CVMString>		("Achievement", "Get_Unlocked_Frame",		&Get_Unlocked_Frame);
	vm->Get_Bindings()->Bind_Method<void,int>		("Achievement", "Set_Unlocked",			&Set_Unlocked);
	vm->Get_Bindings()->Bind_Method<int>			("Achievement", "Get_Unlocked",			&Get_Unlocked);

	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle>	("Achievement", "Set_Track_Statistic",			&Set_Track_Statistic);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>		("Achievement", "Get_Track_Statistic",			&Get_Track_Statistic);
	vm->Get_Bindings()->Bind_Method<void,int>				("Achievement", "Set_Use_Statistic_Progress",			&Set_Use_Statistic_Progress);
	vm->Get_Bindings()->Bind_Method<int>					("Achievement", "Get_Use_Statistic_Progress",			&Get_Use_Statistic_Progress);
}

