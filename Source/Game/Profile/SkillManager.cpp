// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Profile/SkillManager.h"
#include "Game/Profile/ItemManager.h"

#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMBinary.h"

#include "Engine/Renderer/Atlases/Atlas.h"
#include "Engine/Resources/ResourceFactory.h"

#include "Engine/Engine/GameEngine.h"
#include "Engine/Scripts/EngineVirtualMachine.h"

#include "Engine/Engine/EngineOptions.h"

#include "Game/Runner/Game.h"

SkillManager::SkillManager()
	: m_init(false)
{
}

SkillManager::~SkillManager()
{
}

SkillArchetype* SkillManager::Find_Archetype(CVMLinkedSymbol* symbol)
{
	for (std::vector<SkillArchetype>::iterator iter = m_archetypes.begin(); iter != m_archetypes.end(); iter++)
	{
		SkillArchetype& item = *iter;
		if (item.script_object.Get().Get()->Get_Symbol() == symbol)
		{
			return &item;
		}
	}
	return NULL;
}

SkillArchetype* SkillManager::Find_Archetype(const char* name)
{
	for (std::vector<SkillArchetype>::iterator iter = m_archetypes.begin(); iter != m_archetypes.end(); iter++)
	{
		SkillArchetype& item = *iter;
		if (strcmp(item.script_object.Get().Get()->Get_Symbol()->symbol->name, name) == 0)
		{
			return &item;
		}
	}
	return NULL;
}

SkillArchetype* SkillManager::Find_Archetype(int id)
{
	for (std::vector<SkillArchetype>::iterator iter = m_archetypes.begin(); iter != m_archetypes.end(); iter++)
	{
		SkillArchetype& item = *iter;
		if (item.script_object.Get().Get()->Get_Symbol()->symbol->unique_id == id)
		{
			return &item;
		}
	}
	return NULL;
}

std::vector<SkillArchetype*> SkillManager::Get_Archetypes()
{
	std::vector<SkillArchetype*> result;
	for (std::vector<SkillArchetype>::iterator iter = m_archetypes.begin(); iter != m_archetypes.end(); iter++)
	{
		SkillArchetype& item = *iter;
		result.push_back(&item);
	}
	return result;
}

bool SkillManager::Init()
{	
	DBG_ASSERT(!m_init);
	DBG_LOG("Loading skills ...");

	EngineVirtualMachine* vm = GameEngine::Get()->Get_VM();
	m_skill_class = vm->Find_Class("Skill");
	m_skill_archetype_class = vm->Find_Class("Skill_Archetype");
	DBG_ASSERT(m_skill_class != NULL);
	DBG_ASSERT(m_skill_archetype_class != NULL);
	
	std::vector<CVMLinkedSymbol*> extended_classes = vm->Find_Derived_Classes(m_skill_archetype_class);

	m_archetypes.resize(extended_classes.size());

	CVMContextLock lock = vm->Set_Context(vm->Get_Static_Context());

	int index = 0;
	for (std::vector<CVMLinkedSymbol*>::iterator iter = extended_classes.begin(); iter != extended_classes.end(); iter++, index++)
	{
		CVMLinkedSymbol* item_class = *iter;
		DBG_LOG("Loading Skill Class: %s", item_class->symbol->name);

		SkillArchetype& item = m_archetypes.at(index);

		item.script_object = vm->New_Object(item_class, true, &item);
		vm->Set_Default_State(item.script_object);
	}

	for (std::vector<SkillArchetype>::iterator iter = m_archetypes.begin(); iter != m_archetypes.end(); iter++)
	{
		SkillArchetype& item = *iter;
		if (item.parent_name != "")
		{
			item.parent = Find_Archetype(item.parent_name.c_str());
			DBG_ASSERT_STR(item.parent != NULL, "Could not find skill parent '%s'.", item.parent_name.c_str());

			item.parent->children.push_back(&item);
		}
		else
		{
			item.parent = NULL;
		}

		item.icon = ResourceFactory::Get()->Get_Atlas_Frame(item.icon_frame.c_str()); 
		DBG_ASSERT_STR(item.icon != NULL, "Could not find skill frame '%s'.", item.icon_frame.c_str()); 
	}

	m_init = true;
	return true;
}
