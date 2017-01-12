// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Game/Votes/EditModeVote.h"

#include "Game/Network/GameNetManager.h"
#include "Game/Network/GameNetUser.h"
#include "Game/Network/GameNetServer.h"

#include "Engine/Localise/Locale.h"

EditModeVote::EditModeVote()
{
}

int	EditModeVote::Get_Type()
{
	static int hash = StringHelper::Hash("EditModeVote");
	return hash;
}

std::string EditModeVote::Get_Name()
{
	return S("#sub_menu_edit_mode_vote_name");
}

std::string EditModeVote::Get_Description()
{
	GameNetServer* server = GameNetManager::Get()->Game_Server();
	GameNetClient* client = GameNetManager::Get()->Game_Client();

	GameNetServerState state;
	state.In_Editor_Mode = false;

	if (client != NULL)
	{
		state = client->Get_Server_State();
	}
	else if (server != NULL)
	{
		state = server->Get_Server_State();
	}

	if (state.In_Editor_Mode == true)
	{
		return S("#sub_menu_edit_mode_vote_exit_description");
	}
	else
	{
		return S("#sub_menu_edit_mode_vote_enter_description");
	}
}

int	EditModeVote::Get_Timeout_Period()
{
	return 30 * 1000;
}

bool EditModeVote::Is_Valid()
{
	return true;
}

void EditModeVote::Pass()
{
	GameNetManager::Get()->Game_Server()->Toggle_Editor_Mode();
}

void EditModeVote::Fail()
{
	// Nothing to do here.
}

Vote* EditModeVote::Clone()
{
	return new EditModeVote();
}