// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Game/Votes/ChangeMapVote.h"

#include "Game/Network/GameNetManager.h"
#include "Game/Network/GameNetUser.h"
#include "Game/Network/GameNetServer.h"

#include "Engine/Resources/ResourceFactory.h"

#include "Game/Game/ChatManager.h"

#include "Engine/Scene/Map/MapFileHandle.h"
#include "Engine/Scene/Map/MapFile.h"

#include "Engine/Localise/Locale.h"

#include "Generic/Math/Random.h"

ChangeMapVote::ChangeMapVote()
{
}

int	ChangeMapVote::Get_Type()
{
	static int hash = StringHelper::Hash("ChangeMapVote");
	return hash;
}

std::string ChangeMapVote::Get_Name()
{
	return S("#sub_menu_change_map_vote_name");
}

std::string ChangeMapVote::Get_Description()
{
	MapFileHandle* handle = ResourceFactory::Get()->Get_Map_File_By_GUID(Get_Extra().c_str());
	if (handle != NULL)
	{
		return SF("#sub_menu_change_map_vote_description", handle->Get()->Get_Header()->Short_Name.c_str());
	}
	else
	{
		return SF("#sub_menu_change_map_vote_description", S("#sub_menu_kick_vote_unknown_map"));
	}
}

int	ChangeMapVote::Get_Timeout_Period()
{
	return 30 * 1000;
}

bool ChangeMapVote::Is_Valid()
{
	MapFileHandle* handle = ResourceFactory::Get()->Get_Map_File_By_GUID(Get_Extra().c_str());

	// If server dosen't have map we cannot change to it.
	if (handle == NULL)
	{
		NetUser* user = NetManager::Get()->Get_User_By_Net_ID(Get_User_ID());
		if (user != NULL)
		{
			ChatManager::Get()->Send_Server(ChatMessageType::Server, SF("#chat_message_called_invalid_map_vote", user->Get_Display_Username().c_str()));
		}
		else
		{
			ChatManager::Get()->Send_Server(ChatMessageType::Server, SF("#chat_message_called_invalid_map_vote", "Unknown"));
		}
		DBG_LOG("Ignoring client vote for map thats not installed on server.");

		return false;
	}

	return true;
}

void ChangeMapVote::Pass()
{
	MapFileHandle* handle = ResourceFactory::Get()->Get_Map_File_By_GUID(Get_Extra().c_str());
	if (handle != NULL)
	{
		NetManager::Get()->Server()->Force_Map_Change(
			handle->Get()->Get_Header()->GUID.c_str(), 
			handle->Get()->Get_Header()->Workshop_ID,
			Random::Static_Next(),
			0);
	}
}

void ChangeMapVote::Fail()
{
	// Nothing to do here.
}

Vote* ChangeMapVote::Clone()
{
	return new ChangeMapVote();
}

