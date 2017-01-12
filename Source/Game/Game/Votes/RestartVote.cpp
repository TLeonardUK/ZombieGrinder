// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Game/Votes/RestartVote.h"

#include "Game/Network/GameNetManager.h"
#include "Game/Network/GameNetUser.h"
#include "Game/Network/GameNetServer.h"

#include "Engine/Localise/Locale.h"

RestartVote::RestartVote()
{
}

int	RestartVote::Get_Type()
{
	static int hash = StringHelper::Hash("RestartVote");
	return hash;
}

std::string RestartVote::Get_Name()
{
	return S("#sub_menu_restart_vote_name");
}

std::string RestartVote::Get_Description()
{
	return S("#sub_menu_restart_vote_description");
}

int	RestartVote::Get_Timeout_Period()
{
	return 30 * 1000;
}

bool RestartVote::Is_Valid()
{
	return true;
}

void RestartVote::Pass()
{
	NetManager::Get()->Server()->Restart_Map();
}

void RestartVote::Fail()
{
	// Nothing to do here.
}

Vote* RestartVote::Clone()
{
	return new RestartVote();
}