/* *****************************************************************

		CVMBinary.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CVMBINARY_H_
#define _CVMBINARY_H_

#include "Generic/Types/String.h"
#include <vector>


#include "XScript/VirtualMachine/CSymbol.h"

class PatchedBinaryStream;
class CSemanter;
struct CVMReplicationInfo;

// =================================================================
// Tightly packed binary data. Do not modify, layout is assumed
// by serialize/deserialize.
// =================================================================
/*
struct CVMOperand
{
	union
	{
		float				float_value;
		int					int_value;
	};
};
struct CVMInstruction
{	
	InstructionOpCode::Type op_code;
	CVMOperand*				operands;//[COperand::max_operands_per_instruction];
};
*/

struct CVMDebugInfo
{
	int bytecode_offset;
	int file_index;
	int row;
	int column;
};

struct CVMInterface
{
	int						interface_symbol_index;
	int						vf_table_offset;
};
struct CVMBaseDataType
{
	enum Type
	{
		Int,
		Bool,
		Float,
		String,
		Array,
		Object,
		Void,
	};
};
struct CVMDataType
{
	CVMBaseDataType::Type	type;
	CVMDataType*			element_type;
	const char*				name;
	int						class_index;
	int						class_index_patched;

	bool Equal_To(CVMDataType* other)
	{
		if (type == other->type &&
			class_index == other->class_index)
		{
			if (element_type != NULL)
			{
				if (other->element_type == NULL)
				{
					return false;
				}
				return element_type->Equal_To(other->element_type);
			}
			return true;
		}
		return false;
	}
};
struct CVMMetaDataType
{
	enum Type
	{
		// Keep synced with CASTNodeMetaDataType
		String,
		Int,
		Float,
		Dictionary
	};
};
struct CVMMetaData
{
	CVMMetaDataType::Type type;

	char*				name;
	float				float_value;
	int					int_value;
	char*				string_value;
};
struct CVMClassData
{
	int					interface_count;
	CVMInterface*		interfaces;
	
	CVMDataType*		data_type;

	int					static_table_size;

	int					super_class_index;
	int					default_constructor_index;

	int					default_state_index;
	int					state_count;
	int*				state_indexes;

	int					is_replicated;
	CVMReplicationInfo*	replication_info;

	int					field_count;
	int*				fields_indexes;

	int					method_count;
	int*				method_indexes;

	int					virtual_table_size;
	int*				virtual_table_indexes;	
	
	int					is_interface;
	int					is_native;

	//int				is_generic;
	//CVMDataType**		generic_types;
};
struct CVMReplicationVarOwner
{
	enum Type
	{
		Server = 0,
		Client = 1,
		ObjectOwner = 2
	};
};
struct CVMReplicationVarMode
{
	enum Type
	{
		Absolute				= 0,
		Interpolate_Linear		= 1,
		Interpolate_Smoothed	= 2,
	};
};
struct CVMReplicationVar
{
	int					var_symbol_index;
	int					prop_symbol_index;
	int					owner;
	int					mode;
	/*
	union
	{
		struct float_value
		{
			f32	min;
			f32	max;
		};
		struct int_value
		{
			s32	min;
			s32	max;
		};
	};
	*/
};
struct CVMReplicationInfo
{
	int				   priority;
	int				   has_client_controlled_vars;
	int				   variable_count;
	CVMReplicationVar* variables;
};
struct CVMStateData
{
	int					is_default;
	int					event_count;
	int*				event_indexes;
};
struct CVMPropertyData
{
	int					is_serialized;
	int					serialized_version;
	CVMDataType*		data_type;
	int					get_property_symbol;
	int					set_property_symbol;
};
struct CVMFieldData
{
	CVMDataType*		data_type;
	int					offset;
	int					is_serialized;
	int					serialized_version;
	int					is_static;
};
struct CVMMethodData
{
	int					return_data_type_class_index;
	CVMDataType*		return_data_type;
	
	int					parameter_count;
	int*				parameter_indexes;
	
	int					is_virtual;
	int					is_native;
	int					is_static;
	int					is_abstract;
	int					is_constructor;
	int					is_rpc;
	
	int					vf_table_index;

	int					register_count;

	int					local_count;
	int*				local_indexes;

	int					debug_count;
	CVMDebugInfo*		debug;

	int					instruction_count;
	int					bytecode_size;
	char*				bytecode;

//	int					instruction_count;
//	CVMInstruction*		instructions;
};
struct CVMVariableData
{
	CVMDataType*		data_type;
	int					is_parameter;
};
struct CVMBinarySymbol
{	
	int					unique_id;
	SymbolType::Type	type;
	const char*			name;
	const char*			native_name;

	int					class_scope_index;

	CSymbolDebugInfo	debug;

	int					children_count;
	int*				children_indexes;

	int					metadata_count;
	CVMMetaData*		metadata;

	// Type-Specific data
	union
	{
		CVMClassData*		class_data;
		CVMFieldData*		field_data;
		CVMMethodData*		method_data;
		CVMVariableData*	variable_data;
		CVMStateData*		state_data;
		CVMPropertyData*	property_data;
	};

	void Get_Meta_Data_Value(const char* name, int* val)
	{
		for (int i = 0; i < metadata_count; i++)
		{
			CVMMetaData& meta = metadata[i];
			if (stricmp(name, meta.name) == 0)
			{
				if (meta.type == CVMMetaDataType::Int)
				{
					*val = meta.int_value;
					return;
				}
			}
		}
	}
	
	void Get_Meta_Data_Value(const char* name, bool* val)
	{
		for (int i = 0; i < metadata_count; i++)
		{
			CVMMetaData& meta = metadata[i];
			if (stricmp(name, meta.name) == 0)
			{
				if (meta.type == CVMMetaDataType::Int)
				{
					*val = (meta.int_value != 0);
					return;
				}
			}
		}
	}

	void Get_Meta_Data_Value(const char* name, float* val)
	{
		for (int i = 0; i < metadata_count; i++)
		{
			CVMMetaData& meta = metadata[i];
			if (stricmp(name, meta.name) == 0)
			{
				if (meta.type == CVMMetaDataType::Float)
				{
					*val = meta.float_value;
					return;
				}
			}
		}
	}

	void Get_Meta_Data_Value(const char* name, const char** val)
	{
		for (int i = 0; i < metadata_count; i++)
		{
			CVMMetaData& meta = metadata[i];
			if (stricmp(name, meta.name) == 0)
			{
				if (meta.type == CVMMetaDataType::String)
				{
					*val = meta.string_value;
					return;
				}
			}
		}
	}

	void Get_Meta_Data_Value(const char* name, std::string* val)
	{
		for (int i = 0; i < metadata_count; i++)
		{
			CVMMetaData& meta = metadata[i];
			if (stricmp(name, meta.name) == 0)
			{
				if (meta.type == CVMMetaDataType::String)
				{
					*val = meta.string_value;
					return;
				}
			}
		}
	}

	// Grab meta data.
	template<typename T>
	T Get_Meta_Data(const char* name, T default_value)
	{
		T value = default_value;
		Get_Meta_Data_Value(name, &value);
		return value;
	}
};
struct CVMBinaryData
{
	const char*			name;
	int					string_table_count;
	int					symbol_table_count;
	char**				string_table;
	CVMBinarySymbol**	symbols;
};

// =================================================================
//	Encapsulates a compiled script which is emitted by the
//  a translator for use on the VM.
// =================================================================
class CVMBinary
{
private:
	CVMBinaryData*					m_resource_data;

	std::vector<String>		m_string_table;
	std::vector<CSymbol*>			m_symbol_table;

	CSemanter*						m_semanter;

	struct MergeBinarySymbol
	{
		CVMBinarySymbol* symbol;
		int index;
		bool have_instructions_been_patched;
		bool have_symbols_been_patched;
	};

	std::vector<MergeBinarySymbol>			m_merge_symbol_table;
	std::vector<String>				m_merge_string_table;
	HashTable<int, int>						m_merge_string_table_lookup;
	HashTable<int, int>						m_merge_symbol_table_lookup;

private:
	String Get_Unique_Symbol_ID(CSymbol* symbol);

	int Add_String_To_Merge_Table(String str);
	int Add_Symbol_To_Merge_Table(CVMBinarySymbol* symbol);
	void Patch_Index_Array(int* src_array, int src_count, std::vector<int> remap);
	void Patch_Data_Type(CVMDataType* dt, std::vector<int> symbol_remaps);
	void Patch_Symbols(std::vector<int> symbol_remaps, std::vector<int> string_remaps);
	void Patch_Instructions(std::vector<int> symbol_remaps, std::vector<int> string_remaps);

public:
	CVMBinary();
	~CVMBinary();

	INLINE CVMBinaryData* GetBinaryData()
	{
		return m_resource_data;
	}

	CVMBaseDataType::Type DataTypeToVMDataType(CDataType* type);

	void Setup(CSemanter* semanter, std::vector<String> string_table, std::vector<CSymbol*> symbol_table);

	void Serialize(PatchedBinaryStream* output);
	void Serialize_Merged(PatchedBinaryStream* output);
	void Deserialize(PatchedBinaryStream* input);

	void Merge(CVMBinary* binary);

};

#endif