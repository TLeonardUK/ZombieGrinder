// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Scene/Actors/Components/Tickable/TickableComponent.h"

#include "Engine/Engine/GameEngine.h"
#include "Engine/Scene/Scene.h"

TickableComponent::TickableComponent()
{
}

TickableComponent::~TickableComponent()
{
	Scene* scene = GameEngine::Get()->Get_Scene();
	if (scene)
	{
		scene->Remove_Tickable(this);
	}
}