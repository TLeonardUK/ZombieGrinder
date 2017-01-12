// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Scripts/Runtime/CRuntime_Item_Combination.h"
#include "XScript/VirtualMachine/CVMBindingHelper.h"
#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMContext.h"
#include "XScript/VirtualMachine/CVMBinary.h"

#include "Game/Profile/ItemManager.h"

CVMString CRuntime_Item_Combination::Get_Other_Group(CVirtualMachine* vm, CVMValue self)
{
	ItemCombination* combo = reinterpret_cast<ItemCombination*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return combo->other_group.c_str();
}

void CRuntime_Item_Combination::Set_Other_Group(CVirtualMachine* vm, CVMValue self, CVMString value)
{
	ItemCombination* combo = reinterpret_cast<ItemCombination*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	combo->other_group = value.C_Str();
}

int CRuntime_Item_Combination::Get_Combine_Method(CVirtualMachine* vm, CVMValue self)
{
	ItemCombination* combo = reinterpret_cast<ItemCombination*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return combo->combine_method;
}

void CRuntime_Item_Combination::Set_Combine_Method(CVirtualMachine* vm, CVMValue self, int value)
{
	ItemCombination* combo = reinterpret_cast<ItemCombination*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	combo->combine_method = (ItemCombineMethod::Type)value;
}

CVMString CRuntime_Item_Combination::Get_Result_Item(CVirtualMachine* vm, CVMValue self)
{
	ItemCombination* combo = reinterpret_cast<ItemCombination*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return combo->result_item.c_str();
}

void CRuntime_Item_Combination::Set_Result_Item(CVirtualMachine* vm, CVMValue self, CVMString value)
{
	ItemCombination* combo = reinterpret_cast<ItemCombination*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	combo->result_item = value.C_Str();
}

CVMObjectHandle CRuntime_Item_Combination::Create(CVirtualMachine* vm, CVMValue self, CVMString other, int method, CVMString result)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);

	ItemArchetype* archetype = static_cast<ItemArchetype*>(vm->Get_Active_Context()->MetaData);
	vm->Assert(archetype != NULL);

	ItemCombination* combo = &archetype->combinations[archetype->combination_count++];
	obj->Set_Meta_Data(combo);

	combo->other_group = other.C_Str();
	combo->combine_method = (ItemCombineMethod::Type)method;
	combo->result_item = result.C_Str();

	return obj;
}

void CRuntime_Item_Combination::Bind(CVirtualMachine* vm)
{
	vm->Get_Bindings()->Bind_Method<CVMString>("Item_Combination", "Get_Other_Group", &Get_Other_Group);
	vm->Get_Bindings()->Bind_Method<void,CVMString>("Item_Combination", "Set_Other_Group", &Set_Other_Group);
	vm->Get_Bindings()->Bind_Method<int>("Item_Combination", "Get_Combine_Method", &Get_Combine_Method);
	vm->Get_Bindings()->Bind_Method<void,int>("Item_Combination", "Set_Combine_Method", &Set_Combine_Method);
	vm->Get_Bindings()->Bind_Method<CVMString>("Item_Combination", "Get_Result_Item", &Get_Result_Item);
	vm->Get_Bindings()->Bind_Method<void,CVMString>("Item_Combination", "Set_Result_Item", &Set_Result_Item);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle,CVMString,int,CVMString>("Item_Combination", "Create", &Create);
}
