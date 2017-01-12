// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Scripts/Runtime/CRuntime_Item_Archetype.h"
#include "XScript/VirtualMachine/CVMBindingHelper.h"
#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMContext.h"
#include "XScript/VirtualMachine/CVMBinary.h"
#include "Engine/Resources/ResourceFactory.h"

#include "Game/Profile/ItemManager.h"

CVMString CRuntime_Item_Archetype::Get_Name(CVirtualMachine* vm, CVMValue self)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->base_name.c_str();
}

void CRuntime_Item_Archetype::Set_Name(CVirtualMachine* vm, CVMValue self, CVMString value)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->base_name = value.C_Str();
}

CVMString CRuntime_Item_Archetype::Get_Override_Inventory_ID_Name(CVirtualMachine* vm, CVMValue self)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->override_inventory_id_name.c_str();
}

void CRuntime_Item_Archetype::Set_Override_Inventory_ID_Name(CVirtualMachine* vm, CVMValue self, CVMString value)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->override_inventory_id_name = value.C_Str();
}

CVMString CRuntime_Item_Archetype::Get_Varient_Prefix(CVirtualMachine* vm, CVMValue self)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->varient_prefix.c_str();
}

void CRuntime_Item_Archetype::Set_Varient_Prefix(CVirtualMachine* vm, CVMValue self, CVMString value)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->varient_prefix = value.C_Str();
}

CVMString CRuntime_Item_Archetype::Get_Description(CVirtualMachine* vm, CVMValue self)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->base_description.c_str();
}

void CRuntime_Item_Archetype::Set_Description(CVirtualMachine* vm, CVMValue self, CVMString value)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->base_description = value.C_Str();
}

CVMString CRuntime_Item_Archetype::Get_Category(CVirtualMachine* vm, CVMValue self)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->category.c_str();
}

void CRuntime_Item_Archetype::Set_Category(CVirtualMachine* vm, CVMValue self, CVMString value)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->category = value.C_Str();
}

int CRuntime_Item_Archetype::Get_Slot(CVirtualMachine* vm, CVMValue self)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->item_slot;
}

void CRuntime_Item_Archetype::Set_Slot(CVirtualMachine* vm, CVMValue self, int value)
{	
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->item_slot = value;
}

int CRuntime_Item_Archetype::Get_Cost(CVirtualMachine* vm, CVMValue self)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->cost;
}

void CRuntime_Item_Archetype::Set_Cost(CVirtualMachine* vm, CVMValue self, int value)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->cost = value;
}

int CRuntime_Item_Archetype::Get_Is_Stackable(CVirtualMachine* vm, CVMValue self)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->is_stackable ? 1 : 0;
}

void CRuntime_Item_Archetype::Set_Is_Stackable(CVirtualMachine* vm, CVMValue self, int value)
{	
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->is_stackable = (value != 0);
}

int CRuntime_Item_Archetype::Get_Use_Custom_Color(CVirtualMachine* vm, CVMValue self)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->use_custom_color ? 1 : 0;
}

void CRuntime_Item_Archetype::Set_Use_Custom_Color(CVirtualMachine* vm, CVMValue self, int value)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->use_custom_color = (value != 0);
}


int CRuntime_Item_Archetype::Get_Unlock_Criteria(CVirtualMachine* vm, CVMValue self)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->unlock_critera;
}

void CRuntime_Item_Archetype::Set_Unlock_Criteria(CVirtualMachine* vm, CVMValue self, int value)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->unlock_critera = (ItemUnlockCriteria::Type)value;
}

int CRuntime_Item_Archetype::Get_Unlock_Criteria_Threshold(CVirtualMachine* vm, CVMValue self)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->unlock_critera_threshold;
}

void CRuntime_Item_Archetype::Set_Unlock_Criteria_Threshold(CVirtualMachine* vm, CVMValue self, int value)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->unlock_critera_threshold = value;
}

CVMObjectHandle CRuntime_Item_Archetype::Get_Unlock_Criteria_Item(CVirtualMachine* vm, CVMValue self)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->unlock_critera_item->type_instance;
}

void CRuntime_Item_Archetype::Set_Unlock_Criteria_Item(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	CVMLinkedSymbol* type_class = reinterpret_cast<CVMLinkedSymbol*>(value.Get()->Get_Meta_Data());
	item->unlock_critera_item = type_class;
}


int CRuntime_Item_Archetype::Get_Is_Buyable(CVirtualMachine* vm, CVMValue self)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->is_buyable ? 1 : 0;
}

void CRuntime_Item_Archetype::Set_Is_Buyable(CVirtualMachine* vm, CVMValue self, int value)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->is_buyable = (value != 0);
}

int CRuntime_Item_Archetype::Get_Is_Tintable(CVirtualMachine* vm, CVMValue self)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->is_tintable ? 1 : 0;
}

void CRuntime_Item_Archetype::Set_Is_Tintable(CVirtualMachine* vm, CVMValue self, int value)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->is_tintable = (value != 0);
}

int CRuntime_Item_Archetype::Get_Is_Unpackable(CVirtualMachine* vm, CVMValue self)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->is_unpackable ? 1 : 0;
}

void CRuntime_Item_Archetype::Set_Is_Unpackable(CVirtualMachine* vm, CVMValue self, int value)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->is_unpackable = (value != 0);
}

int CRuntime_Item_Archetype::Get_Is_Sellable(CVirtualMachine* vm, CVMValue self)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->is_sellable ? 1 : 0;
}

void CRuntime_Item_Archetype::Set_Is_Sellable(CVirtualMachine* vm, CVMValue self, int value)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->is_sellable = (value != 0);
}

int CRuntime_Item_Archetype::Get_Is_Tradable(CVirtualMachine* vm, CVMValue self)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->is_tradable ? 1 : 0;
}

void CRuntime_Item_Archetype::Set_Is_Tradable(CVirtualMachine* vm, CVMValue self, int value)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->is_tradable = (value != 0);
}

int CRuntime_Item_Archetype::Get_Is_Dropable(CVirtualMachine* vm, CVMValue self)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->is_dropable ? 1 : 0;
}

void CRuntime_Item_Archetype::Set_Is_Dropable(CVirtualMachine* vm, CVMValue self, int value)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->is_dropable = (value != 0);
}

int CRuntime_Item_Archetype::Get_Is_Inventory_Dropable(CVirtualMachine* vm, CVMValue self)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->is_inventory_droppable ? 1 : 0;
}

void CRuntime_Item_Archetype::Set_Is_Inventory_Dropable(CVirtualMachine* vm, CVMValue self, int value)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->is_inventory_droppable = (value != 0);
}

int CRuntime_Item_Archetype::Get_Is_PVP_Usable(CVirtualMachine* vm, CVMValue self)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->is_pvp_usable ? 1 : 0;
}

void CRuntime_Item_Archetype::Set_Is_PVP_Usable(CVirtualMachine* vm, CVMValue self, int value)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->is_pvp_usable = (value != 0);
}

float CRuntime_Item_Archetype::Get_Max_Durability(CVirtualMachine* vm, CVMValue self)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->max_durability;
}

void CRuntime_Item_Archetype::Set_Max_Durability(CVirtualMachine* vm, CVMValue self, float value)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->max_durability = value;
}


float CRuntime_Item_Archetype::Get_Initial_Armour_Amount(CVirtualMachine* vm, CVMValue self)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->initial_armour_amount;
}

void CRuntime_Item_Archetype::Set_Initial_Armour_Amount(CVirtualMachine* vm, CVMValue self, float value)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->initial_armour_amount = value;
}

int CRuntime_Item_Archetype::Get_Max_Stack(CVirtualMachine* vm, CVMValue self)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->max_stack;
}

void CRuntime_Item_Archetype::Set_Max_Stack(CVirtualMachine* vm, CVMValue self, int value)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->max_stack = value;
}

int CRuntime_Item_Archetype::Get_Rarity(CVirtualMachine* vm, CVMValue self)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->rarity;
}

void CRuntime_Item_Archetype::Set_Rarity(CVirtualMachine* vm, CVMValue self, int value)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->rarity = (ItemRarity::Type)value;
}

CVMObjectHandle CRuntime_Item_Archetype::Get_Default_Tint(CVirtualMachine* vm, CVMValue self)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));

	CVMLinkedSymbol* vec4_class = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);

	CVMObjectHandle result = vm->New_Object(vec4_class, false);
	CVMObject* obj = result.Get();
	obj->Resize(4);
	obj->Get_Slot(0).float_value = item->default_tint.R;
	obj->Get_Slot(1).float_value = item->default_tint.G;
	obj->Get_Slot(2).float_value = item->default_tint.B;
	obj->Get_Slot(3).float_value = item->default_tint.A;

	return result;
}

void CRuntime_Item_Archetype::Set_Default_Tint(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->default_tint.R = (u8)value.Get()->Get_Slot(0).float_value;
	item->default_tint.G = (u8)value.Get()->Get_Slot(1).float_value;
	item->default_tint.B = (u8)value.Get()->Get_Slot(2).float_value;
	item->default_tint.A = (u8)value.Get()->Get_Slot(3).float_value;
}

CVMString CRuntime_Item_Archetype::Get_Icon_Animation(CVirtualMachine* vm, CVMValue self)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->icon_animation_name.c_str();
}

void CRuntime_Item_Archetype::Set_Icon_Animation(CVirtualMachine* vm, CVMValue self, CVMString value)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->icon_animation_name = value.C_Str();
}

CVMString CRuntime_Item_Archetype::Get_Body_Animation(CVirtualMachine* vm, CVMValue self)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->body_animation_name.c_str();
}

void CRuntime_Item_Archetype::Set_Body_Animation(CVirtualMachine* vm, CVMValue self, CVMString value)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->body_animation_name = value.C_Str();
}

CVMObjectHandle CRuntime_Item_Archetype::Get_Icon_Offset(CVirtualMachine* vm, CVMValue self)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));

	CVMLinkedSymbol* vec2_class = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);

	CVMObjectHandle result = vm->New_Object(vec2_class, false);
	CVMObject* obj = result.Get();
	obj->Resize(2);
	obj->Get_Slot(0).float_value = item->icon_offset.X;
	obj->Get_Slot(1).float_value = item->icon_offset.Y;

	return result;
}

void CRuntime_Item_Archetype::Set_Icon_Offset(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->icon_offset.X = value.Get()->Get_Slot(0).float_value;
	item->icon_offset.Y = value.Get()->Get_Slot(1).float_value;
}

int CRuntime_Item_Archetype::Get_Is_Icon_Direction_Based(CVirtualMachine* vm, CVMValue self)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->is_icon_direction_based ? 1 : 0;
}

void CRuntime_Item_Archetype::Set_Is_Icon_Direction_Based(CVirtualMachine* vm, CVMValue self, int value)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->is_icon_direction_based = (value != 0);
}

int CRuntime_Item_Archetype::Get_Is_Icon_Overlay(CVirtualMachine* vm, CVMValue self)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->is_icon_overlay ? 1 : 0;
}

void CRuntime_Item_Archetype::Set_Is_Icon_Overlay(CVirtualMachine* vm, CVMValue self, int value)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->is_icon_overlay = (value != 0);
}

CVMString CRuntime_Item_Archetype::Get_Combine_Group(CVirtualMachine* vm, CVMValue self)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->combine_group.c_str();
}

void CRuntime_Item_Archetype::Set_Combine_Group(CVirtualMachine* vm, CVMValue self, CVMString value)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->combine_group = value.C_Str();
}

CVMString CRuntime_Item_Archetype::Get_Combine_Name(CVirtualMachine* vm, CVMValue self)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->combine_name.c_str();
}

void CRuntime_Item_Archetype::Set_Combine_Name(CVirtualMachine* vm, CVMValue self, CVMString value)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->combine_name = value.C_Str();
}


CVMObjectHandle CRuntime_Item_Archetype::Get_Combinations(CVirtualMachine* vm, CVMValue self)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->combinations_array;
}

void CRuntime_Item_Archetype::Set_Combinations(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->combinations_array = value;
}

CVMObjectHandle CRuntime_Item_Archetype::Find_By_Name(CVirtualMachine* vm, CVMString name)
{
	CVMLinkedSymbol* archetype_class = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);
	
	ItemArchetype* item = ItemManager::Get()->Find_Archetype(name.C_Str());

	CVMObjectHandle result = vm->New_Object(archetype_class, false, item);

	return result;
}

CVMObjectHandle CRuntime_Item_Archetype::Find_By_Type(CVirtualMachine* vm, CVMObjectHandle name)
{
	CVMLinkedSymbol* class_symbol = reinterpret_cast<CVMLinkedSymbol*>(name.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));	

	ItemArchetype* item = ItemManager::Get()->Find_Archetype(class_symbol);
	vm->Assert(item != NULL);

	return item->script_object;
}

CVMObjectHandle CRuntime_Item_Archetype::Get_Weapon_Type(CVirtualMachine* vm, CVMValue self)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	if (item->weapon_type == NULL)
	{
		return NULL;
	}
	return item->weapon_type->type_instance;
}

void CRuntime_Item_Archetype::Set_Weapon_Type(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	CVMLinkedSymbol* type_class = reinterpret_cast<CVMLinkedSymbol*>(value.Get()->Get_Meta_Data());	
	item->weapon_type = type_class;
}
	
CVMObjectHandle CRuntime_Item_Archetype::Get_Ammo_Type(CVirtualMachine* vm, CVMValue self)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	if (item->ammo_type == NULL)
	{
		return NULL;
	}
	return item->ammo_type->type_instance;
}

void CRuntime_Item_Archetype::Set_Ammo_Type(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	CVMLinkedSymbol* type_class = reinterpret_cast<CVMLinkedSymbol*>(value.Get()->Get_Meta_Data());	
	item->ammo_type = type_class;
}

CVMObjectHandle CRuntime_Item_Archetype::Get_Post_Process_FX_Type(CVirtualMachine* vm, CVMValue self)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	if (item->post_process_fx_type == NULL)
	{
		return NULL;
	}
	return item->post_process_fx_type->type_instance;
}

void CRuntime_Item_Archetype::Set_Post_Process_FX_Type(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	CVMLinkedSymbol* type_class = reinterpret_cast<CVMLinkedSymbol*>(value.Get()->Get_Meta_Data());	
	item->post_process_fx_type = type_class;
}

CVMObjectHandle CRuntime_Item_Archetype::Get_Upgrade_Tree(CVirtualMachine* vm, CVMValue self)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->upgrade_tree;
}

void CRuntime_Item_Archetype::Set_Upgrade_Tree(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->upgrade_tree = value;
}

int CRuntime_Item_Archetype::Get_Is_Premium_Only(CVirtualMachine* vm, CVMValue self)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->is_premium_only ? 1 : 0;
}

void CRuntime_Item_Archetype::Set_Is_Premium_Only(CVirtualMachine* vm, CVMValue self, int value)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->is_premium_only = value != 0;
}

CVMString CRuntime_Item_Archetype::Get_Premium_Price(CVirtualMachine* vm, CVMValue self)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return item->premium_price.c_str();
}

void CRuntime_Item_Archetype::Set_Premium_Price(CVirtualMachine* vm, CVMValue self, CVMString value)
{
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	item->premium_price = value.C_Str();
}

void CRuntime_Item_Archetype::Bind(CVirtualMachine* vm)
{
	vm->Get_Bindings()->Bind_Method<CVMString>("Item_Archetype", "Get_Varient_Prefix", &Get_Varient_Prefix);
	vm->Get_Bindings()->Bind_Method<void, CVMString>("Item_Archetype", "Set_Varient_Prefix", &Set_Varient_Prefix);

	vm->Get_Bindings()->Bind_Method<CVMString>("Item_Archetype", "Get_Name", &Get_Name);
	vm->Get_Bindings()->Bind_Method<void,CVMString>("Item_Archetype", "Set_Name", &Set_Name);
	vm->Get_Bindings()->Bind_Method<CVMString>("Item_Archetype", "Get_Description", &Get_Description);
	vm->Get_Bindings()->Bind_Method<void,CVMString>("Item_Archetype", "Set_Description", &Set_Description);
	vm->Get_Bindings()->Bind_Method<CVMString>("Item_Archetype", "Get_Category", &Get_Category);
	vm->Get_Bindings()->Bind_Method<void,CVMString>("Item_Archetype", "Set_Category", &Set_Category);
	vm->Get_Bindings()->Bind_Method<int>("Item_Archetype", "Get_Slot", &Get_Slot);
	vm->Get_Bindings()->Bind_Method<void,int>("Item_Archetype", "Set_Slot", &Set_Slot);
	vm->Get_Bindings()->Bind_Method<int>("Item_Archetype", "Get_Cost", &Get_Cost);
	vm->Get_Bindings()->Bind_Method<void,int>("Item_Archetype", "Set_Cost", &Set_Cost);
	vm->Get_Bindings()->Bind_Method<int>("Item_Archetype", "Get_Is_Unpackable", &Get_Is_Unpackable);
	vm->Get_Bindings()->Bind_Method<void, int>("Item_Archetype", "Set_Is_Unpackable", &Set_Is_Unpackable);
	
	vm->Get_Bindings()->Bind_Method<int>("Item_Archetype", "Get_Is_Stackable", &Get_Is_Stackable);
	vm->Get_Bindings()->Bind_Method<void,int>("Item_Archetype", "Set_Is_Stackable", &Set_Is_Stackable);

	vm->Get_Bindings()->Bind_Method<int>("Item_Archetype", "Get_Use_Custom_Color", &Get_Use_Custom_Color);
	vm->Get_Bindings()->Bind_Method<void, int>("Item_Archetype", "Set_Use_Custom_Color", &Set_Use_Custom_Color);


	vm->Get_Bindings()->Bind_Method<int>("Item_Archetype", "Get_Unlock_Criteria", &Get_Unlock_Criteria);
	vm->Get_Bindings()->Bind_Method<void, int>("Item_Archetype", "Set_Unlock_Criteria", &Set_Unlock_Criteria);
	vm->Get_Bindings()->Bind_Method<int>("Item_Archetype", "Get_Unlock_Criteria_Threshold", &Get_Unlock_Criteria_Threshold);
	vm->Get_Bindings()->Bind_Method<void, int>("Item_Archetype", "Set_Unlock_Criteria_Threshold", &Set_Unlock_Criteria_Threshold);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>("Item_Archetype", "Get_Unlock_Criteria_Item", &Get_Unlock_Criteria_Item);
	vm->Get_Bindings()->Bind_Method<void, CVMObjectHandle>("Item_Archetype", "Set_Unlock_Criteria_Item", &Set_Unlock_Criteria_Item);

	vm->Get_Bindings()->Bind_Method<int>("Item_Archetype", "Get_Is_Buyable", &Get_Is_Buyable);
	vm->Get_Bindings()->Bind_Method<void,int>("Item_Archetype", "Set_Is_Buyable", &Set_Is_Buyable);
	vm->Get_Bindings()->Bind_Method<int>("Item_Archetype", "Get_Is_Tintable", &Get_Is_Tintable);
	vm->Get_Bindings()->Bind_Method<void,int>("Item_Archetype", "Set_Is_Tintable", &Set_Is_Tintable);
	vm->Get_Bindings()->Bind_Method<int>("Item_Archetype", "Get_Is_Sellable", &Get_Is_Sellable);
	vm->Get_Bindings()->Bind_Method<void,int>("Item_Archetype", "Set_Is_Sellable", &Set_Is_Sellable);
	vm->Get_Bindings()->Bind_Method<int>("Item_Archetype", "Get_Is_Tradable", &Get_Is_Tradable);
	vm->Get_Bindings()->Bind_Method<void,int>("Item_Archetype", "Set_Is_Tradable", &Set_Is_Tradable);
	vm->Get_Bindings()->Bind_Method<int>("Item_Archetype", "Get_Is_Dropable", &Get_Is_Dropable);
	vm->Get_Bindings()->Bind_Method<void,int>("Item_Archetype", "Set_Is_Dropable", &Set_Is_Dropable);
	vm->Get_Bindings()->Bind_Method<int>("Item_Archetype", "Get_Is_Inventory_Droppable", &Get_Is_Inventory_Dropable);
	vm->Get_Bindings()->Bind_Method<void,int>("Item_Archetype", "Set_Is_Inventory_Droppable", &Set_Is_Inventory_Dropable);
	vm->Get_Bindings()->Bind_Method<int>("Item_Archetype", "Get_Is_PVP_Usable", &Get_Is_PVP_Usable);
	vm->Get_Bindings()->Bind_Method<void,int>("Item_Archetype", "Set_Is_PVP_Usable", &Set_Is_PVP_Usable);
	vm->Get_Bindings()->Bind_Method<int>("Item_Archetype", "Get_Max_Stack", &Get_Max_Stack);
	vm->Get_Bindings()->Bind_Method<void,int>("Item_Archetype", "Set_Max_Stack", &Set_Max_Stack);
	vm->Get_Bindings()->Bind_Method<int>("Item_Archetype", "Get_Rarity", &Get_Rarity);
	vm->Get_Bindings()->Bind_Method<void,int>("Item_Archetype", "Set_Rarity", &Set_Rarity);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>("Item_Archetype", "Get_Default_Tint", &Get_Default_Tint);
	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle>("Item_Archetype", "Set_Default_Tint", &Set_Default_Tint);
	vm->Get_Bindings()->Bind_Method<CVMString>("Item_Archetype", "Get_Icon_Animation", &Get_Icon_Animation);
	vm->Get_Bindings()->Bind_Method<void,CVMString>("Item_Archetype", "Set_Icon_Animation", &Set_Icon_Animation);
	vm->Get_Bindings()->Bind_Method<CVMString>("Item_Archetype", "Get_Body_Animation", &Get_Body_Animation);
	vm->Get_Bindings()->Bind_Method<void,CVMString>("Item_Archetype", "Set_Body_Animation", &Set_Body_Animation);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>("Item_Archetype", "Get_Icon_Offset", &Get_Icon_Offset);
	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle>("Item_Archetype", "Set_Icon_Offset", &Set_Icon_Offset);
	vm->Get_Bindings()->Bind_Method<int>("Item_Archetype", "Get_Is_Icon_Direction_Based", &Get_Is_Icon_Direction_Based);
	vm->Get_Bindings()->Bind_Method<void,int>("Item_Archetype", "Set_Is_Icon_Direction_Based", &Set_Is_Icon_Direction_Based);
	vm->Get_Bindings()->Bind_Method<int>("Item_Archetype", "Get_Is_Icon_Overlay", &Get_Is_Icon_Overlay);
	vm->Get_Bindings()->Bind_Method<void,int>("Item_Archetype", "Set_Is_Icon_Overlay", &Set_Is_Icon_Overlay);
	vm->Get_Bindings()->Bind_Method<CVMString>("Item_Archetype", "Get_Combine_Group", &Get_Combine_Group);
	vm->Get_Bindings()->Bind_Method<void,CVMString>("Item_Archetype", "Set_Combine_Group", &Set_Combine_Group);
	vm->Get_Bindings()->Bind_Method<CVMString>("Item_Archetype", "Get_Combine_Name", &Get_Combine_Name);
	vm->Get_Bindings()->Bind_Method<void,CVMString>("Item_Archetype", "Set_Combine_Name", &Set_Combine_Name);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>("Item_Archetype", "Get_Combinations", &Get_Combinations);
	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle>("Item_Archetype", "Set_Combinations", &Set_Combinations);
	vm->Get_Bindings()->Bind_Function<CVMObjectHandle,CVMString>("Item_Archetype", "Find_By_Name", &Find_By_Name);
	vm->Get_Bindings()->Bind_Function<CVMObjectHandle,CVMObjectHandle>("Item_Archetype", "Find_By_Type", &Find_By_Type);

	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>("Item_Archetype", "Get_Weapon_Type", &Get_Weapon_Type);
	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle>("Item_Archetype", "Set_Weapon_Type", &Set_Weapon_Type);

	vm->Get_Bindings()->Bind_Method<float>("Item_Archetype", "Get_Initial_Armour_Amount", &Get_Initial_Armour_Amount);
	vm->Get_Bindings()->Bind_Method<void, float>("Item_Archetype", "Set_Initial_Armour_Amount", &Set_Initial_Armour_Amount);

	vm->Get_Bindings()->Bind_Method<float>("Item_Archetype", "Get_Max_Durability", &Get_Max_Durability);
	vm->Get_Bindings()->Bind_Method<void, float>("Item_Archetype", "Set_Max_Durability", &Set_Max_Durability);

	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>("Item_Archetype", "Get_Ammo_Type", &Get_Ammo_Type);
	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle>("Item_Archetype", "Set_Ammo_Type", &Set_Ammo_Type);

	vm->Get_Bindings()->Bind_Method<int>("Item_Archetype", "Get_Is_Premium_Only", &Get_Is_Premium_Only);
	vm->Get_Bindings()->Bind_Method<void, int>("Item_Archetype", "Set_Is_Premium_Only", &Set_Is_Premium_Only);
	vm->Get_Bindings()->Bind_Method<CVMString>("Item_Archetype", "Get_Premium_Price", &Get_Premium_Price);
	vm->Get_Bindings()->Bind_Method<void, CVMString>("Item_Archetype", "Set_Premium_Price", &Set_Premium_Price);

	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>("Item_Archetype", "Get_Post_Process_FX_Type", &Get_Post_Process_FX_Type);
	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle>("Item_Archetype", "Set_Post_Process_FX_Type", &Set_Post_Process_FX_Type);

	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>("Item_Archetype", "Get_Upgrade_Tree", &Get_Upgrade_Tree);
	vm->Get_Bindings()->Bind_Method<void, CVMObjectHandle>("Item_Archetype", "Set_Upgrade_Tree", &Set_Upgrade_Tree);

	vm->Get_Bindings()->Bind_Method<CVMString>("Item_Archetype", "Get_Override_Inventory_ID_Name", &Get_Override_Inventory_ID_Name);
	vm->Get_Bindings()->Bind_Method<void, CVMString>("Item_Archetype", "Set_Override_Inventory_ID_Name", &Set_Override_Inventory_ID_Name);
}
