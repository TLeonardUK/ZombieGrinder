// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Scripts/Runtime/CRuntime_Challenge.h"
#include "XScript/VirtualMachine/CVMBindingHelper.h"
#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMContext.h"
#include "XScript/VirtualMachine/CVMBinary.h"

#include "Engine/Resources/ResourceFactory.h"

#include "Game/Online/ChallengeManager.h"

void CRuntime_Challenge::Set_ID(CVirtualMachine* vm, CVMValue self, CVMString string)
{
	Challenge* achievement = reinterpret_cast<Challenge*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	achievement->id = string.C_Str();
}

CVMString CRuntime_Challenge::Get_ID(CVirtualMachine* vm, CVMValue self)
{
	Challenge* achievement = reinterpret_cast<Challenge*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return achievement->id.c_str();
}

void CRuntime_Challenge::Set_Name(CVirtualMachine* vm, CVMValue self, CVMString string)
{
	Challenge* achievement = reinterpret_cast<Challenge*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	achievement->name = string.C_Str();
}

CVMString CRuntime_Challenge::Get_Name(CVirtualMachine* vm, CVMValue self)
{
	Challenge* achievement = reinterpret_cast<Challenge*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return achievement->name.c_str();
}

void CRuntime_Challenge::Set_Description(CVirtualMachine* vm, CVMValue self, CVMString string)
{
	Challenge* achievement = reinterpret_cast<Challenge*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	achievement->description = string.C_Str();
}

CVMString CRuntime_Challenge::Get_Description(CVirtualMachine* vm, CVMValue self)
{
	Challenge* achievement = reinterpret_cast<Challenge*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return achievement->description.c_str();
}

void CRuntime_Challenge::Set_Timeframe(CVirtualMachine* vm, CVMValue self, int val)
{
	Challenge* achievement = reinterpret_cast<Challenge*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	achievement->timeframe = (ChallengeTimeframe::Type)val;
}

int CRuntime_Challenge::Get_Timeframe(CVirtualMachine* vm, CVMValue self)
{
	Challenge* achievement = reinterpret_cast<Challenge*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return (int)achievement->timeframe;
}

void CRuntime_Challenge::Set_Requires_Activation(CVirtualMachine* vm, CVMValue self, int val)
{
	Challenge* achievement = reinterpret_cast<Challenge*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	achievement->requires_activation = (val != 0);
}

int CRuntime_Challenge::Get_Requires_Activation(CVirtualMachine* vm, CVMValue self)
{
	Challenge* achievement = reinterpret_cast<Challenge*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return (int)achievement->requires_activation;
}

void CRuntime_Challenge::Set_Max_Progress(CVirtualMachine* vm, CVMValue self, float val)
{
	Challenge* achievement = reinterpret_cast<Challenge*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	achievement->max_progress = val;
}

float CRuntime_Challenge::Get_Max_Progress(CVirtualMachine* vm, CVMValue self)
{
	Challenge* achievement = reinterpret_cast<Challenge*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return achievement->max_progress;
}

void CRuntime_Challenge::Set_Progress(CVirtualMachine* vm, CVMValue self, float val)
{
	Challenge* achievement = reinterpret_cast<Challenge*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	achievement->progress = val;
}

float CRuntime_Challenge::Get_Progress(CVirtualMachine* vm, CVMValue self)
{
	Challenge* achievement = reinterpret_cast<Challenge*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return achievement->progress;
}

int CRuntime_Challenge::Was_Completed(CVirtualMachine* vm, int value)
{
	ChallengeTimeframe::Type type = (ChallengeTimeframe::Type)value;
	Challenge* active = ChallengeManager::Get()->Get_Active_Challenge(type);
	if (active->active)
	{
		int ret = active->was_just_completed ? 1 : 0;
		active->was_just_completed = false;
		return ret;
	}
	return 0;
}

void CRuntime_Challenge::Start_Custom_Map(CVirtualMachine* vm, CVMValue self, CVMString map, int seed)
{
	Challenge* achievement = reinterpret_cast<Challenge*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	ChallengeManager::Get()->Start_Custom_Map(achievement, map.C_Str(), seed);
}

int CRuntime_Challenge::Get_In_Custom_Map(CVirtualMachine* vm, CVMValue self)
{
	Challenge* achievement = reinterpret_cast<Challenge*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return ChallengeManager::Get()->In_Custom_Map(achievement);
}

void CRuntime_Challenge::Set_Specific_Date(CVirtualMachine* vm, CVMValue self, int value)
{
	Challenge* achievement = reinterpret_cast<Challenge*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	achievement->specific_date = !!value;
}

int CRuntime_Challenge::Get_Specific_Date(CVirtualMachine* vm, CVMValue self)
{
	Challenge* achievement = reinterpret_cast<Challenge*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return achievement->specific_date ? 1 : 0;
}

void CRuntime_Challenge::Set_Start_Day(CVirtualMachine* vm, CVMValue self, int value)
{
	Challenge* achievement = reinterpret_cast<Challenge*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	achievement->start_day = value;
}

int CRuntime_Challenge::Get_Start_Day(CVirtualMachine* vm, CVMValue self)
{
	Challenge* achievement = reinterpret_cast<Challenge*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return achievement->start_day;
}

void CRuntime_Challenge::Set_Start_Month(CVirtualMachine* vm, CVMValue self, int value)
{
	Challenge* achievement = reinterpret_cast<Challenge*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	achievement->start_month = value;
}

int CRuntime_Challenge::Get_Start_Month(CVirtualMachine* vm, CVMValue self)
{
	Challenge* achievement = reinterpret_cast<Challenge*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return achievement->start_month;
}

void CRuntime_Challenge::Set_Announce(CVirtualMachine* vm, CVMValue self, int value)
{
	Challenge* achievement = reinterpret_cast<Challenge*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	achievement->announce = !!value;
}

int CRuntime_Challenge::Get_Announce(CVirtualMachine* vm, CVMValue self)
{
	Challenge* achievement = reinterpret_cast<Challenge*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return achievement->announce ? 1 : 0;
}

void CRuntime_Challenge::Set_Announce_Name(CVirtualMachine* vm, CVMValue self, CVMString string)
{
	Challenge* achievement = reinterpret_cast<Challenge*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	achievement->announce_name = string.C_Str();
}

CVMString CRuntime_Challenge::Get_Announce_Name(CVirtualMachine* vm, CVMValue self)
{
	Challenge* achievement = reinterpret_cast<Challenge*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return achievement->announce_name.c_str();
}

void CRuntime_Challenge::Set_Announce_Description(CVirtualMachine* vm, CVMValue self, CVMString string)
{
	Challenge* achievement = reinterpret_cast<Challenge*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	achievement->announce_description = string.C_Str();
}

CVMString CRuntime_Challenge::Get_Announce_Description(CVirtualMachine* vm, CVMValue self)
{
	Challenge* achievement = reinterpret_cast<Challenge*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return achievement->announce_description.c_str();
}

void CRuntime_Challenge::Set_Announce_Image(CVirtualMachine* vm, CVMValue self, CVMString string)
{
	Challenge* achievement = reinterpret_cast<Challenge*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	achievement->announce_image = string.C_Str();
}

CVMString CRuntime_Challenge::Get_Announce_Image(CVirtualMachine* vm, CVMValue self)
{
	Challenge* achievement = reinterpret_cast<Challenge*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return achievement->announce_image.c_str();
}

void CRuntime_Challenge::Bind(CVirtualMachine* vm)
{
	vm->Get_Bindings()->Bind_Method<void, CVMString>("Challenge", "Set_ID", &Set_ID);
	vm->Get_Bindings()->Bind_Method<CVMString>("Challenge", "Get_ID", &Get_ID);
	vm->Get_Bindings()->Bind_Method<void, CVMString>("Challenge", "Set_Name", &Set_Name);
	vm->Get_Bindings()->Bind_Method<CVMString>("Challenge", "Get_Name", &Get_Name);
	vm->Get_Bindings()->Bind_Method<void, CVMString>("Challenge", "Set_Description", &Set_Description);
	vm->Get_Bindings()->Bind_Method<CVMString>("Challenge", "Get_Description", &Get_Description);
	vm->Get_Bindings()->Bind_Method<void, int>("Challenge", "Set_Timeframe", &Set_Timeframe);
	vm->Get_Bindings()->Bind_Method<int>("Challenge", "Get_Timeframe", &Get_Timeframe);
	vm->Get_Bindings()->Bind_Method<void, int>("Challenge", "Set_Requires_Activation", &Set_Requires_Activation);
	vm->Get_Bindings()->Bind_Method<int>("Challenge", "Get_Requires_Activation", &Get_Requires_Activation);
	vm->Get_Bindings()->Bind_Method<void, float>("Challenge", "Set_Max_Progress", &Set_Max_Progress);
	vm->Get_Bindings()->Bind_Method<float>("Challenge", "Get_Max_Progress", &Get_Max_Progress);
	vm->Get_Bindings()->Bind_Method<void, float>("Challenge", "Set_Progress", &Set_Progress);
	vm->Get_Bindings()->Bind_Method<float>("Challenge", "Get_Progress", &Get_Progress);

	vm->Get_Bindings()->Bind_Method<void, int>("Challenge", "Set_Announce", &Set_Announce);
	vm->Get_Bindings()->Bind_Method<int>("Challenge", "Get_Announce", &Get_Announce);
	vm->Get_Bindings()->Bind_Method<void, CVMString>("Challenge", "Set_Announce_Name", &Set_Announce_Name);
	vm->Get_Bindings()->Bind_Method<CVMString>("Challenge", "Get_Announce_Name", &Get_Announce_Name);
	vm->Get_Bindings()->Bind_Method<void, CVMString>("Challenge", "Set_Announce_Description", &Set_Announce_Description);
	vm->Get_Bindings()->Bind_Method<CVMString>("Challenge", "Get_Announce_Description", &Get_Announce_Description);
	vm->Get_Bindings()->Bind_Method<void, CVMString>("Challenge", "Set_Announce_Image", &Set_Announce_Image);
	vm->Get_Bindings()->Bind_Method<CVMString>("Challenge", "Get_Announce_Image", &Get_Announce_Image);

	vm->Get_Bindings()->Bind_Method<void, int>("Challenge", "Set_Specific_Date", &Set_Specific_Date);
	vm->Get_Bindings()->Bind_Method<int>("Challenge", "Get_Specific_Date", &Get_Specific_Date);
	vm->Get_Bindings()->Bind_Method<void, int>("Challenge", "Set_Start_Day", &Set_Start_Day);
	vm->Get_Bindings()->Bind_Method<int>("Challenge", "Get_Start_Day", &Get_Start_Day);
	vm->Get_Bindings()->Bind_Method<void, int>("Challenge", "Set_Start_Month", &Set_Start_Month);
	vm->Get_Bindings()->Bind_Method<int>("Challenge", "Get_Start_Month", &Get_Start_Month);

	vm->Get_Bindings()->Bind_Method<void,CVMString,int>("Challenge", "Start_Custom_Map", &Start_Custom_Map);
	vm->Get_Bindings()->Bind_Method<int>("Challenge", "Get_In_Custom_Map", &Get_In_Custom_Map);
	
	vm->Get_Bindings()->Bind_Function<int, int>("Challenge", "Was_Completed", &Was_Completed);
}

