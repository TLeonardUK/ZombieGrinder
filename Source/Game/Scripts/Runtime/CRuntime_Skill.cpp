// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Profile/Profile.h"
#include "Game/Profile/SkillManager.h"
#include "Game/Scripts/Runtime/CRuntime_Skill.h"
#include "XScript/VirtualMachine/CVMBindingHelper.h"
#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMContext.h"
#include "XScript/VirtualMachine/CVMBinary.h"

CVMObjectHandle CRuntime_Skill::Create(CVirtualMachine* vm, CVMValue self)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);

	Profile* profile = static_cast<Profile*>(vm->Get_Active_Context()->MetaData);
	vm->Assert(profile != NULL);

	Skill* skill = new Skill();
	skill->archetype = NULL;
	skill->equip_slot = -1;
	skill->script_object = obj;
	obj->Set_Meta_Data(skill);

	return obj;
}

CVMObjectHandle CRuntime_Skill::Get_Archetype(CVirtualMachine* vm, CVMValue self)
{
	Skill* item = reinterpret_cast<Skill*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));

	CVMLinkedSymbol* archetype_class = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);

	CVMObjectHandle result = vm->New_Object(archetype_class, false, item->archetype);

	return result;
}

void CRuntime_Skill::Set_Archetype(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value)
{
	Skill* item = reinterpret_cast<Skill*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	SkillArchetype* archetype = reinterpret_cast<SkillArchetype*>(value.Get()->Get_Meta_Data());
	item->archetype = archetype;
}

int CRuntime_Skill::Get_Archetype_ID(CVirtualMachine* vm, CVMValue self)
{
	Skill* item = reinterpret_cast<Skill*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->archetype == NULL ? 0 : item->archetype->script_object.Get().Get()->Get_Symbol()->symbol->unique_id;
}

void CRuntime_Skill::Set_Archetype_ID(CVirtualMachine* vm, CVMValue self, int value)
{
	Skill* item = reinterpret_cast<Skill*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->archetype = SkillManager::Get()->Find_Archetype(value);
}

int CRuntime_Skill::Get_Unique_ID(CVirtualMachine* vm, CVMValue self)
{
	Skill* item = reinterpret_cast<Skill*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->unique_id;
}

void CRuntime_Skill::Set_Unique_ID(CVirtualMachine* vm, CVMValue self, int value)
{
	Skill* item = reinterpret_cast<Skill*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->unique_id = value;
}

int CRuntime_Skill::Get_Recieve_Time(CVirtualMachine* vm, CVMValue self)
{
	Skill* item = reinterpret_cast<Skill*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->recieve_time;
}

void CRuntime_Skill::Set_Recieve_Time(CVirtualMachine* vm, CVMValue self, int value)
{
	Skill* item = reinterpret_cast<Skill*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->recieve_time = value;
}

int CRuntime_Skill::Get_Equip_Time(CVirtualMachine* vm, CVMValue self)
{
	Skill* item = reinterpret_cast<Skill*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->equip_time;
}

void CRuntime_Skill::Set_Equip_Time(CVirtualMachine* vm, CVMValue self, int value)
{
	Skill* item = reinterpret_cast<Skill*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->equip_time = value;
}

int CRuntime_Skill::Get_Equip_Slot(CVirtualMachine* vm, CVMValue self)
{
	Skill* item = reinterpret_cast<Skill*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->equip_slot;
}

void CRuntime_Skill::Set_Equip_Slot(CVirtualMachine* vm, CVMValue self, int value)
{
	Skill* item = reinterpret_cast<Skill*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->equip_slot = value;
}

int CRuntime_Skill::Get_Was_Rolled_Back(CVirtualMachine* vm, CVMValue self)
{
	Skill* item = reinterpret_cast<Skill*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->was_rolled_back;
}

void CRuntime_Skill::Set_Was_Rolled_Back(CVirtualMachine* vm, CVMValue self, int value)
{
	Skill* item = reinterpret_cast<Skill*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->was_rolled_back = value != 0;
}

void CRuntime_Skill::Bind(CVirtualMachine* vm)
{
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>("Skill", "Create", &Create);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>("Skill", "Get_Archetype", &Get_Archetype);
	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle>("Skill", "Set_Archetype", &Set_Archetype);
	vm->Get_Bindings()->Bind_Method<int>("Skill", "Get_Archetype_ID", &Get_Archetype_ID);
	vm->Get_Bindings()->Bind_Method<void,int>("Skill", "Set_Archetype_ID", &Set_Archetype_ID);
	vm->Get_Bindings()->Bind_Method<int>("Skill", "Get_Unique_ID", &Get_Unique_ID);
	vm->Get_Bindings()->Bind_Method<void,int>("Skill", "Set_Unique_ID", &Set_Unique_ID);
	vm->Get_Bindings()->Bind_Method<int>("Skill", "Get_Recieve_Time", &Get_Recieve_Time);
	vm->Get_Bindings()->Bind_Method<void,int>("Skill", "Set_Recieve_Time", &Set_Recieve_Time);
	vm->Get_Bindings()->Bind_Method<int>("Skill", "Get_Equip_Time", &Get_Equip_Time);
	vm->Get_Bindings()->Bind_Method<void,int>("Skill", "Set_Equip_Time", &Set_Equip_Time);
	vm->Get_Bindings()->Bind_Method<int>("Skill", "Get_Equip_Slot", &Get_Equip_Slot);
	vm->Get_Bindings()->Bind_Method<void,int>("Skill", "Set_Equip_Slot", &Set_Equip_Slot);
	vm->Get_Bindings()->Bind_Method<int>("Skill", "Get_Was_Rolled_Back", &Get_Was_Rolled_Back);
	vm->Get_Bindings()->Bind_Method<void,int>("Skill", "Set_Was_Rolled_Back", &Set_Was_Rolled_Back);
}
