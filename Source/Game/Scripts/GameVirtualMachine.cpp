// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Scripts/GameVirtualMachine.h"

#include "Game/Scripts/Runtime/Components/CRuntime_SpriteComponent.h"
#include "Game/Scripts/Runtime/Components/CRuntime_CollisionComponent.h"
#include "Game/Scripts/Runtime/Components/CRuntime_EffectComponent.h"
#include "Game/Scripts/Runtime/Components/CRuntime_BoidComponent.h"
#include "Game/Scripts/Runtime/Components/CRuntime_PathPlannerComponent.h"
#include "Game/Scripts/Runtime/Components/CRuntime_ParticleCollectorComponent.h"
#include "Game/Scripts/Runtime/CRuntime_Actor.h"
#include "Game/Scripts/Runtime/CRuntime_Network.h"
#include "Game/Scripts/Runtime/CRuntime_NetUser.h"
#include "Game/Scripts/Runtime/CRuntime_Achievement.h"
#include "Game/Scripts/Runtime/CRuntime_Challenge.h"
#include "Game/Scripts/Runtime/CRuntime_DLC.h"
#include "Game/Scripts/Runtime/CRuntime_Leaderboard.h"
#include "Game/Scripts/Runtime/CRuntime_Statistics.h"
#include "Game/Scripts/Runtime/CRuntime_Item.h"
#include "Game/Scripts/Runtime/CRuntime_Item_Combination.h"
#include "Game/Scripts/Runtime/CRuntime_Item_Archetype.h"
#include "Game/Scripts/Runtime/CRuntime_Item_Upgrade_Tree.h"
#include "Game/Scripts/Runtime/CRuntime_Item_Upgrade_Tree_Node.h"
#include "Game/Scripts/Runtime/CRuntime_Item_Upgrade_Tree_Node_Modifier.h"
#include "Game/Scripts/Runtime/CRuntime_Skill.h"
#include "Game/Scripts/Runtime/CRuntime_Skill_Archetype.h"
#include "Game/Scripts/Runtime/CRuntime_Profile.h"
#include "Game/Scripts/Runtime/CRuntime_Time.h"
#include "Game/Scripts/Runtime/CRuntime_Scene.h"
#include "Game/Scripts/Runtime/CRuntime_Input.h"
#include "Game/Scripts/Runtime/CRuntime_Game_Mode.h"
#include "Game/Scripts/Runtime/CRuntime_FX.h"
#include "Game/Scripts/Runtime/CRuntime_Event.h"
#include "Game/Scripts/Runtime/CRuntime_Post_Process_FX.h"
#include "Game/Scripts/Runtime/CRuntime_Pathing.h"
#include "Game/Scripts/Runtime/CRuntime_Engine.h"
#include "Game/Scripts/Runtime/CRuntime_Options.h"

void GameVirtualMachine::Register_Runtime()
{
	// Register base types.
	EngineVirtualMachine::Register_Runtime();

	// Register game types.
	CRuntime_SpriteComponent::Bind(this);
	CRuntime_CollisionComponent::Bind(this);
	CRuntime_EffectComponent::Bind(this);
	CRuntime_BoidComponent::Bind(this);
	CRuntime_PathPlannerComponent::Bind(this);
	CRuntime_Actor::Bind(this);
	CRuntime_NetUser::Bind(this);
	CRuntime_Network::Bind(this);
	CRuntime_Achievement::Bind(this);
	CRuntime_Challenge::Bind(this);
	CRuntime_DLC::Bind(this);
	CRuntime_Leaderboard::Bind(this);
	CRuntime_Statistics::Bind(this);
	CRuntime_Item::Bind(this);
	CRuntime_Item_Archetype::Bind(this);
	CRuntime_Item_Upgrade_Tree::Bind(this);
	CRuntime_Item_Upgrade_Tree_Node::Bind(this);
	CRuntime_Item_Upgrade_Tree_Node_Modifier::Bind(this);
	CRuntime_Item_Combination::Bind(this);
	CRuntime_Skill::Bind(this);
	CRuntime_Skill_Archetype::Bind(this);
	CRuntime_Profile::Bind(this);
	CRuntime_Time::Bind(this);
	CRuntime_Scene::Bind(this);
	CRuntime_Input::Bind(this);
	CRuntime_Game_Mode::Bind(this);
	CRuntime_FX::Bind(this);
	CRuntime_Event::Bind(this);
	CRuntime_Post_Process_FX::Bind(this);
	CRuntime_ParticleCollectorComponent::Bind(this);
	CRuntime_Pathing::Bind(this);
	CRuntime_Engine::Bind(this);
	CRuntime_Options::Bind(this);
}

GameVirtualMachine::GameVirtualMachine()
	: EngineVirtualMachine()
{
}

GameVirtualMachine::~GameVirtualMachine()
{
}

