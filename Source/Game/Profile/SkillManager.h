// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_SKILL_MANAGER_
#define _GAME_SKILL_MANAGER_

#include "Generic/Patterns/Singleton.h"
#include <vector>

#include "Engine/Localise/Locale.h"

#include "Engine/Renderer/Text/MarkupFontRenderer.h"

#include "XScript/VirtualMachine/CVMObject.h"
#include "XScript/VirtualMachine/CGarbageCollector.h"

class BinaryStream;
struct CVMLinkedSymbol;
struct SkillArchetype;
struct ItemArchetype;
struct AtlasFrame;

struct Skill
{
public:
	SkillArchetype*	archetype;
	int				unique_id;
	int				recieve_time;
	int				equip_slot;
	int				equip_time;
	bool			was_rolled_back;
	CVMGCRoot		script_object;
};

struct SkillUnlockCriteria
{
	enum Type
	{
		Required_Rank = 0,
		Kills_With_Weapon = 1,
		Damage_With_Weapon = 2,
		Ailments_From_Weapon = 3
	};
};

struct SkillEnergyType
{
	enum Type
	{
		Enemy_Kills = 0,
	};
};

struct SkillArchetype
{
public:
	std::string						name;
	std::string						description;

	std::string						icon_frame;
	AtlasFrame*						icon;

	int								cost;

	float							energy_required;
	SkillEnergyType::Type			energy_type;
	
	float							duration;

	bool							is_passive;
	bool							team_based;
	bool							are_children_mutex;
	bool							cannot_rollback;

	std::string						player_effect;

	std::string						parent_name;
	SkillArchetype*					parent;

	std::vector<SkillArchetype*>	children;

	CVMGCRoot						script_object;

	SkillUnlockCriteria::Type		unlock_critera;
	CVMLinkedSymbol*				unlock_critera_item;
	int								unlock_critera_threshold;

	std::string Get_Display_Name()
	{
		std::string escape = MarkupFontRenderer::Escape(S(name.c_str()));
		return SF("#menu_skills_name", escape.c_str());
	}

	SkillArchetype()
		: cannot_rollback(false)
		, duration(0.0f)
		, team_based(false)
		, unlock_critera(SkillUnlockCriteria::Required_Rank)
		, unlock_critera_item(NULL)
		, unlock_critera_threshold(0)
		, energy_required(0.0f)
		, energy_type(SkillEnergyType::Enemy_Kills)
	{
	}
};

class SkillManager : public Singleton<SkillManager>
{
	MEMORY_ALLOCATOR(SkillManager, "Game");

private:
	bool m_init;
	CVMLinkedSymbol* m_skill_class;
	CVMLinkedSymbol* m_skill_archetype_class;
	std::vector<SkillArchetype> m_archetypes;

public:
	SkillManager();
	~SkillManager();

	SkillArchetype* Find_Archetype(CVMLinkedSymbol* symbol);
	SkillArchetype* Find_Archetype(const char* name);
	SkillArchetype* Find_Archetype(int id);
	std::vector<SkillArchetype*> Get_Archetypes();

	bool Init();

};

#endif

