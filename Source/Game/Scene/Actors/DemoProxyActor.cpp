// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Scene/Actors/DemoProxyActor.h"

#include "Engine/Engine/GameEngine.h"
#include "Engine/Scene/Scene.h"

DemoProxyActor::~DemoProxyActor()
{
}

bool DemoProxyActor::Is_Demo_Proxy()
{
	return true;
}

bool DemoProxyActor::Is_Relevant_To_Demo()
{
	return true;
}
