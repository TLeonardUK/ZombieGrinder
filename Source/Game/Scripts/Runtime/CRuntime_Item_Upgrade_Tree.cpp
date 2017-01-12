// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Scripts/Runtime/CRuntime_Item_Upgrade_Tree.h"
#include "XScript/VirtualMachine/CVMBindingHelper.h"
#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMContext.h"
#include "XScript/VirtualMachine/CVMBinary.h"

#include "Game/Profile/Profile.h"

#include "Game/Profile/ItemManager.h"

CVMObjectHandle CRuntime_Item_Upgrade_Tree::Create(CVirtualMachine* vm, CVMValue self)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);

	ItemArchetype* profile = static_cast<ItemArchetype*>(vm->Get_Active_Context()->MetaData);
	vm->Assert(profile != NULL);

	ItemUpgradeTree* item = new ItemUpgradeTree();
	item->script_object = obj;
	obj->Set_Meta_Data(item);

	return obj;
}

CVMObjectHandle CRuntime_Item_Upgrade_Tree::Get_Active_Compound_Modifiers(CVirtualMachine* vm, CVMValue self, CVMObjectHandle upgrade_ids)
{
	ItemUpgradeTree* item = reinterpret_cast<ItemUpgradeTree*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));

	std::vector<int> upgrade_ids_vec;
	std::vector<ItemUpgradeTreeNodeModifier> modifiers;

	for (int i = 0; i < upgrade_ids.Get()->Slot_Count(); i++)
	{
		upgrade_ids_vec.push_back(upgrade_ids.Get()->Get_Slot(i).int_value);
	}

	item->Get_Active_Compound_Modifiers(modifiers, upgrade_ids_vec);

	CVMObjectHandle float_modifier_values = vm->New_Array(vm->FloatArray_Class, ItemUpgradeTreeNodeModifierType::COUNT);

	for (int i = 0; i < ItemUpgradeTreeNodeModifierType::COUNT; i++)
	{
		float_modifier_values.Get()->Get_Slot(i).float_value = 0.0f;
	}

	for (std::vector<ItemUpgradeTreeNodeModifier>::iterator iter = modifiers.begin(); iter != modifiers.end(); iter++)
	{
		ItemUpgradeTreeNodeModifier& mod = *iter;
		float_modifier_values.Get()->Get_Slot((int)mod.modifier).float_value = mod.scale;
	}
	
	/*
	for (int i = 0; i < ItemUpgradeTreeNodeModifierType::COUNT; i++)
	{
		float_modifier_values.Get()->Get_Slot(i).float_value = 0.3f;
	}
	*/

   	return float_modifier_values;
}

CVMObjectHandle CRuntime_Item_Upgrade_Tree::Get_Nodes(CVirtualMachine* vm, CVMValue self)
{
	ItemUpgradeTree* item = reinterpret_cast<ItemUpgradeTree*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->nodes;
}

void CRuntime_Item_Upgrade_Tree::Set_Nodes(CVirtualMachine* vm, CVMValue self, CVMObjectHandle nodes)
{
	ItemUpgradeTree* item = reinterpret_cast<ItemUpgradeTree*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->nodes = nodes;
}

CVMObjectHandle CRuntime_Item_Upgrade_Tree::Find_By_Type(CVirtualMachine* vm, CVMObjectHandle name)
{
	CVMLinkedSymbol* class_symbol = reinterpret_cast<CVMLinkedSymbol*>(name.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));

	ItemUpgradeTree* item = ItemManager::Get()->Find_Upgrade_Tree(class_symbol);
	vm->Assert(item != NULL);

	return item->script_object;
}

void CRuntime_Item_Upgrade_Tree::Bind(CVirtualMachine* vm)
{
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>("Item_Upgrade_Tree", "Create", &Create);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>("Item_Upgrade_Tree", "Get_Nodes", &Get_Nodes);
	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle>("Item_Upgrade_Tree", "Set_Nodes", &Set_Nodes);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle, CVMObjectHandle>("Item_Upgrade_Tree", "Get_Active_Compound_Modifiers", &Get_Active_Compound_Modifiers);
	vm->Get_Bindings()->Bind_Function<CVMObjectHandle, CVMObjectHandle>("Item_Upgrade_Tree", "Find_By_Type", &Find_By_Type);
}
