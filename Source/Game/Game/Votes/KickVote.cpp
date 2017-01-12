// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Game/Votes/KickVote.h"

#include "Game/Network/GameNetManager.h"
#include "Game/Network/GameNetUser.h"
#include "Game/Network/GameNetServer.h"

#include "Game/Game/ChatManager.h"

#include "Engine/Localise/Locale.h"

KickVote::KickVote()
{
}

int	KickVote::Get_Type()
{
	static int hash = StringHelper::Hash("KickVote");
	return hash;
}

std::string KickVote::Get_Name()
{
	return S("#sub_menu_kick_vote_name");
}

std::string KickVote::Get_Description()
{
	NetUser* user = NetManager::Get()->Get_User_By_Net_ID(Get_User_ID());
	if (user != NULL)
	{
		return SF("#sub_menu_kick_vote_description", user->Get_Username().c_str());
	}
	else
	{
		return SF("#sub_menu_kick_vote_description", S("#sub_menu_kick_vote_unknown_user"));
	}
}

int	KickVote::Get_Timeout_Period()
{
	return 30 * 1000;
}

bool KickVote::Is_Valid()
{
	NetUser* user = NetManager::Get()->Get_User_By_Net_ID(Get_User_ID());
	return (user != NULL);
}

void KickVote::Pass()
{
	NetUser* user = NetManager::Get()->Get_User_By_Net_ID(Get_User_ID());
	if (user != NULL)
	{
		if (user->Get_Online_User()->Is_Developer())
		{
			ChatManager::Get()->Send_Server(ChatMessageType::Server, S("#chat_message_vote_cannot_kick_dev"));
		}
		else if (user->Get_Online_User()->Is_Local())
		{
			ChatManager::Get()->Send_Server(ChatMessageType::Server, S("#chat_message_vote_cannot_kick_host"));
		}
		else
		{
			NetManager::Get()->Server()->Kick_User(user);
		}
	}
}

void KickVote::Fail()
{
	// Nothing to do here.
}

Vote* KickVote::Clone()
{
	return new KickVote();
}