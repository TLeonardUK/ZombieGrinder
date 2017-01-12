// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Scripts/Runtime/CRuntime_Item_Upgrade_Tree_Node.h"
#include "XScript/VirtualMachine/CVMBindingHelper.h"
#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMContext.h"
#include "XScript/VirtualMachine/CVMBinary.h"

#include "Game/Profile/Profile.h"

#include "Game/Profile/ItemManager.h"

CVMObjectHandle CRuntime_Item_Upgrade_Tree_Node::Create(CVirtualMachine* vm, CVMValue self)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);

	ItemArchetype* profile = static_cast<ItemArchetype*>(vm->Get_Active_Context()->MetaData);
	vm->Assert(profile != NULL);

	ItemUpgradeTreeNode* item = new ItemUpgradeTreeNode();
	item->script_object = obj;
	obj->Set_Meta_Data(item);

	return obj;
}

CVMObjectHandle CRuntime_Item_Upgrade_Tree_Node::Get_Position(CVirtualMachine* vm, CVMValue self)
{
	ItemUpgradeTreeNode* item = reinterpret_cast<ItemUpgradeTreeNode*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return vm->Create_Vec2((float)item->x, (float)item->y);
}

void CRuntime_Item_Upgrade_Tree_Node::Set_Position(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value)
{
	ItemUpgradeTreeNode* item = reinterpret_cast<ItemUpgradeTreeNode*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->x = (int)value.Get()->Get_Slot(0).float_value;
	item->y = (int)value.Get()->Get_Slot(1).float_value;
}

int CRuntime_Item_Upgrade_Tree_Node::Get_ID(CVirtualMachine* vm, CVMValue self)
{
	ItemUpgradeTreeNode* item = reinterpret_cast<ItemUpgradeTreeNode*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->id;
}

CVMString CRuntime_Item_Upgrade_Tree_Node::Get_Name(CVirtualMachine* vm, CVMValue self)
{
	ItemUpgradeTreeNode* item = reinterpret_cast<ItemUpgradeTreeNode*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->name.c_str();
}

void CRuntime_Item_Upgrade_Tree_Node::Set_Name(CVirtualMachine* vm, CVMValue self, CVMString value)
{
	ItemUpgradeTreeNode* item = reinterpret_cast<ItemUpgradeTreeNode*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->name = value.C_Str();
	item->id = StringHelper::Hash(item->name.c_str());
}

CVMObjectHandle CRuntime_Item_Upgrade_Tree_Node::Get_Gem_Type(CVirtualMachine* vm, CVMValue self)
{
	ItemUpgradeTreeNode* item = reinterpret_cast<ItemUpgradeTreeNode*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	if (item->gem_type == NULL)
	{
		return NULL;
	}
	return item->gem_type->type_instance;
}

void CRuntime_Item_Upgrade_Tree_Node::Set_Gem_Type(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value)
{
	ItemUpgradeTreeNode* item = reinterpret_cast<ItemUpgradeTreeNode*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	if (value.Get() != NULL)
	{
		CVMLinkedSymbol* type_class = reinterpret_cast<CVMLinkedSymbol*>(value.Get()->Get_Meta_Data());
		item->gem_type = type_class;
	}
	else
	{
		item->gem_type = NULL;
	}
}

int CRuntime_Item_Upgrade_Tree_Node::Get_Connect_Up(CVirtualMachine* vm, CVMValue self)
{
	ItemUpgradeTreeNode* item = reinterpret_cast<ItemUpgradeTreeNode*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->bConnectUp ? 1 : 0;
}

void CRuntime_Item_Upgrade_Tree_Node::Set_Connect_Up(CVirtualMachine* vm, CVMValue self, int value)
{
	ItemUpgradeTreeNode* item = reinterpret_cast<ItemUpgradeTreeNode*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->bConnectUp = !!value;
}

int CRuntime_Item_Upgrade_Tree_Node::Get_Connect_Down(CVirtualMachine* vm, CVMValue self)
{
	ItemUpgradeTreeNode* item = reinterpret_cast<ItemUpgradeTreeNode*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->bConnectDown ? 1 : 0;
}

void CRuntime_Item_Upgrade_Tree_Node::Set_Connect_Down(CVirtualMachine* vm, CVMValue self, int value)
{
	ItemUpgradeTreeNode* item = reinterpret_cast<ItemUpgradeTreeNode*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->bConnectDown = !!value;
}

int CRuntime_Item_Upgrade_Tree_Node::Get_Connect_Left(CVirtualMachine* vm, CVMValue self)
{
	ItemUpgradeTreeNode* item = reinterpret_cast<ItemUpgradeTreeNode*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->bConnectLeft ? 1 : 0;
}

void CRuntime_Item_Upgrade_Tree_Node::Set_Connect_Left(CVirtualMachine* vm, CVMValue self, int value)
{
	ItemUpgradeTreeNode* item = reinterpret_cast<ItemUpgradeTreeNode*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->bConnectLeft = !!value;
}

int CRuntime_Item_Upgrade_Tree_Node::Get_Connect_Right(CVirtualMachine* vm, CVMValue self)
{
	ItemUpgradeTreeNode* item = reinterpret_cast<ItemUpgradeTreeNode*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->bConnectRight ? 1  : 0;
}

void CRuntime_Item_Upgrade_Tree_Node::Set_Connect_Right(CVirtualMachine* vm, CVMValue self, int value)
{
	ItemUpgradeTreeNode* item = reinterpret_cast<ItemUpgradeTreeNode*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->bConnectRight = !!value;
}

int CRuntime_Item_Upgrade_Tree_Node::Get_Cost(CVirtualMachine* vm, CVMValue self)
{
	ItemUpgradeTreeNode* item = reinterpret_cast<ItemUpgradeTreeNode*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->cost;
}

void CRuntime_Item_Upgrade_Tree_Node::Set_Cost(CVirtualMachine* vm, CVMValue self, int value)
{
	ItemUpgradeTreeNode* item = reinterpret_cast<ItemUpgradeTreeNode*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->cost = value;
}

CVMObjectHandle CRuntime_Item_Upgrade_Tree_Node::Get_Modifiers(CVirtualMachine* vm, CVMValue self)
{
	ItemUpgradeTreeNode* item = reinterpret_cast<ItemUpgradeTreeNode*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->modifiers;
}

void CRuntime_Item_Upgrade_Tree_Node::Set_Modifiers(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value)
{
	ItemUpgradeTreeNode* item = reinterpret_cast<ItemUpgradeTreeNode*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->modifiers = value;
}

void CRuntime_Item_Upgrade_Tree_Node::Bind(CVirtualMachine* vm)
{
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>("Item_Upgrade_Tree_Node", "Get_Position", &Get_Position);
	vm->Get_Bindings()->Bind_Method<void, CVMObjectHandle>("Item_Upgrade_Tree_Node", "Set_Position", &Set_Position);
	vm->Get_Bindings()->Bind_Method<int>("Item_Upgrade_Tree_Node", "Get_ID", &Get_ID);
	vm->Get_Bindings()->Bind_Method<CVMString>("Item_Upgrade_Tree_Node", "Get_Name", &Get_Name);
	vm->Get_Bindings()->Bind_Method<void, CVMString>("Item_Upgrade_Tree_Node", "Set_Name", &Set_Name);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>("Item_Upgrade_Tree_Node", "Get_Gem_Type", &Get_Gem_Type);
	vm->Get_Bindings()->Bind_Method<void, CVMObjectHandle>("Item_Upgrade_Tree_Node", "Set_Gem_Type", &Set_Gem_Type);
	vm->Get_Bindings()->Bind_Method<int>("Item_Upgrade_Tree_Node", "Get_Connect_Up", &Get_Connect_Up);
	vm->Get_Bindings()->Bind_Method<void, int>("Item_Upgrade_Tree_Node", "Set_Connect_Up", &Set_Connect_Up);
	vm->Get_Bindings()->Bind_Method<int>("Item_Upgrade_Tree_Node", "Get_Connect_Down", &Get_Connect_Down);
	vm->Get_Bindings()->Bind_Method<void, int>("Item_Upgrade_Tree_Node", "Set_Connect_Down", &Set_Connect_Down);
	vm->Get_Bindings()->Bind_Method<int>("Item_Upgrade_Tree_Node", "Get_Connect_Left", &Get_Connect_Left);
	vm->Get_Bindings()->Bind_Method<void, int>("Item_Upgrade_Tree_Node", "Set_Connect_Left", &Set_Connect_Left);
	vm->Get_Bindings()->Bind_Method<int>("Item_Upgrade_Tree_Node", "Get_Connect_Right", &Get_Connect_Right);
	vm->Get_Bindings()->Bind_Method<void, int>("Item_Upgrade_Tree_Node", "Set_Connect_Right", &Set_Connect_Right);
	vm->Get_Bindings()->Bind_Method<int>("Item_Upgrade_Tree_Node", "Get_Cost", &Get_Cost);
	vm->Get_Bindings()->Bind_Method<void, int>("Item_Upgrade_Tree_Node", "Set_Cost", &Set_Cost);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>("Item_Upgrade_Tree_Node", "Get_Modifiers", &Get_Modifiers);
	vm->Get_Bindings()->Bind_Method<void, CVMObjectHandle>("Item_Upgrade_Tree_Node", "Set_Modifiers", &Set_Modifiers);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>("Item_Upgrade_Tree_Node", "Create", &Create);
}
