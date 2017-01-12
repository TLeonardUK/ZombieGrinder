// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Scripts/Runtime/CRuntime_Item_Upgrade_Tree_Node_Modifier.h"
#include "XScript/VirtualMachine/CVMBindingHelper.h"
#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMContext.h"
#include "XScript/VirtualMachine/CVMBinary.h"

#include "Game/Profile/Profile.h"

#include "Game/Profile/ItemManager.h"

CVMObjectHandle CRuntime_Item_Upgrade_Tree_Node_Modifier::Create(CVirtualMachine* vm, CVMValue self)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);

	ItemArchetype* profile = static_cast<ItemArchetype*>(vm->Get_Active_Context()->MetaData);
	vm->Assert(profile != NULL);

	ItemUpgradeTreeNodeModifier* item = new ItemUpgradeTreeNodeModifier();
	item->script_object = obj;
	obj->Set_Meta_Data(item);

	return obj;
}

int CRuntime_Item_Upgrade_Tree_Node_Modifier::Get_Modifier(CVirtualMachine* vm, CVMValue self)
{
	ItemUpgradeTreeNodeModifier* item = reinterpret_cast<ItemUpgradeTreeNodeModifier*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return (int)item->modifier;
}

void CRuntime_Item_Upgrade_Tree_Node_Modifier::Set_Modifier(CVirtualMachine* vm, CVMValue self, int value)
{
	ItemUpgradeTreeNodeModifier* item = reinterpret_cast<ItemUpgradeTreeNodeModifier*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->modifier = static_cast<ItemUpgradeTreeNodeModifierType::Type>(value);
}

float CRuntime_Item_Upgrade_Tree_Node_Modifier::Get_Scale(CVirtualMachine* vm, CVMValue self)
{
	ItemUpgradeTreeNodeModifier* item = reinterpret_cast<ItemUpgradeTreeNodeModifier*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->scale;
}

void CRuntime_Item_Upgrade_Tree_Node_Modifier::Set_Scale(CVirtualMachine* vm, CVMValue self, float value)
{
	ItemUpgradeTreeNodeModifier* item = reinterpret_cast<ItemUpgradeTreeNodeModifier*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->scale = value;
}

void CRuntime_Item_Upgrade_Tree_Node_Modifier::Bind(CVirtualMachine* vm)
{
	vm->Get_Bindings()->Bind_Method<int>("Item_Upgrade_Tree_Node_Modifier", "Get_Modifier", &Get_Modifier);
	vm->Get_Bindings()->Bind_Method<void, int>("Item_Upgrade_Tree_Node_Modifier", "Set_Modifier", &Set_Modifier);
	vm->Get_Bindings()->Bind_Method<float>("Item_Upgrade_Tree_Node_Modifier", "Get_Scale", &Get_Scale);
	vm->Get_Bindings()->Bind_Method<void, float>("Item_Upgrade_Tree_Node_Modifier", "Set_Scale", &Set_Scale);

	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>("Item_Upgrade_Tree_Node_Modifier", "Create", &Create);
}
