// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Scripts/Runtime/CRuntime_Item.h"
#include "XScript/VirtualMachine/CVMBindingHelper.h"
#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMContext.h"
#include "XScript/VirtualMachine/CVMBinary.h"

#include "Game/Profile/Profile.h"

#include "Game/Profile/ItemManager.h"

CVMObjectHandle CRuntime_Item::Create(CVirtualMachine* vm, CVMValue self)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);

	Profile* profile = static_cast<Profile*>(vm->Get_Active_Context()->MetaData);
	vm->Assert(profile != NULL);
	
	Item* item = new Item();
	item->equip_slot = -1;
	item->archetype = NULL;
	item->script_object = obj;
	item->was_inventory_drop = false;
	item->item_upgrade_ids = vm->New_Array(vm->IntArray_Class, 0);
	obj->Set_Meta_Data(item);

	return obj;
}

CVMObjectHandle CRuntime_Item::Get_Archetype(CVirtualMachine* vm, CVMValue self)
{
	Item* item = reinterpret_cast<Item*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));

	CVMLinkedSymbol* archetype_class = item->archetype->script_object.Get().Get()->Get_Symbol();

	CVMObjectHandle result = vm->New_Object(archetype_class, false, item->archetype);

	return result;
}

void CRuntime_Item::Set_Archetype(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value)
{
	Item* item = reinterpret_cast<Item*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	ItemArchetype* archetype = reinterpret_cast<ItemArchetype*>(value.Get()->Get_Meta_Data());
	item->archetype = archetype;
}

int CRuntime_Item::Get_Archetype_ID(CVirtualMachine* vm, CVMValue self)
{
	Item* item = reinterpret_cast<Item*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->archetype == NULL ? 0 : item->archetype->script_object.Get().Get()->Get_Symbol()->symbol->unique_id;
}

void CRuntime_Item::Set_Archetype_ID(CVirtualMachine* vm, CVMValue self, int value)
{
	Item* item = reinterpret_cast<Item*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->archetype = ItemManager::Get()->Find_Archetype(value);
}

int CRuntime_Item::Get_Unique_ID(CVirtualMachine* vm, CVMValue self)
{
	Item* item = reinterpret_cast<Item*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->unique_id;
}

void CRuntime_Item::Set_Unique_ID(CVirtualMachine* vm, CVMValue self, int value)
{
	Item* item = reinterpret_cast<Item*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->unique_id = value;
}

int CRuntime_Item::Get_Recieve_Time(CVirtualMachine* vm, CVMValue self)
{
	Item* item = reinterpret_cast<Item*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->recieve_time;
}

void CRuntime_Item::Set_Recieve_Time(CVirtualMachine* vm, CVMValue self, int value)
{
	Item* item = reinterpret_cast<Item*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->recieve_time = value;
}

CVMObjectHandle CRuntime_Item::Get_Primary_Color(CVirtualMachine* vm, CVMValue self)
{
	Item* item = reinterpret_cast<Item*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));

	CVMLinkedSymbol* vec4_class = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);

	CVMObjectHandle result = vm->New_Object(vec4_class, false);
	CVMObject* obj = result.Get();
	obj->Resize(4);
	obj->Get_Slot(0).float_value = item->primary_color.R;
	obj->Get_Slot(1).float_value = item->primary_color.G;
	obj->Get_Slot(2).float_value = item->primary_color.B;
	obj->Get_Slot(3).float_value = item->primary_color.A;

	return result;
}

void CRuntime_Item::Set_Primary_Color(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value)
{
	Item* item = reinterpret_cast<Item*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->primary_color.R = (u8)value.Get()->Get_Slot(0).float_value;
	item->primary_color.G = (u8)value.Get()->Get_Slot(1).float_value;
	item->primary_color.B = (u8)value.Get()->Get_Slot(2).float_value;
	item->primary_color.A = (u8)value.Get()->Get_Slot(3).float_value;
}

int CRuntime_Item::Get_Equip_Time(CVirtualMachine* vm, CVMValue self)
{
	Item* item = reinterpret_cast<Item*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->equip_time;
}

void CRuntime_Item::Set_Equip_Time(CVirtualMachine* vm, CVMValue self, int value)
{
	Item* item = reinterpret_cast<Item*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->equip_time = value;
}

int CRuntime_Item::Get_Equip_Slot(CVirtualMachine* vm, CVMValue self)
{
	Item* item = reinterpret_cast<Item*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->equip_slot;
}

void CRuntime_Item::Set_Equip_Slot(CVirtualMachine* vm, CVMValue self, int value)
{
	Item* item = reinterpret_cast<Item*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->equip_slot = value;
}

int CRuntime_Item::Get_Attached_To_ID(CVirtualMachine* vm, CVMValue self)
{
	Item* item = reinterpret_cast<Item*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->attached_to_id;
}

void CRuntime_Item::Set_Attached_To_ID(CVirtualMachine* vm, CVMValue self, int value)
{
	Item* item = reinterpret_cast<Item*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->attached_to_id = value;
}

int CRuntime_Item::Get_Was_Inventory_Drop(CVirtualMachine* vm, CVMValue self)
{
	Item* item = reinterpret_cast<Item*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->was_inventory_drop;
}

void CRuntime_Item::Set_Was_Inventory_Drop(CVirtualMachine* vm, CVMValue self, int value)
{
	Item* item = reinterpret_cast<Item*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->was_inventory_drop = (value != 0);
}

CVMString CRuntime_Item::Get_Inventory_Drop_ID(CVirtualMachine* vm, CVMValue self)
{
	Item* item = reinterpret_cast<Item*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->inventory_drop_id.c_str();
}

void CRuntime_Item::Set_Inventory_Drop_ID(CVirtualMachine* vm, CVMValue self, CVMString value)
{
	Item* item = reinterpret_cast<Item*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->inventory_drop_id = value.C_Str();
}

CVMString CRuntime_Item::Get_Inventory_Original_Drop_ID(CVirtualMachine* vm, CVMValue self)
{
	Item* item = reinterpret_cast<Item*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->inventory_original_drop_id.c_str();
}

void CRuntime_Item::Set_Inventory_Original_Drop_ID(CVirtualMachine* vm, CVMValue self, CVMString value)
{
	Item* item = reinterpret_cast<Item*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->inventory_original_drop_id = value.C_Str();
}

int CRuntime_Item::Get_Is_Indestructable(CVirtualMachine* vm, CVMValue self)
{
	Item* item = reinterpret_cast<Item*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->indestructable;
}

void CRuntime_Item::Set_Is_Indestructable(CVirtualMachine* vm, CVMValue self, int value)
{
	Item* item = reinterpret_cast<Item*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->indestructable = (value != 0);
}

float CRuntime_Item::Get_Equip_Duration(CVirtualMachine* vm, CVMValue self)
{
	Item* item = reinterpret_cast<Item*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->equip_duration;
}

void CRuntime_Item::Set_Equip_Duration(CVirtualMachine* vm, CVMValue self, float value)
{
	Item* item = reinterpret_cast<Item*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->equip_duration = value;
}

CVMObjectHandle  CRuntime_Item::Get_Upgrade_IDs(CVirtualMachine* vm, CVMValue self)
{
	Item* item = reinterpret_cast<Item*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->item_upgrade_ids;
}

void  CRuntime_Item::Set_Upgrade_IDs(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value)
{
	Item* item = reinterpret_cast<Item*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->item_upgrade_ids = value;
}

void CRuntime_Item::Bind(CVirtualMachine* vm)
{
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>("Item", "Create", &Create);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>("Item", "Get_Archetype", &Get_Archetype);
	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle>("Item", "Set_Archetype", &Set_Archetype);
	vm->Get_Bindings()->Bind_Method<int>("Item", "Get_Archetype_ID", &Get_Archetype_ID);
	vm->Get_Bindings()->Bind_Method<void,int>("Item", "Set_Archetype_ID", &Set_Archetype_ID);
	vm->Get_Bindings()->Bind_Method<int>("Item", "Get_Unique_ID", &Get_Unique_ID);
	vm->Get_Bindings()->Bind_Method<void,int>("Item", "Set_Unique_ID", &Set_Unique_ID);
	vm->Get_Bindings()->Bind_Method<int>("Item", "Get_Attached_To_ID", &Get_Attached_To_ID);
	vm->Get_Bindings()->Bind_Method<void,int>("Item", "Set_Attached_To_ID", &Set_Attached_To_ID);
	vm->Get_Bindings()->Bind_Method<int>("Item", "Get_Recieve_Time", &Get_Recieve_Time);
	vm->Get_Bindings()->Bind_Method<void,int>("Item", "Set_Recieve_Time", &Set_Recieve_Time);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>("Item", "Get_Primary_Color", &Get_Primary_Color);
	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle>("Item", "Set_Primary_Color", &Set_Primary_Color);
	vm->Get_Bindings()->Bind_Method<int>("Item", "Get_Equip_Time", &Get_Equip_Time);
	vm->Get_Bindings()->Bind_Method<void,int>("Item", "Set_Equip_Time", &Set_Equip_Time);
	vm->Get_Bindings()->Bind_Method<int>("Item", "Get_Equip_Slot", &Get_Equip_Slot);
	vm->Get_Bindings()->Bind_Method<void,int>("Item", "Set_Equip_Slot", &Set_Equip_Slot);

	vm->Get_Bindings()->Bind_Method<int>("Item", "Get_Was_Inventory_Drop", &Get_Was_Inventory_Drop);
	vm->Get_Bindings()->Bind_Method<void,int>("Item", "Set_Was_Inventory_Drop", &Set_Was_Inventory_Drop);
	vm->Get_Bindings()->Bind_Method<CVMString>("Item", "Get_Inventory_Drop_ID", &Get_Inventory_Drop_ID);
	vm->Get_Bindings()->Bind_Method<void,CVMString>("Item", "Set_Inventory_Drop_ID", &Set_Inventory_Drop_ID);
	vm->Get_Bindings()->Bind_Method<CVMString>("Item", "Get_Inventory_Original_Drop_ID", &Get_Inventory_Original_Drop_ID);
	vm->Get_Bindings()->Bind_Method<void,CVMString>("Item", "Set_Inventory_Original_Drop_ID", &Set_Inventory_Original_Drop_ID);

	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>("Item", "Get_Upgrade_Ids", &Get_Upgrade_IDs);
	vm->Get_Bindings()->Bind_Method<void, CVMObjectHandle>("Item", "Set_Upgrade_Ids", &Set_Upgrade_IDs);

	vm->Get_Bindings()->Bind_Method<int>("Item", "Get_Is_Indestructable", &Get_Is_Indestructable);
	vm->Get_Bindings()->Bind_Method<void, int>("Item", "Set_Is_Indestructable", &Set_Is_Indestructable);
	vm->Get_Bindings()->Bind_Method<float>("Item", "Get_Equip_Duration", &Get_Equip_Duration);
	vm->Get_Bindings()->Bind_Method<void, float>("Item", "Set_Equip_Duration", &Set_Equip_Duration);
}
