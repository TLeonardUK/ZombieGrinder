/* *****************************************************************

		CVMObject.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#include "XScript/VirtualMachine/CVMObject.h"
#include "XScript/VirtualMachine/CVMValue.h"
#include "XScript/VirtualMachine/CVMBinary.h"
#include "XScript/VirtualMachine/CVirtualMachine.h"

#include "Engine/IO/Stream.h"

#include "Generic/Threads/Atomic.h"

#include "Generic/Math/Math.h"

#include "Generic/Helper/PersistentLogHelper.h"

#include <cmath>
#include <cstdlib>

//#define DUMP_SERIALIZE_LOGS

#ifndef GC_USE_ARRAY_GLOBAL_LISTS
CVMObject* CVMObject::m_global_list = NULL;
int CVMObject::m_global_list_size = 0;
#else
CVMObject** CVMObject::m_collectable_list = NULL;
int CVMObject::m_collectable_list_size = 0;
int CVMObject::m_collectable_list_capacity = 0;
CVMObject** CVMObject::m_uncollectable_list = NULL;
int CVMObject::m_uncollectable_list_size = 0;
int CVMObject::m_uncollectable_list_capacity = 0;
#endif

#ifndef GC_USE_ARRAY_GLOBAL_LISTS
void CVMObject::Remove_Link()
{
	if (m_prev != NULL)
		m_prev->m_next = m_next;
	if (m_next != NULL)
		m_next->m_prev = m_prev;

	if (this == m_global_list)
		m_global_list = m_next;

	m_global_list_size--;
}

void CVMObject::Add_Link()
{
	DBG_ASSERT(m_prev == NULL);
	DBG_ASSERT(m_next == NULL);
	DBG_ASSERT(!m_in_list);
	m_in_list = true;

	if (m_global_list != NULL)
		m_global_list->m_prev = this;

	m_next = m_global_list;
	m_global_list = this;

	m_global_list_size++;
}
#else

void CVMObject::Remove_Collectable_Link()
{
	m_collectable_list[m_index] = m_collectable_list[m_collectable_list_size - 1];
	m_collectable_list[m_index]->m_index = m_index;
	m_collectable_list_size--;
	m_index = 0xFEDEFEDE;
	m_in_list = false;
}

void CVMObject::Add_Collectable_Link()
{
	if (m_collectable_list_size >= m_collectable_list_capacity)
	{
		int new_capacity = m_collectable_list_capacity * 2;

		if (new_capacity == 0)
		{
			new_capacity = 1024;
		}

		CVMObject** new_list = new CVMObject*[new_capacity];
		if (m_collectable_list != NULL)
		{
			memcpy(new_list, m_collectable_list, m_collectable_list_size * sizeof(CVMObject*));
		}
		m_collectable_list = new_list;
		m_collectable_list_capacity = new_capacity;
	}

	m_index = m_collectable_list_size;
	m_collectable_list_size++;

	m_collectable_list[m_index] = this;
	m_in_list = true;
}

void CVMObject::Remove_Uncollectable_Link()
{
	m_uncollectable_list[m_index] = m_uncollectable_list[m_uncollectable_list_size - 1];
	m_uncollectable_list[m_index]->m_index = m_index;
	m_uncollectable_list_size--;
	m_index = 0xFEDEFEDE;
	m_in_list = false;
}

void CVMObject::Add_Uncollectable_Link()
{
	if (m_uncollectable_list_size >= m_uncollectable_list_capacity)
	{
		int new_capacity = m_uncollectable_list_capacity * 2;

		if (new_capacity == 0)
		{
			new_capacity = 1024;
		}

		CVMObject** new_list = new CVMObject*[new_capacity];
		if (m_uncollectable_list != NULL)
		{
			memcpy(new_list, m_uncollectable_list, m_uncollectable_list_size * sizeof(CVMObject*));
		}
		m_uncollectable_list = new_list;
		m_uncollectable_list_capacity = new_capacity;
	}

	m_index = m_uncollectable_list_size;
	m_uncollectable_list_size++;

	m_uncollectable_list[m_index] = this;
	m_in_list = true;
}

#endif

void CVMObject::Mark_Collectable()
{
	m_uncollectable--;
	if (m_uncollectable == 0)
	{
		Remove_Uncollectable_Link();
		Add_Collectable_Link();
	}
	DBG_ASSERT(m_uncollectable >= 0);
}

void CVMObject::Mark_Uncollectable()
{
	if (m_uncollectable == 0)
	{
		Remove_Collectable_Link();
		Add_Uncollectable_Link();
		m_gc_mark = true;
	}
	m_uncollectable++;
}

CVMObject::~CVMObject()
{
	if (m_data != NULL)
	{
		m_vm->Get_GC()->Free_Unmanaged<CVMValue>(m_data);
	}

	if (m_uncollectable)
	{
		Remove_Uncollectable_Link();
	}
	else
	{
		Remove_Collectable_Link();
	}
}

CVMObject::CVMObject(CVirtualMachine* vm)
	: m_data(NULL)
	, m_data_size(0)
	, m_capacity(0)
	, m_class(NULL)
	, m_vm(vm)
	, m_meta_data(NULL)
	, m_state_change_counter(0)
	, m_state_stack_index(0)
#ifndef GC_USE_ARRAY_GLOBAL_LISTS
	, m_next(NULL)
	, m_prev(NULL)
#endif
	, m_in_list(false)
	, m_gc_mark(false)
	, m_uncollectable(0)
{
	Add_Collectable_Link();
}

void* CVMObject::Get_Meta_Data_NullCheck(CVirtualMachine* vm)
{
	vm->Assert(m_meta_data != NULL, "Attempt to access native object thats null (probably despawned).");
	return m_meta_data;
}

CVMLinkedSymbol* CVMObject::Current_State()
{
	if (m_state_stack_index > 0)
	{
		return m_state_stack[m_state_stack_index - 1];
	}
	return NULL;
}

void CVMObject::Push_State(CVMLinkedSymbol* symbol)
{
	m_vm->Assert(m_state_stack_index < max_state_stack);
	m_state_stack[m_state_stack_index] = symbol;
	m_state_stack_index++;
	m_state_change_counter++;
}

void CVMObject::Pop_State()
{
	m_state_stack_index--;
	m_state_change_counter++;
}

CVMLinkedSymbol* CVMObject::Get_Symbol()
{
	return m_class;
}

CVMValue& CVMObject::Get_Slot(int index)
{
	m_vm->Assert(index >= 0 && index < m_data_size, "Attempt to get invalid slot (%i / %i).", index, m_data_size);
	return m_data[index];
}

int CVMObject::Slot_Count()
{
	return m_data_size;
}

void CVMObject::Init_Object(CVMLinkedSymbol* sym)
{
	m_class = sym;
	Resize(sym->symbol->class_data->field_count);
	//Clear_Data();
}

void CVMObject::Init_Array(CVMLinkedSymbol* sym, int size)
{
	m_class = sym;
	Resize(size);
	//Clear_Data();
}

void CVMObject::Clear_Data()
{
	for (int i = 0; i < m_data_size; i++)
	{
		m_data[i].Clear();
	}	
}

void CVMObject::Resize(int size)
{
	m_vm->Assert(size >= 0);
	if (size == m_data_size)
	{
		return;
	}

	if (size < m_capacity)
	{
		m_data_size = size;
		for (int i = size; i < m_capacity; i++)
		{			
			CVMValue& value = m_data[i];
			value.Clear();
		}

		return;
	}

	//int new_capacity = 1;
	//while (new_capacity < size)
	//{
	//	new_capacity *= 2;
	//}

	// Note: Tight resizing seems to be more efficient, less clearing to do per-alloc.
	int new_capacity = size;
	
	CVMValue* new_data = m_vm->Get_GC()->Alloc_Unmanaged<CVMValue>(new_capacity);
	for (int i = 0; i < new_capacity; i++)
	{
		CVMValue& value = new_data[i];
		if (i < m_data_size)
		{
			value = m_data[i];
		}

		// TODO: Not required! Alread cleared when alloced.
		//else
		//{
		//	value.Clear();
		//}
	}

	if (m_data != NULL)
	{
		m_vm->Get_GC()->Free_Unmanaged<CVMValue>(m_data);
	}

	m_data = new_data;
	m_data_size = size;
	m_capacity = new_capacity;
}

CVMObject* CVMObject::Slice(int start)
{
	return Slice(start, m_data_size);
}

CVMObject* CVMObject::Slice(int start_index, int end_index)
{
	int length = m_data_size;

	if (start_index < 0)
	{
		start_index += length;
		if (start_index < 0)
		{
			start_index = 0;
		}		
	}
	else if (start_index > length)
	{
		start_index = length;
	}
	
	if (end_index < 0)
	{
		end_index += length;
	}
	else if (end_index > length)
	{
		end_index = length;
	}

	if (start_index >= end_index)
	{
		CVMObject* obj = new(m_vm->Get_GC()->Alloc(sizeof(CVMObject))) CVMObject(m_vm); 
		obj->Init_Array(m_class, 0);
		return obj;
	}
	else if (start_index == 0 && end_index == length)
	{
		return Clone();
	}
	else
	{
		int new_size = end_index - start_index;

		CVMObject* obj = new(m_vm->Get_GC()->Alloc(sizeof(CVMObject))) CVMObject(m_vm); 
		obj->Init_Array(m_class, new_size);

		for (int i = start_index; i < start_index + new_size; i++)
		{
			obj->m_data[i - start_index] = m_data[i];
		}

		return obj;
	}

	return NULL;
}

void CVMObject::Shift(int offset)
{
	// If we are shifting left start from the right side, otherwise
	// start from the left side. Prevents overwriting.

	offset = Sgn(offset) * (abs(offset) % m_data_size);

	for (int i = 0; i < m_data_size; i++)
	{
		int dst_slot = i + offset;
		
		// Wrap index around.
		if (dst_slot < 0)
		{
			dst_slot = m_data_size + dst_slot;
		}
		else if (dst_slot >= m_data_size)
		{
			dst_slot = dst_slot % m_data_size;
		}

		m_data[i] = m_data[dst_slot];
	}
}

void CVMObject::Add_First(CVMValue value)
{
	Resize(m_data_size + 1);
	Shift(1);
	m_data[0] = value;
}

void CVMObject::Add_Array_First(CVMObject* value)
{
	int slot_count = value->Slot_Count();
	Resize(m_data_size + slot_count);
	Shift(slot_count);
	
	for (int i = 0; i < slot_count; i++)
	{
		m_data[i] = value->Get_Slot(i);
	}
}

void CVMObject::Add_Last(CVMValue value)
{
	Resize(m_data_size + 1);
	m_data[m_data_size - 1] = value;
}

void CVMObject::Add_Array_Last(CVMObject* value)
{
	int slot_count = value->Slot_Count();
	Resize(m_data_size + slot_count);
	for (int i = 0; i < slot_count; i++)
	{
		m_data[m_data_size - slot_count + i] = value->Get_Slot(i);
	}
}

void CVMObject::Clear()
{
	for (int i = 0; i < m_data_size; i++)
	{
		m_data[i].Clear();
	}
	Resize(0);
}

CVMObject* CVMObject::Clone()
{
	CVMObject* obj = new(m_vm->Get_GC()->Alloc(sizeof(CVMObject))) CVMObject(m_vm); 
	obj->Init_Array(m_class, m_data_size);
	
	for (int i = 0; i < m_data_size; i++)
	{
		obj->m_data[i] = m_data[i];
	}

	return obj;
}

bool CVMObject::Contains(CVMValue needle)
{
	for (int i = 0; i < m_data_size; i++)
	{
		if (m_data[i].Equal_To(needle, m_class->symbol->class_data->data_type->element_type))
		{
			return true;
		}
	}
	return false;
}

void CVMObject::Replace(CVMValue what, CVMValue to)
{
	for (int i = 0; i < m_data_size; i++)
	{
		if (m_data[i].Equal_To(what, m_class->symbol->class_data->data_type->element_type))
		{
			m_data[i] = to;
		}
	}
}

void CVMObject::Reverse()
{
	int half_size = (int)floorf(m_data_size / 2.0f);
	for (int i = 0; i < half_size; i++)
	{
		m_data[i] = m_data[m_data_size - (i + 1)];
	}
}

CVMValue CVMObject::Remove_Index(int index)
{
	CVMValue original = m_data[index];
	for (int i = index; i < m_data_size - 1; i++)
	{
		m_data[i] = m_data[i + 1];
	}
	Resize(m_data_size - 1);
	return original;
}

void CVMObject::Remove(CVMValue needle)
{
	for (int i = 0; i < m_data_size; i++)
	{
		if (m_data[i].Equal_To(needle, m_class->symbol->class_data->data_type->element_type))
		{
			Remove_Index(i);
			i--;
			continue;
		}
	}
}

CVMValue CVMObject::Remove_First()
{
	CVMValue result = m_data[0];
	Shift(-1);
	Resize(m_data_size - 1);
	return result;
}

CVMValue CVMObject::Remove_Last()
{
	CVMValue result = m_data[m_data_size - 1];
	Resize(m_data_size - 1);
	return result;
}

void CVMObject::Insert(CVMValue value, int index)
{
	Resize(m_data_size + 1);
	for (int i = m_data_size - 2; i >= index; i--)
	{
		m_data[i + 1] = m_data[i];
	}
	m_data[index] = value;
}

void CVMObject::Trim_Start(CVMValue needle)
{
	int trim_start = 0; 

	for (trim_start = 0; trim_start < m_data_size; trim_start++)
	{
		if (!m_data[trim_start].Equal_To(needle, m_class->symbol->class_data->data_type->element_type))
		{
			break;
		}
	}

	if (trim_start > 0)
	{
		Shift(-trim_start);
		Resize(m_data_size - trim_start);
	}
}

void CVMObject::Trim_End(CVMValue needle)
{
	int trim_start = 0; 

	for (int i = m_data_size - 1; i > 0; i--, trim_start++)
	{
		if (!m_data[trim_start].Equal_To(needle, m_class->symbol->class_data->data_type->element_type))
		{
			break;
		}
	}

	if (trim_start > 0)
	{
		Resize(m_data_size - trim_start);
	}
}

void CVMObject::Trim(CVMValue needle)
{
	Trim_Start(needle);
	Trim_End(needle);
}

void CVMObject::Pad_Left(int length, CVMValue pad_value)
{
	int padding = length - m_data_size;
	if (padding <= 0)
	{
		return;
	}
	Resize(m_data_size + padding);
	Shift(padding);
	for (int i = 0; i < padding; i++)
	{
		m_data[i] = pad_value;
	}
}

void CVMObject::Pad_Right(int length, CVMValue pad_value)
{
	int padding = length - m_data_size;
	if (padding <= 0)
	{
		return;
	}
	Resize(m_data_size + padding);
	for (int i = m_data_size - padding; i < m_data_size; i++)
	{
		m_data[i] = pad_value;
	}
}

int CVMObject::Index_Of(CVMValue needle, int start)
{
	for (int i = start; i < m_data_size; i++)
	{
		if (m_data[i].Equal_To(needle, m_class->symbol->class_data->data_type->element_type))
		{
			return i;
		}
	}
	return -1;
}

int CVMObject::Index_Of_Any(CVMValue needle, int start)
{
	CVMObject* other = needle.object_value.Get();
	for (int i = 0; i < m_data_size; i++)
	{
		for (int n = 0; n < other->Slot_Count(); n++)
		{
			if (m_data[i].Equal_To(other->Get_Slot(n), m_class->symbol->class_data->data_type->element_type))
			{
				return i;
			}
		}
	}
	return -1;
}

int CVMObject::Last_Index_Of(CVMValue needle, int start)
{
	for (int i = m_data_size - 1; i >= start; i--)
	{
		if (m_data[i].Equal_To(needle, m_class->symbol->class_data->data_type->element_type))
		{
			return i;
		}
	}
	return -1;
}

int CVMObject::Last_Index_Of_Any(CVMValue needle, int start)
{
	CVMObject* other = needle.object_value.Get();
	for (int i = m_data_size - 1; i >= start; i--)
	{
		for (int n = 0; n < other->Slot_Count(); n++)
		{
			if (m_data[i].Equal_To(other->Get_Slot(n), m_class->symbol->class_data->data_type->element_type))
			{
				return i;
			}
		}
	}
	return -1;
}

void CVMObject::Sort(CVMObject* comparer)
{
	const char* lvalue_name = Get_Symbol()->symbol->class_data->data_type->element_type->name;
	CVMLinkedSymbol* sort_func = m_vm->Find_Function(comparer->Get_Symbol(), "Compare", 2, lvalue_name, lvalue_name);
	m_vm->Assert(sort_func != NULL);

	Quick_Sort(comparer, sort_func, 0, m_data_size - 1);
}

void CVMObject::Quick_Sort(CVMObject* comparer, CVMLinkedSymbol* sort_func, int left, int right)
{
	if (left < right)
	{
		int pivot = left + (right - left) / 2;
		pivot = Quick_Sort_Partition(comparer, sort_func, left, right, pivot);
		Quick_Sort(comparer, sort_func, left, pivot - 1);
		Quick_Sort(comparer, sort_func, pivot + 1, right);
	}
}

int CVMObject::Quick_Sort_Partition(CVMObject* comparer, CVMLinkedSymbol* sort_func, int left, int right, int pivot)
{
	CVMValue pivot_value = m_data[pivot];		
	m_data[pivot] = m_data[right];
	m_data[right] = pivot_value;
		
	int store_index = left;
		
	for (int i = left; i < right; i++)
	{
		CVMValue i_value = m_data[i];
			
		m_vm->Push_Parameter(i_value);
		m_vm->Push_Parameter(pivot_value);
		m_vm->Invoke(sort_func, CVMValue(comparer), false, false);
		
		CVMValue retval;
		m_vm->Get_Return_Value(retval);

		if (retval.int_value <= 0)
		{
			m_data[i] = m_data[store_index];
			m_data[store_index] = i_value;
			store_index++;
		}
	}
		
	CVMValue store_value = m_data[store_index];		
	m_data[store_index] = m_data[right];
	m_data[right] = store_value;
		
	return store_index;
}



CVMObjectHandle::~CVMObjectHandle()
{
}

CVMObjectHandle::CVMObjectHandle()
	: m_object(NULL)
	, m_vf_table_offset(0)
{
}

CVMObjectHandle::CVMObjectHandle(CVMObject* object)
	: m_object(object)
	, m_vf_table_offset(0)
{
}

CVMObjectHandle::CVMObjectHandle(const CVMObjectHandle& other)
{
	m_object = other.m_object;
	m_vf_table_offset = other.m_vf_table_offset;
}

CVMObjectHandle::CVMObjectHandle(CVMObject* object, int vf_table_offset)
	: m_object(object)
	, m_vf_table_offset(vf_table_offset)
{
}

CVMObject* CVMObjectHandle::GetNullCheck(CVirtualMachine* vm)
{
	vm->Assert(m_object != NULL, "Attempted to access null object.");
	return m_object;
}

int CVMObjectHandle::Get_VF_Table_Offset()
{
	return m_vf_table_offset;
}

CVMObjectHandle& CVMObjectHandle::operator =(const CVMObjectHandle& other)
{
	m_object = other.m_object;
	m_vf_table_offset = other.m_vf_table_offset;
	return *this;
}




//bool GObjectDump = false;

void CVMObject::Serialize_Field(Stream* stream, CVMLinkedSymbol* sym, CVMReplicationVar* properties, CVMObjectSerializeFlags::Type flags, int* final_version)
{
#ifdef DUMP_SERIALIZE_LOGS
	if (flags == CVMObjectSerializeFlags::Full)
		DBG_LOG("Serialize_Field(%s)", sym->symbol->name);
#endif

	if (sym->symbol->type == SymbolType::ClassField)
	{
		CVMFieldData* field_data = sym->symbol->field_data;
		Serialize_Value(stream, m_data[field_data->offset], field_data->data_type, flags, final_version);		
	}
	else if (sym->symbol->type == SymbolType::ClassProperty)
	{
		CVMLinkedSymbol* get_method = m_vm->Get_Symbol_Table_Entry(sym->symbol->property_data->get_property_symbol);

		CVMObjectHandle handle(this);
		m_vm->Invoke(get_method, CVMValue(handle), false, false);
		
		CVMValue value;
		m_vm->Get_Return_Value(value);

		Serialize_Value(stream, value, get_method->symbol->method_data->return_data_type, flags, final_version);
	}
	else
	{
		DBG_ASSERT(false);
	}
}

void CVMObject::Serialize_Value(Stream* stream, CVMValue& value, CVMDataType* type, CVMObjectSerializeFlags::Type flags, int* final_version)
{
#ifdef DUMP_SERIALIZE_LOGS
	if (flags == CVMObjectSerializeFlags::Full)
		DBG_LOG("Serialize_Value(%s)", type->name);
#endif

	// TODO: reduce data size based on ranges in script.

	switch (type->type)
	{
	case CVMBaseDataType::Int:
		{
			stream->Write<s32>(value.int_value);
#ifdef DUMP_SERIALIZE_LOGS
			if (flags == CVMObjectSerializeFlags::Full)
				DBG_LOG("\t%i", value.int_value);
#endif
			break;
		}
	case CVMBaseDataType::Bool:
		{
			stream->Write<s8>(value.int_value);
#ifdef DUMP_SERIALIZE_LOGS
			if (flags == CVMObjectSerializeFlags::Full)
				DBG_LOG("\t%i", value.int_value);
#endif
			break;
		}
	case CVMBaseDataType::Float:		
		{
			stream->Write<f32>(value.float_value);
#ifdef DUMP_SERIALIZE_LOGS
			if (flags == CVMObjectSerializeFlags::Full)
				DBG_LOG("\t%f", value.float_value);
#endif
			break;
		}
	case CVMBaseDataType::String:
		{
			int len = value.string_value.Length();
			stream->Write<u16>(len); // 64k should really be enough for a string!
			stream->WriteBuffer(value.string_value.C_Str(), 0, len);
#ifdef DUMP_SERIALIZE_LOGS
			if (flags == CVMObjectSerializeFlags::Full)
				DBG_LOG("\t%s", value.string_value.C_Str());
#endif
			break;
		}
	case CVMBaseDataType::Array:
		{
			CVMObject* obj = value.object_value.Get();
			stream->Write<u8>((obj != NULL));
			if (obj != NULL)
			{
				stream->Write<s32>(obj->m_data_size);
				for (int i = 0; i < obj->m_data_size; i++)
				{
					Serialize_Value(stream, obj->m_data[i], type->element_type, CVMObjectSerializeFlags::Full, final_version);
				}
			}
			break;
		}
	case CVMBaseDataType::Object:
		{
			CVMObject* obj = value.object_value.Get();
			stream->Write<u8>((obj != NULL));
			if (obj != NULL)
			{
				stream->Write<int>(obj->Get_Symbol()->symbol->unique_id);
				obj->Serialize(stream, CVMObjectSerializeFlags::Full, final_version);
			}
			break;
		}
	}
}

void CVMObject::Serialize_Class(Stream* stream, CVMObjectSerializeFlags::Type flags, CVMLinkedSymbol* sym, int* final_version)
{
	// Save base class.
	// UPDATE: Actually no, super-class fields will be in child-array.
//	int super_idx = sym->symbol->class_data->super_class_index;
//	if (super_idx > 0)
//	{
//		CVMLinkedSymbol* super_sym = m_vm->Get_Symbol_Table_Entry(super_idx);
//		Serialize_Class(stream, flags, super_sym, final_version);
//	}

#ifdef DUMP_SERIALIZE_LOGS
	if (flags == CVMObjectSerializeFlags::Full)
		DBG_LOG("Serialize_Class(%s)", sym->symbol->name);
#endif

	if (flags == CVMObjectSerializeFlags::Full)
	{
		for (int i = 0; i < sym->symbol->children_count; i++)
		{
			CVMLinkedSymbol* child = m_vm->Get_Symbol_Table_Entry(sym->symbol->children_indexes[i]);
			if (child->symbol->type == SymbolType::ClassField &&
				child->symbol->field_data->is_serialized != 0)
			{
				if (child->symbol->field_data->serialized_version > *final_version)
				{
					*final_version = child->symbol->field_data->serialized_version;
				}
				Serialize_Field(stream, child, NULL, flags, final_version);
			}
			else if (child->symbol->type == SymbolType::ClassProperty &&
	 				 child->symbol->property_data->is_serialized != 0)
			{
				if (child->symbol->property_data->serialized_version > *final_version)
				{
					*final_version = child->symbol->property_data->serialized_version;
				}
				Serialize_Field(stream, child, NULL, flags, final_version);
			}
		}
	}
	else if (sym->symbol->class_data->is_replicated != 0)
	{
		CVMReplicationInfo* rep_info = sym->symbol->class_data->replication_info;
		for (int i = 0; i < rep_info->variable_count; i++)
		{
			CVMReplicationVar& var = rep_info->variables[i];
			if 
			(
				flags == CVMObjectSerializeFlags::Server_Replicated || // serialize everything as server.
				var.owner == CVMReplicationVarOwner::ObjectOwner ||
				(var.owner == CVMReplicationVarOwner::Client && flags == CVMObjectSerializeFlags::Client_Replicated) // serialize client vars as client.
			)
			{
				if (var.var_symbol_index >= 0)
				{
					CVMLinkedSymbol* field = m_vm->Get_Symbol_Table_Entry(var.var_symbol_index);
					if (field->symbol->field_data->serialized_version > *final_version)
					{
						*final_version = field->symbol->field_data->serialized_version;
					}
					Serialize_Field(stream, field, &var, flags, final_version);
				}
				else
				{
					CVMLinkedSymbol* prop = m_vm->Get_Symbol_Table_Entry(var.prop_symbol_index);
					if (prop->symbol->property_data->serialized_version > *final_version)
					{
						*final_version = prop->symbol->property_data->serialized_version;
					}
					Serialize_Field(stream, prop, &var, flags, final_version);
				}
			}
		}
	}
}

void CVMObject::Serialize(Stream* stream, CVMObjectSerializeFlags::Type flags, int* final_version)
{
	CVMLinkedSymbol* sym = Get_Symbol();
	Serialize_Class(stream, flags, sym, final_version);
}

void CVMObject::Deserialize_Field(Stream* stream, CVMLinkedSymbol* sym, CVMReplicationVar* properties, CVMObjectSerializeFlags::Type flags, int version)
{
#ifdef DUMP_SERIALIZE_LOGS
	if (flags == CVMObjectSerializeFlags::Full)
		DBG_LOG("Deserialize_Field(%s)", sym->symbol->name);
#endif

	if (sym->symbol->type == SymbolType::ClassField)
	{
		CVMFieldData* field_data = sym->symbol->field_data;
		Deserialize_Value(stream, m_data[field_data->offset], field_data->data_type, flags, version);	
	}
	else if (sym->symbol->type == SymbolType::ClassProperty)
	{
		CVMLinkedSymbol* set_method = m_vm->Get_Symbol_Table_Entry(sym->symbol->property_data->set_property_symbol);
		CVMLinkedSymbol* get_method = m_vm->Get_Symbol_Table_Entry(sym->symbol->property_data->get_property_symbol);
	
		CVMObjectHandle handle(this);
		m_vm->Invoke(get_method, CVMValue(handle), false, false);

		CVMValue value;
		m_vm->Get_Return_Value(value);

		Deserialize_Value(stream, value, sym->symbol->property_data->data_type, flags, version);

		m_vm->Push_Parameter(value);
		m_vm->Invoke(set_method, CVMValue(handle), false, false);
	}
	else
	{
		DBG_ASSERT(false);
	}
}

void CVMObject::Deserialize_Value(Stream* stream, CVMValue& value, CVMDataType* type, CVMObjectSerializeFlags::Type flags, int version)
{
#ifdef DUMP_SERIALIZE_LOGS
	if (flags == CVMObjectSerializeFlags::Full)
		DBG_LOG("Deserialize_Value(%s)", type->name);
#endif
	// TODO: reduce data size based on ranges in script.

	switch (type->type)
	{
	case CVMBaseDataType::Int:
		{
			value.int_value = stream->Read<s32>();
#ifdef DUMP_SERIALIZE_LOGS
			if (flags == CVMObjectSerializeFlags::Full)
				DBG_LOG("\t%i", value.int_value);
#endif
			break;
		}
	case CVMBaseDataType::Bool:
		{
			value.int_value = stream->Read<s8>();
#ifdef DUMP_SERIALIZE_LOGS
			if (flags == CVMObjectSerializeFlags::Full)
				DBG_LOG("\t%i", value.int_value);
#endif
			break;
		}
	case CVMBaseDataType::Float:		
		{
			value.float_value = stream->Read<f32>();
#ifdef DUMP_SERIALIZE_LOGS
			if (flags == CVMObjectSerializeFlags::Full)
				DBG_LOG("\t%f", value.float_value);
#endif
			break;
		}
	case CVMBaseDataType::String:
		{
			int len = stream->Read<u16>();
			value.string_value = CVMString('\0', len);
			stream->ReadBuffer(value.string_value.Str(), 0, len);
#ifdef DUMP_SERIALIZE_LOGS
			if (flags == CVMObjectSerializeFlags::Full)
				DBG_LOG("\t%s", value.string_value.C_Str());
#endif
			break;
		}
	case CVMBaseDataType::Array:
		{
			bool exists = (stream->Read<u8>() != 0);
			if (exists)
			{
				int elements = stream->Read<s32>();
				
				CVMObjectHandle arr = value.object_value;
				if (arr.Get() == NULL)
				{
					arr = m_vm->New_Array(m_vm->Get_Symbol_Table_Entry(type->class_index), elements);
				}
				else
				{
					arr.Get()->Resize(elements);
				}

#ifdef DUMP_SERIALIZE_LOGS
				if (flags == CVMObjectSerializeFlags::Full)
					DBG_LOG("\tArray of size %i", elements);
#endif

				for (int i = 0; i < elements; i++)
				{
					Deserialize_Value(stream, arr.Get()->m_data[i], type->element_type, CVMObjectSerializeFlags::Full, version);
				}

				value.object_value = arr;
			}
			else
			{
				value.object_value = NULL;
			}
			break;
		}
	case CVMBaseDataType::Object:
		{
			bool exists = (stream->Read<u8>() != 0);
			if (exists)
			{
				int unique_id = stream->Read<int>();

				CVMObjectHandle obj = value.object_value;
				if (obj.Get() == NULL)
				{
					obj = m_vm->New_Object(m_vm->Find_Symbol(unique_id), true);
				}

				obj.Get()->Deserialize(stream, CVMObjectSerializeFlags::Full, version);

				value.object_value = obj;
			}
			else
			{
				value.object_value = NULL;
			}
			break;
		}
	default:
		{
			DBG_ASSERT(false);
		}
	}
}

void CVMObject::Deserialize_Class(Stream* stream, CVMObjectSerializeFlags::Type flags, CVMLinkedSymbol* sym, int version)
{
	// Deserialize base class.
	// UPDATE: Actually no, super-class fields will be in child-array.
	/*int super_idx = sym->symbol->class_data->super_class_index;
	if (super_idx > 0)
	{
		CVMLinkedSymbol* super_sym = m_vm->Get_Symbol_Table_Entry(super_idx);
		Deserialize_Class(stream, flags, super_sym, version);
	}
	*/

#ifdef DUMP_SERIALIZE_LOGS
	if (flags == CVMObjectSerializeFlags::Full)
		DBG_LOG("Deserialize_Class(%s)", sym->symbol->name);
#endif

	if (flags == CVMObjectSerializeFlags::Full)
	{
		for (int i = 0; i < sym->symbol->children_count; i++)
		{
			CVMLinkedSymbol* child = m_vm->Get_Symbol_Table_Entry(sym->symbol->children_indexes[i]);
			if (child->symbol->type == SymbolType::ClassField &&
				child->symbol->field_data->is_serialized != 0)
			{
				if (child->symbol->field_data->serialized_version <= version)
				{
					Deserialize_Field(stream, child, NULL, flags, version);
				}
			}
			else if (child->symbol->type == SymbolType::ClassProperty &&
	 				 child->symbol->property_data->is_serialized != 0)
			{
				if (child->symbol->property_data->serialized_version <= version)
				{
					Deserialize_Field(stream, child, NULL, flags, version);
				}
			}
		}
	}
	else if (sym->symbol->class_data->is_replicated != 0)
	{
		CVMReplicationInfo* rep_info = sym->symbol->class_data->replication_info;
		for (int i = 0; i < rep_info->variable_count; i++)
		{
			CVMReplicationVar& var = rep_info->variables[i];
			if
			(
				flags == CVMObjectSerializeFlags::Server_Replicated || // serialize everything as server.
				var.owner == CVMReplicationVarOwner::ObjectOwner ||
				(var.owner == CVMReplicationVarOwner::Client && flags == CVMObjectSerializeFlags::Client_Replicated) // serialize client vars as client.
			)
			{
				if (var.var_symbol_index >= 0)
				{
					CVMLinkedSymbol* field = m_vm->Get_Symbol_Table_Entry(var.var_symbol_index);
					if (field->symbol->field_data->serialized_version <= version)
					{
						Deserialize_Field(stream, field, &var, flags, version);
					}
				}
				else
				{
					CVMLinkedSymbol* prop = m_vm->Get_Symbol_Table_Entry(var.prop_symbol_index);
					if (prop->symbol->property_data->serialized_version <= version)
					{
						Deserialize_Field(stream, prop, &var, flags, version);
					}
				}
			}
		}
	}
}

void CVMObject::Deserialize(Stream* stream, CVMObjectSerializeFlags::Type flags, int version)
{
	CVMLinkedSymbol* sym = Get_Symbol();
	Deserialize_Class(stream, flags, sym, version);
}

template <typename T>
T Interpolate_By_Mode(T from, T to, T current, float delta, CVMReplicationVarMode::Type mode)
{
	switch (mode)
	{
	case CVMReplicationVarMode::Absolute:
		{
			if (delta <= 0.5f)
				return from;
			else
				return to;
		}
	case CVMReplicationVarMode::Interpolate_Linear:
		{
			return (T)Math::Lerp((float)from, (float)to, delta);
		}
	case CVMReplicationVarMode::Interpolate_Smoothed:
		{
			// Right this shit is complicated, get ready for some confusion!
			// What we are going to do here is forward-extrapolate the valuve.

			// We do this by calculating the diff for each frame and storing it. 
			// We then use the latest x number of these diffs to calculate a "current heading".

			// We then extrapolate the value as:
			//		remaining_time = (average_recieve_interval - time_since_to_state_recieved);
			//		delta = 1.0f - (remaining_time / average_recieve_interval);
			//		total_distance = velocity * average_recieve_interval;
			//		delta_distance = total_distance * delta;
			//		ideal_extrapolated = to + (current_heading * delta_distance) ;
			//		actual_extrapolated = extrapolated_at_last_to + (current_heading * delta_distance);

			// We then adjust for drift by doing:
			//		extrapolated = Lerp(actual_extrapolated, ideal_extrapolated, 0.25f)

			const float TELEPORT_THRESHOLD = 100.0f;

			// If we have moved far enough assume we are teleporting.
			float dist = (float)fabs(from - to);
			if (dist > TELEPORT_THRESHOLD)
			{
				return to;
			}

			return (T)Math::Lerp((float)from, (float)to, delta);
		//	return (T)Math::SmoothStep((float)from, (float)to, delta);
		//	return (T)Math::Lerp((float)current, (float)to, 0.1f);
		}
	}

	return from;
}

void CVMObject::Interpolate_Value(Stream* from_stream, Stream* to_stream, float delta, CVMValue& value, CVMDataType* type, CVMObjectSerializeFlags::Type flags, int version, bool owner, bool do_not_apply, CVMReplicationVar* default_replication_var)
{
	switch (type->type)
	{
	case CVMBaseDataType::Int:
		{
			s32 from = from_stream->Read<s32>();
			s32 to	 = to_stream->Read<s32>();

			if (!do_not_apply)
				value.int_value = Interpolate_By_Mode<s32>(from, to, value.int_value, delta, (CVMReplicationVarMode::Type)default_replication_var->mode);
	
			break;
		}
	case CVMBaseDataType::Bool:
		{
			s8 from = from_stream->Read<s8>();
			s8 to	= to_stream->Read<s8>();

			if (!do_not_apply)
				value.int_value = Interpolate_By_Mode<s8>(from, to, value.int_value, delta, (CVMReplicationVarMode::Type)default_replication_var->mode);

			break;
		}
	case CVMBaseDataType::Float:		
		{
			f32 from = from_stream->Read<f32>();
			f32 to	 = to_stream->Read<f32>();

			if (!do_not_apply)
				value.float_value = Interpolate_By_Mode<f32>(from, to, value.float_value, delta, (CVMReplicationVarMode::Type)default_replication_var->mode);
	
			break;
		}
	case CVMBaseDataType::String:
		{
			int from_len = from_stream->Read<u16>();
			int to_len = to_stream->Read<u16>();

			CVMString from = CVMString('\0', from_len);
			CVMString to   = CVMString('\0', to_len);

			from_stream->ReadBuffer(from.Str(), 0, from_len);
			to_stream->ReadBuffer(to.Str(), 0, to_len);

			if (!do_not_apply)
			{
				if (delta <= 0.5)
					value.string_value = from;
				else
					value.string_value = to;
			}

			break;
		}
	case CVMBaseDataType::Array:
		{
			bool from_exists = (from_stream->Read<u8>() != 0);
			bool to_exists   = (to_stream->Read<u8>() != 0);
			
			int from_elements = from_stream->Read<s32>();		
			int to_elements = to_stream->Read<s32>();

			DBG_ASSERT_STR(from_exists == true && to_exists == true, "Replicated arrays must be persistent.");
			DBG_ASSERT_STR(from_elements == to_elements, "Replicated arrays must have persistent length.");
				
			CVMValue dummy;
			CVMValue& out = value;
			if (do_not_apply == true)
			{
				out = dummy;
			}

			CVMObjectHandle arr = out.object_value;
			if (arr.Get() == NULL)
			{
				arr = m_vm->New_Array(m_vm->Get_Symbol_Table_Entry(type->class_index), from_elements);
			}
			else
			{
				arr.Get()->Resize(from_elements);
			}

			for (int i = 0; i < from_elements; i++)
			{
				Interpolate_Value(from_stream, to_stream, delta, arr.Get()->m_data[i], type->element_type, CVMObjectSerializeFlags::Full, version, owner, do_not_apply, default_replication_var);
			}

			out.object_value = arr;

			break;
		}
	case CVMBaseDataType::Object:
		{
			bool from_exists = (from_stream->Read<u8>() != 0);
			bool to_exists   = (to_stream->Read<u8>() != 0);

			int from_unique_id = from_exists ? from_stream->Read<int>() : 0;
			int to_unique_id = to_exists ? to_stream->Read<int>() : 0;

			DBG_ASSERT_STR(from_exists == true && to_exists == true, "Replicated objects must be persistent.");
			
			if (!do_not_apply)
			{
				CVMObjectHandle obj = value.object_value;
				if (obj.Get() == NULL)
				{
					obj = m_vm->New_Object(m_vm->Find_Symbol(from_unique_id), true);
				}

				obj.Get()->Interpolate(from_stream, to_stream, delta, CVMObjectSerializeFlags::Full, version, owner, default_replication_var);

				value.object_value = obj;
			}
			else
			{
				CVMObjectHandle obj = value.object_value;
				if (obj.Get() == NULL)
				{
					obj = m_vm->New_Object(m_vm->Find_Symbol(from_unique_id), true);
				}

				obj.Get()->Interpolate(from_stream, to_stream, delta, CVMObjectSerializeFlags::Full, version, owner, default_replication_var);

				value.object_value = obj;
			}

			break;
		}
	default:
		{
			DBG_ASSERT(false);
		}
	}
}

void CVMObject::Interpolate_Field(Stream* from_stream, Stream* to_stream, float delta, CVMLinkedSymbol* sym, CVMReplicationVar* props, CVMObjectSerializeFlags::Type flags, int version, bool owner, bool do_not_apply, CVMReplicationVar* default_replication_var)
{	
	if (sym->symbol->type == SymbolType::ClassField)
	{
		CVMFieldData* field_data = sym->symbol->field_data;
		Interpolate_Value(from_stream, to_stream, delta, m_data[field_data->offset], field_data->data_type, flags, version, owner, do_not_apply, props);	
	}
	else if (sym->symbol->type == SymbolType::ClassProperty)
	{
		CVMLinkedSymbol* set_method = m_vm->Get_Symbol_Table_Entry(sym->symbol->property_data->set_property_symbol);
		CVMLinkedSymbol* get_method = m_vm->Get_Symbol_Table_Entry(sym->symbol->property_data->get_property_symbol);
	
		CVMObjectHandle handle(this);
		m_vm->Invoke(get_method, CVMValue(handle), false, false);

		CVMValue value;
		m_vm->Get_Return_Value(value);

		Interpolate_Value(from_stream, to_stream, delta, value, sym->symbol->property_data->data_type, flags, version, owner, do_not_apply, props);

		//DBG_LOG("Interpolating field %s (do_not_apply=%i)", sym->symbol->name, do_not_apply);

		if (do_not_apply == false)
		{
			m_vm->Push_Parameter(value);
			m_vm->Invoke(set_method, CVMValue(handle), false, false);
		}
	}
	else
	{
		DBG_ASSERT(false);
	}
}

void CVMObject::Interpolate_Class(Stream* from_stream, Stream* to_stream, float delta, CVMObjectSerializeFlags::Type flags, CVMLinkedSymbol* sym, int version, bool owner, CVMReplicationVar* default_replication_var)
{
	// Deserialize base class.
	// UPDATE: Actually no, super-class fields will be in child-array.
	/*
	int super_idx = sym->symbol->class_data->super_class_index;
	if (super_idx > 0)
	{
		CVMLinkedSymbol* super_sym = m_vm->Get_Symbol_Table_Entry(super_idx);
		Interpolate_Class(from_stream, to_stream, delta, flags, super_sym, version, owner, default_replication_var);
	}
	*/

#ifdef DUMP_SERIALIZE_LOGS
	if (flags == CVMObjectSerializeFlags::Full)
		DBG_LOG("Deserialize_Class(%s)", sym->symbol->name);
#endif

	if (flags == CVMObjectSerializeFlags::Full)
	{
		for (int i = 0; i < sym->symbol->children_count; i++)
		{
			CVMLinkedSymbol* child = m_vm->Get_Symbol_Table_Entry(sym->symbol->children_indexes[i]);
			if (child->symbol->type == SymbolType::ClassField &&
				child->symbol->field_data->is_serialized != 0)
			{
				if (child->symbol->field_data->serialized_version <= version)
				{
					Interpolate_Field(from_stream, to_stream, delta, child, default_replication_var, flags, version, owner, false, default_replication_var);
				}
			}
			else if (child->symbol->type == SymbolType::ClassProperty &&
	 				 child->symbol->property_data->is_serialized != 0)
			{
				if (child->symbol->property_data->serialized_version <= version)
				{
					Interpolate_Field(from_stream, to_stream, delta, child, default_replication_var, flags, version, owner, false, default_replication_var);
				}
			}
		}
	}
	else if (sym->symbol->class_data->is_replicated != 0)
	{
		CVMReplicationInfo* rep_info = sym->symbol->class_data->replication_info;
		for (int i = 0; i < rep_info->variable_count; i++)
		{
			CVMReplicationVar& var = rep_info->variables[i];
			if
			(
				flags == CVMObjectSerializeFlags::Server_Replicated || // serialize everything as server.
				var.owner == CVMReplicationVarOwner::ObjectOwner ||
				(var.owner == CVMReplicationVarOwner::Client && flags == CVMObjectSerializeFlags::Client_Replicated) // serialize client vars as client.
			)
			{
				// Read values but don't apply for client variables if we are the owner of this object.
				//bool do_not_apply = (var.owner == CVMReplicationVarOwner::Client && (flags & CVMObjectSerializeFlags::Client_Replicated)) && owner;
				bool do_not_apply = ((var.owner == CVMReplicationVarOwner::Client || var.owner == CVMReplicationVarOwner::ObjectOwner) && owner);

				if (var.var_symbol_index >= 0)
				{
					CVMLinkedSymbol* field = m_vm->Get_Symbol_Table_Entry(var.var_symbol_index);
					if (field->symbol->field_data->serialized_version <= version)
					{
						Interpolate_Field(from_stream, to_stream, delta, field, &var, flags, version, owner, do_not_apply, default_replication_var);
					}
				}
				else
				{
					CVMLinkedSymbol* prop = m_vm->Get_Symbol_Table_Entry(var.prop_symbol_index);
					if (prop->symbol->property_data->serialized_version <= version)
					{
						Interpolate_Field(from_stream, to_stream, delta, prop, &var, flags, version, owner, do_not_apply, default_replication_var);
					}
				}
			}
		}
	}
}

void CVMObject::Interpolate(Stream* from_stream, Stream* to_stream, float delta, CVMObjectSerializeFlags::Type flags, int version, bool owner, CVMReplicationVar* default_replication_var)
{
	CVMReplicationVar default_replication_mode;
	default_replication_mode.mode = CVMReplicationVarMode::Interpolate_Linear;


	if (default_replication_var == NULL)
	{
		default_replication_var = &default_replication_mode;
	}

	CVMLinkedSymbol* sym = Get_Symbol();
	Interpolate_Class(from_stream, to_stream, delta, flags, sym, version, owner, default_replication_var);
}

Vector4 CVMObject::To_Vec4()
{
	Vector4 vec;
	vec.X = Get_Slot(0).float_value;
	vec.Y = Get_Slot(1).float_value;
	vec.Z = Get_Slot(2).float_value;
	vec.W = Get_Slot(3).float_value;
	return vec;
}

Vector3 CVMObject::To_Vec3()
{
	Vector3 vec;
	vec.X = Get_Slot(0).float_value;
	vec.Y = Get_Slot(1).float_value;
	vec.Z = Get_Slot(2).float_value;
	return vec;
}

Vector2 CVMObject::To_Vec2()
{
	Vector2 vec;
	vec.X = Get_Slot(0).float_value;
	vec.Y = Get_Slot(1).float_value;
	return vec;
}