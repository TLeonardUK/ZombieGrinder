// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "XScript/VirtualMachine/CVMBindingHelper.h"
#include "Game/Scripts/Runtime/CRuntime_Engine.h"
#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMContext.h"
#include "XScript/VirtualMachine/CVMBinary.h"

#include "Game/Network/GameNetUser.h"

#include "Engine/Engine/GameEngine.h"

#include "Game/Profile/ProfileManager.h"

#include "Game/Runner/Game.h"

#include "Engine/Input/Input.h"

#include "Engine/Display/GfxDisplay.h"

int CRuntime_Engine::Has_Crashed(CVirtualMachine* vm)
{
	return Game::Get()->Has_Crashed();
}

int CRuntime_Engine::Has_Uploaded_To_Workshop(CVirtualMachine* vm)
{
	return Game::Get()->Has_Uploaded_To_Workshop();
}

int CRuntime_Engine::In_Map_Restricted_Mode(CVirtualMachine* vm)
{
	return Game::Get()->In_Map_Restricted_Mode();
}

int CRuntime_Engine::In_Restricted_Mode(CVirtualMachine* vm)
{
	return Game::Get()->In_Restricted_Mode();
}

int CRuntime_Engine::Get_Highest_Profile_Level(CVirtualMachine* vm)
{
	int max_level = 0;
	for (int i = 0; i < ProfileManager::Get()->Get_Profile_Count(); i++)
	{
		int profile_level = ProfileManager::Get()->Get_Profile(i)->Level;
		if (profile_level > max_level)
		{
			max_level = profile_level;
		}
	}
	return max_level;
}

void CRuntime_Engine::Bind(CVirtualMachine* vm)
{
	vm->Get_Bindings()->Bind_Function<int>("Engine", "Has_Crashed", &Has_Crashed);
	vm->Get_Bindings()->Bind_Function<int>("Engine", "Has_Uploaded_To_Workshop", &Has_Uploaded_To_Workshop);
	vm->Get_Bindings()->Bind_Function<int>("Engine", "In_Map_Restricted_Mode", &In_Map_Restricted_Mode);
	vm->Get_Bindings()->Bind_Function<int>("Engine", "In_Restricted_Mode", &In_Restricted_Mode);
	vm->Get_Bindings()->Bind_Function<int>("Engine", "Get_Highest_Profile_Level", &Get_Highest_Profile_Level);
}
