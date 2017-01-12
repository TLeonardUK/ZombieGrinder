// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Scripts/Runtime/CRuntime_Skill_Archetype.h"
#include "XScript/VirtualMachine/CVMBindingHelper.h"
#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMContext.h"
#include "XScript/VirtualMachine/CVMBinary.h"

#include "Game/Profile/SkillManager.h"

CVMString CRuntime_Skill_Archetype::Get_Name(CVirtualMachine* vm, CVMValue self)
{
	SkillArchetype* item = reinterpret_cast<SkillArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	vm->Assert(item->name.c_str() != NULL);
	return item->name.c_str();
}

void CRuntime_Skill_Archetype::Set_Name(CVirtualMachine* vm, CVMValue self, CVMString value)
{
	SkillArchetype* item = reinterpret_cast<SkillArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->name = value.C_Str();
}

CVMString CRuntime_Skill_Archetype::Get_Player_Effect(CVirtualMachine* vm, CVMValue self)
{
	SkillArchetype* item = reinterpret_cast<SkillArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	vm->Assert(item->player_effect.c_str() != NULL);
	return item->player_effect.c_str();
}

void CRuntime_Skill_Archetype::Set_Player_Effect(CVirtualMachine* vm, CVMValue self, CVMString value)
{
	SkillArchetype* item = reinterpret_cast<SkillArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->player_effect = value.C_Str();
}

CVMString CRuntime_Skill_Archetype::Get_Description(CVirtualMachine* vm, CVMValue self)
{
	SkillArchetype* item = reinterpret_cast<SkillArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->description.c_str();
}

void CRuntime_Skill_Archetype::Set_Description(CVirtualMachine* vm, CVMValue self, CVMString value)
{
	SkillArchetype* item = reinterpret_cast<SkillArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->description = value.C_Str();
}

CVMString CRuntime_Skill_Archetype::Get_Icon_Frame(CVirtualMachine* vm, CVMValue self)
{
	SkillArchetype* item = reinterpret_cast<SkillArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->icon_frame.c_str();
}

void CRuntime_Skill_Archetype::Set_Icon_Frame(CVirtualMachine* vm, CVMValue self, CVMString value)
{
	SkillArchetype* item = reinterpret_cast<SkillArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->icon_frame = value.C_Str();
}

int CRuntime_Skill_Archetype::Get_Cost(CVirtualMachine* vm, CVMValue self)
{
	SkillArchetype* item = reinterpret_cast<SkillArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->cost;
}

void CRuntime_Skill_Archetype::Set_Cost(CVirtualMachine* vm, CVMValue self, int value)
{
	SkillArchetype* item = reinterpret_cast<SkillArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->cost = value;
}

CVMString CRuntime_Skill_Archetype::Get_Parent_Name(CVirtualMachine* vm, CVMValue self)
{
	SkillArchetype* item = reinterpret_cast<SkillArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->parent_name.c_str();
}

void CRuntime_Skill_Archetype::Set_Parent_Name(CVirtualMachine* vm, CVMValue self, CVMString value)
{
	SkillArchetype* item = reinterpret_cast<SkillArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->parent_name = value.C_Str();
}

int CRuntime_Skill_Archetype::Get_Is_Passive(CVirtualMachine* vm, CVMValue self)
{
	SkillArchetype* item = reinterpret_cast<SkillArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->is_passive ? 1 : 0;
}

void CRuntime_Skill_Archetype::Set_Is_Passive(CVirtualMachine* vm, CVMValue self, int value)
{
	SkillArchetype* item = reinterpret_cast<SkillArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->is_passive = (value != 0);
}

int CRuntime_Skill_Archetype::Get_Cannot_Rollback(CVirtualMachine* vm, CVMValue self)
{
	SkillArchetype* item = reinterpret_cast<SkillArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->cannot_rollback ? 1 : 0;
}

void CRuntime_Skill_Archetype::Set_Cannot_Rollback(CVirtualMachine* vm, CVMValue self, int value)
{
	SkillArchetype* item = reinterpret_cast<SkillArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->cannot_rollback = (value != 0);
}

int CRuntime_Skill_Archetype::Get_Are_Children_Mutex(CVirtualMachine* vm, CVMValue self)
{
	SkillArchetype* item = reinterpret_cast<SkillArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->are_children_mutex ? 1 : 0;
}

void CRuntime_Skill_Archetype::Set_Are_Children_Mutex(CVirtualMachine* vm, CVMValue self, int value)
{
	SkillArchetype* item = reinterpret_cast<SkillArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->are_children_mutex = (value != 0);
}

int CRuntime_Skill_Archetype::Get_Is_Team_Based(CVirtualMachine* vm, CVMValue self)
{
	SkillArchetype* item = reinterpret_cast<SkillArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->team_based ? 1 : 0;
}

void CRuntime_Skill_Archetype::Set_Is_Team_Based(CVirtualMachine* vm, CVMValue self, int value)
{
	SkillArchetype* item = reinterpret_cast<SkillArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->team_based = !!value;
}

float CRuntime_Skill_Archetype::Get_Duration(CVirtualMachine* vm, CVMValue self)
{
	SkillArchetype* item = reinterpret_cast<SkillArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->duration;
}

void CRuntime_Skill_Archetype::Set_Duration(CVirtualMachine* vm, CVMValue self, float value)
{
	SkillArchetype* item = reinterpret_cast<SkillArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->duration = value;
}

float CRuntime_Skill_Archetype::Get_Energy_Required(CVirtualMachine* vm, CVMValue self)
{
	SkillArchetype* item = reinterpret_cast<SkillArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->energy_required;
}

void CRuntime_Skill_Archetype::Set_Energy_Required(CVirtualMachine* vm, CVMValue self, float value)
{
	SkillArchetype* item = reinterpret_cast<SkillArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->energy_required = value;
}

int CRuntime_Skill_Archetype::Get_Energy_Type(CVirtualMachine* vm, CVMValue self)
{
	SkillArchetype* item = reinterpret_cast<SkillArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->energy_type;
}

void CRuntime_Skill_Archetype::Set_Energy_Type(CVirtualMachine* vm, CVMValue self, int value)
{
	SkillArchetype* item = reinterpret_cast<SkillArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->energy_type = (SkillEnergyType::Type)value;
}

int CRuntime_Skill_Archetype::Get_Unlock_Criteria(CVirtualMachine* vm, CVMValue self)
{
	SkillArchetype* item = reinterpret_cast<SkillArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->unlock_critera;
}

void CRuntime_Skill_Archetype::Set_Unlock_Criteria(CVirtualMachine* vm, CVMValue self, int value)
{
	SkillArchetype* item = reinterpret_cast<SkillArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->unlock_critera = (SkillUnlockCriteria::Type)value;
}

int CRuntime_Skill_Archetype::Get_Unlock_Criteria_Threshold(CVirtualMachine* vm, CVMValue self)
{
	SkillArchetype* item = reinterpret_cast<SkillArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->unlock_critera_threshold;
}

void CRuntime_Skill_Archetype::Set_Unlock_Criteria_Threshold(CVirtualMachine* vm, CVMValue self, int value)
{
	SkillArchetype* item = reinterpret_cast<SkillArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->unlock_critera_threshold = value;
}

CVMObjectHandle CRuntime_Skill_Archetype::Get_Unlock_Criteria_Item(CVirtualMachine* vm, CVMValue self)
{
	SkillArchetype* item = reinterpret_cast<SkillArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->unlock_critera_item->type_instance;
}

void CRuntime_Skill_Archetype::Set_Unlock_Criteria_Item(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value)
{
	SkillArchetype* item = reinterpret_cast<SkillArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	CVMLinkedSymbol* type_class = reinterpret_cast<CVMLinkedSymbol*>(value.Get()->Get_Meta_Data());
	item->unlock_critera_item = type_class;
}


CVMObjectHandle CRuntime_Skill_Archetype::Find_By_Name(CVirtualMachine* vm, CVMString name)
{
	CVMLinkedSymbol* archetype_class = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);
	
	SkillArchetype* skill = SkillManager::Get()->Find_Archetype(name.C_Str());

	CVMObjectHandle result = vm->New_Object(archetype_class, false, skill);

	return result;
}

CVMObjectHandle CRuntime_Skill_Archetype::Find_By_ID(CVirtualMachine* vm, int name)
{
	SkillArchetype* type = SkillManager::Get()->Find_Archetype(name);
	return type == NULL ? NULL : type->script_object.Get();
}

void CRuntime_Skill_Archetype::Bind(CVirtualMachine* vm)
{
	vm->Get_Bindings()->Bind_Method<CVMString>("Skill_Archetype", "Get_Name", &Get_Name);
	vm->Get_Bindings()->Bind_Method<void,CVMString>("Skill_Archetype", "Set_Name", &Set_Name);
	vm->Get_Bindings()->Bind_Method<CVMString>("Skill_Archetype", "Get_Description", &Get_Description);
	vm->Get_Bindings()->Bind_Method<void,CVMString>("Skill_Archetype", "Set_Description", &Set_Description);
	vm->Get_Bindings()->Bind_Method<CVMString>("Skill_Archetype", "Get_Icon_Frame", &Get_Icon_Frame);
	vm->Get_Bindings()->Bind_Method<void,CVMString>("Skill_Archetype", "Set_Icon_Frame", &Set_Icon_Frame);
	vm->Get_Bindings()->Bind_Method<int>("Skill_Archetype", "Get_Cost", &Get_Cost);
	vm->Get_Bindings()->Bind_Method<void,int>("Skill_Archetype", "Set_Cost", &Set_Cost);
	vm->Get_Bindings()->Bind_Method<int>("Skill_Archetype", "Get_Cannot_Rollback", &Get_Cannot_Rollback);
	vm->Get_Bindings()->Bind_Method<void,int>("Skill_Archetype", "Set_Cannot_Rollback", &Set_Cannot_Rollback);
	vm->Get_Bindings()->Bind_Method<int>("Skill_Archetype", "Get_Is_Passive", &Get_Is_Passive);
	vm->Get_Bindings()->Bind_Method<void,int>("Skill_Archetype", "Set_Is_Passive", &Set_Is_Passive);
	vm->Get_Bindings()->Bind_Method<int>("Skill_Archetype", "Get_Are_Children_Mutex", &Get_Are_Children_Mutex);
	vm->Get_Bindings()->Bind_Method<void,int>("Skill_Archetype", "Set_Are_Children_Mutex", &Set_Are_Children_Mutex);
	vm->Get_Bindings()->Bind_Method<CVMString>("Skill_Archetype", "Get_Parent_Name", &Get_Parent_Name);
	vm->Get_Bindings()->Bind_Method<void,CVMString>("Skill_Archetype", "Set_Parent_Name", &Set_Parent_Name);

	vm->Get_Bindings()->Bind_Method<int>("Skill_Archetype", "Get_Unlock_Criteria", &Get_Unlock_Criteria);
	vm->Get_Bindings()->Bind_Method<void, int>("Skill_Archetype", "Set_Unlock_Criteria", &Set_Unlock_Criteria);
	vm->Get_Bindings()->Bind_Method<int>("Skill_Archetype", "Get_Unlock_Criteria_Threshold", &Get_Unlock_Criteria_Threshold);
	vm->Get_Bindings()->Bind_Method<void, int>("Skill_Archetype", "Set_Unlock_Criteria_Threshold", &Set_Unlock_Criteria_Threshold);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>("Skill_Archetype", "Get_Unlock_Criteria_Item", &Get_Unlock_Criteria_Item);
	vm->Get_Bindings()->Bind_Method<void, CVMObjectHandle>("Skill_Archetype", "Set_Unlock_Criteria_Item", &Set_Unlock_Criteria_Item);

	vm->Get_Bindings()->Bind_Method<float>("Skill_Archetype", "Get_Duration", &Get_Duration);
	vm->Get_Bindings()->Bind_Method<void,float>("Skill_Archetype", "Set_Duration", &Set_Duration);
	vm->Get_Bindings()->Bind_Method<int>("Skill_Archetype", "Get_Is_Team_Based", &Get_Is_Team_Based);
	vm->Get_Bindings()->Bind_Method<void,int>("Skill_Archetype", "Set_Is_Team_Based", &Set_Is_Team_Based);

	vm->Get_Bindings()->Bind_Method<float>("Skill_Archetype", "Get_Energy_Required", &Get_Energy_Required);
	vm->Get_Bindings()->Bind_Method<void, float>("Skill_Archetype", "Set_Energy_Required", &Set_Energy_Required);
	vm->Get_Bindings()->Bind_Method<int>("Skill_Archetype", "Get_Energy_Type", &Get_Energy_Type);
	vm->Get_Bindings()->Bind_Method<void, int>("Skill_Archetype", "Set_Energy_Type", &Set_Energy_Type);


	vm->Get_Bindings()->Bind_Method<CVMString>("Skill_Archetype", "Get_Player_Effect", &Get_Player_Effect);
	vm->Get_Bindings()->Bind_Method<void,CVMString>("Skill_Archetype", "Set_Player_Effect", &Set_Player_Effect);

	vm->Get_Bindings()->Bind_Function<CVMObjectHandle,CVMString>("Skill_Archetype", "Find_By_Name", &Find_By_Name);
	vm->Get_Bindings()->Bind_Function<CVMObjectHandle,int>("Skill_Archetype", "Find_By_ID", &Find_By_ID);
}


