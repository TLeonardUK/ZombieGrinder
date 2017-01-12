/* *****************************************************************

		CVMObject.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CVMOBJECT_H_
#define _CVMOBJECT_H_

// Uses an array rather than a linked list to store roots/objects, better spatial locality, less cache thrashing.
#define GC_USE_ARRAY_GLOBAL_LISTS

#include "Generic/Types/String.h"
#include "Generic/Types/Vector4.h"
#include "Generic/Types/Vector3.h"
#include "Generic/Types/Vector2.h"
#include <vector>

#include "Engine/Platform/Platform.h"

#include "XScript/VirtualMachine/CVMString.h"

class CVMValue;
struct CVMLinkedSymbol;
class CVirtualMachine;
struct CVMReplicationVar;
class Stream;
struct CVMDataType;
class ActorReplicator;
class CVMObjectHandle;

extern void VM_Static_Null_Deference_Error(CVirtualMachine* vm);

// =================================================================
//	Flags to determine how to serialize objects.
// =================================================================
struct CVMObjectSerializeFlags
{
	enum Type
	{
		Client_Replicated,
		Server_Replicated,
		Full
	};
};

// =================================================================
//	A virtual machine object!
// =================================================================
class CVMObject
{
private:

#ifdef GC_USE_ARRAY_GLOBAL_LISTS
	static CVMObject** m_collectable_list;
	static int m_collectable_list_size;
	static int m_collectable_list_capacity;

	static CVMObject** m_uncollectable_list;
	static int m_uncollectable_list_size;
	static int m_uncollectable_list_capacity;
#else
	static CVMObject* m_global_list;
	static int m_global_list_size;
#endif

#ifndef GC_USE_ARRAY_GLOBAL_LISTS
	CVMObject* m_next;
	CVMObject* m_prev;
#endif
	bool m_in_list;
	int	m_index;

	bool m_gc_mark;

	enum
	{
		max_state_stack = 16
	};

	CVirtualMachine*	m_vm;
	CVMValue*			m_data;
	int					m_data_size;
	CVMLinkedSymbol*	m_class;
	int					m_capacity;
	void*				m_meta_data;

	int					m_state_change_counter;

	int					m_uncollectable;

	int					m_state_stack_index;
	CVMLinkedSymbol*	m_state_stack[max_state_stack];

	void Quick_Sort(CVMObject* comparer, CVMLinkedSymbol* sort_func, int left, int right);
	int  Quick_Sort_Partition(CVMObject* comprarer, CVMLinkedSymbol* sort_func, int left, int right, int pivot);

	CVMObject(const CVMObject& other)
	{
		// No copy constructor for youuu.
	}

	friend class CVirtualMachine;
	friend class ActorReplicator;

public:

	INLINE void* Get_Meta_Data()
	{
		return m_meta_data;
	}

	void* Get_Meta_Data_NullCheck(CVirtualMachine* vm);

	INLINE void Set_Meta_Data(void* data)
	{
		m_meta_data = data;
	}

	INLINE int Get_State_Change_Counter()
	{
		return m_state_change_counter;
	}

	// You had better fucking now what your doing if you use this function.
	CVMValue* Get_Data()
	{
		return m_data;
	}

	void Mark_Collectable();
	void Mark_Uncollectable();

	void Remove_Collectable_Link();
	void Add_Collectable_Link();

	void Remove_Uncollectable_Link();
	void Add_Uncollectable_Link();

public:
	friend class CGarbageCollector;
	friend class CVMGCRoot;

	~CVMObject();
	CVMObject(CVirtualMachine* vm);

	// Internal stuff.
	void Init_Object(CVMLinkedSymbol* sym);
	void Init_Array(CVMLinkedSymbol* sym, int size);
	CVMLinkedSymbol* Get_Symbol();
	CVMValue& Get_Slot(int index);
	int Slot_Count();

	// Conversion stuff.
	Vector4 To_Vec4();
	Vector3 To_Vec3();
	Vector2 To_Vec2();

	// State stuff.
	CVMLinkedSymbol* Current_State();
	void Push_State(CVMLinkedSymbol* symbol);
	void Pop_State();

	// Manipulation of values.
	void Clear_Data();
	void Resize(int size);
	CVMObject* Slice(int start);
	CVMObject* Slice(int start, int end);
	void Shift(int offset);
	void Add_First(CVMValue value);
	void Add_Array_First(CVMObject* value);
	void Add_Last(CVMValue value);
	void Add_Array_Last(CVMObject* value);
	void Clear();
	CVMObject* Clone();
	void CopyTo(CVMObject* value);
	bool Contains(CVMValue needle);
	void Replace(CVMValue what, CVMValue to);
	void Reverse();
	CVMValue Remove_Index(int index);
	void Remove(CVMValue needle);
	CVMValue Remove_First();
	CVMValue Remove_Last();
	void Insert(CVMValue value, int index);
	void Trim_Start(CVMValue needle);
	void Trim_End(CVMValue needle);
	void Trim(CVMValue needle);
	void Pad_Left(int length, CVMValue pad_value);
	void Pad_Right(int length, CVMValue pad_value);
	int Index_Of(CVMValue pad_value, int start);
	int Index_Of_Any(CVMValue pad_value, int start);
	int Last_Index_Of(CVMValue pad_value, int start);
	int Last_Index_Of_Any(CVMValue pad_value, int start);
	void Sort(CVMObject* comparer);
	
	// Serialization.
	void Serialize_Value(Stream* stream, CVMValue& value, CVMDataType* type, CVMObjectSerializeFlags::Type flags, int* final_version);
	void Serialize_Field(Stream* stream, CVMLinkedSymbol* sym, CVMReplicationVar* props, CVMObjectSerializeFlags::Type flags, int* final_version);
	void Serialize_Class(Stream* stream, CVMObjectSerializeFlags::Type flags, CVMLinkedSymbol* sym, int* final_version);
	void Serialize(Stream* stream, CVMObjectSerializeFlags::Type flags, int* final_version);

	void Deserialize_Value(Stream* stream, CVMValue& value, CVMDataType* type, CVMObjectSerializeFlags::Type flags, int version);
	void Deserialize_Field(Stream* stream, CVMLinkedSymbol* sym, CVMReplicationVar* props, CVMObjectSerializeFlags::Type flags, int version);
	void Deserialize_Class(Stream* stream, CVMObjectSerializeFlags::Type flags, CVMLinkedSymbol* sym, int version);
	void Deserialize(Stream* stream, CVMObjectSerializeFlags::Type flags, int version);

	void Interpolate_Value(Stream* from_stream, Stream* to_stream, float delta, CVMValue& value, CVMDataType* type, CVMObjectSerializeFlags::Type flags, int version, bool owner, bool do_not_apply, CVMReplicationVar* default_replication_var);
	void Interpolate_Field(Stream* from_stream, Stream* to_stream, float delta, CVMLinkedSymbol* sym, CVMReplicationVar* props, CVMObjectSerializeFlags::Type flags, int version, bool owner, bool do_not_apply, CVMReplicationVar* default_replication_var);
	void Interpolate_Class(Stream* from_stream, Stream* to_stream, float delta, CVMObjectSerializeFlags::Type flags, CVMLinkedSymbol* sym, int version, bool owner, CVMReplicationVar* default_replication_var);
	void Interpolate(Stream* from_stream, Stream* to_stream, float delta, CVMObjectSerializeFlags::Type flags, int version, bool owner, CVMReplicationVar* default_replication_var = NULL);

};

// =================================================================
//	An handle to a virtual machine object. Used for shitty
//	reference counting :S. Need a better GC.
// =================================================================
class CVMObjectHandle
{
private:
	CVMObject* m_object;
	int m_vf_table_offset;

public:
	~CVMObjectHandle();
	CVMObjectHandle();
	CVMObjectHandle(CVMObject* object, int vf_table_offset);
	CVMObjectHandle(CVMObject* object);
	
	CVMObjectHandle(const CVMObjectHandle& other);
	CVMObjectHandle& operator =(const CVMObjectHandle& other);

	INLINE CVMObject* Get()
	{
		return m_object;
	}

	CVMObject* GetNullCheck(CVirtualMachine* vm);
	int Get_VF_Table_Offset();

};

#endif