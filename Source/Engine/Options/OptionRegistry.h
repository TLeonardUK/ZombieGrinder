// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_OPTIONS_OPTION_REGISTRY_
#define _ENGINE_OPTIONS_OPTION_REGISTRY_

#include "Generic/Patterns/Singleton.h"

#include "Engine/Options/Option.h"

#include "Generic/Types/HashTable.h"

#include <vector>

#define MAX_OPTION_REGISTRY_STATE_STACK 8

struct OptionRegistryBuffer
{
	MEMORY_ALLOCATOR(OptionRegistryBuffer, "Engine");

private:
	friend class OptionRegistry;

	char* m_data;
	int m_size;

public:
	OptionRegistryBuffer(const char* data, int data_size);
	OptionRegistryBuffer();
	~OptionRegistryBuffer();

	void Set_Buffer(const char* data, int data_size);

	const char* Data();
	int Size();
};

class OptionRegistry : public Singleton<OptionRegistry>
{
	MEMORY_ALLOCATOR(OptionRegistry, "Engine");

private:
	std::vector<Option*> m_options;
	HashTable<Option*, unsigned int> m_options_map;

	OptionRegistryBuffer m_state_stack[MAX_OPTION_REGISTRY_STATE_STACK];
	int m_state_stack_index;

public:
	OptionRegistry();

	void Register(Option* option);

	Option* Find(const char* name);
	Option* Find(unsigned int name);

	bool Serialize(OptionRegistryBuffer* buffer);
	bool Deserialize(OptionRegistryBuffer* buffer);

	void Push_State();
	void Pop_State(bool apply);

};

#endif

