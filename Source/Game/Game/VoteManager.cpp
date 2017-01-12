// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Game/VoteManager.h"
#include "Game/Game/ChatManager.h"

#include "Engine/Input/Input.h"

#include "Engine/Renderer/Text/MarkupFontRenderer.h"

#include "Game/Game/Votes/EditModeVote.h"
#include "Game/Game/Votes/KickVote.h"
#include "Game/Game/Votes/RestartVote.h"
#include "Game/Game/Votes/ChangeMapVote.h"

#include "Engine/Localise/Locale.h"

#include "Engine/Network/NetServer.h"
#include "Engine/Network/NetConnection.h"
#include "Engine/Network/NetUser.h"

#include "Generic/Math/Math.h"

Vote::Vote()
	: m_user_id(-1)
	, m_extra("")
{
}
	
int Vote::Get_User_ID()
{
	return m_user_id;
}

void Vote::Set_User_ID(int id)
{
	m_user_id = id;
}
	
std::string Vote::Get_Extra()
{
	return m_extra;
}

void Vote::Set_Extra(std::string id)
{
	m_extra = id;
}

VoteManager::VoteManager()
	: m_active_vote(NULL)
	, m_last_vote_result(false)
	, m_vote_time_elapsed(0.0f)
	, m_callback_server_call_vote_recieved(this, &VoteManager::Callback_Recieve_Call_Vote, false, NULL)
	, m_callback_server_vote_recieved(this, &VoteManager::Callback_Recieve_Vote, false, NULL)
	, m_callback_client_vote_update_recieved(this, &VoteManager::Callback_Recieve_Update_Vote, false, NULL)
{
	Register_Vote_Type(new EditModeVote());
	Register_Vote_Type(new KickVote());
	Register_Vote_Type(new RestartVote());
	Register_Vote_Type(new ChangeMapVote());
}

VoteManager::~VoteManager()
{
	for (std::vector<Vote*>::iterator iter = m_types.begin(); iter != m_types.end(); iter++)
	{
		SAFE_DELETE(*iter);
	}
	m_types.clear();
	SAFE_DELETE(m_active_vote);
}

void VoteManager::Register_Vote_Type(Vote* type)
{
	m_types.push_back(type);
}

Vote* VoteManager::New_Vote(int type)
{
	for (std::vector<Vote*>::iterator iter = m_types.begin(); iter != m_types.end(); iter++)
	{
		Vote* vote = *iter;
		if (vote->Get_Type() == type)
		{
			return vote->Clone();
		}
	}
	return NULL;
}

Vote* VoteManager::Get_Active_Vote()
{
	return m_active_vote;
}

int VoteManager::Get_Total_Votes()
{
	if (!NetManager::Get()->Server())
	{
		return m_total_votes;
	}
	else
	{
		return m_vote_entries.size();
	}
}

int VoteManager::Get_Total_Pass_Votes()
{
	if (!NetManager::Get()->Server())
	{
		return m_pass_votes;
	}
	else
	{
		int count = 0;
		for (std::vector<VoteEntry>::iterator iter = m_vote_entries.begin(); iter != m_vote_entries.end(); iter++)
		{
			VoteEntry& entry = *iter;
			if (entry.Choice == true)
			{
				count++;
			}
		}
		return count;
	}
}

int VoteManager::Get_Total_Fail_Votes()
{
	if (!NetManager::Get()->Server())
	{
		return m_fail_votes;
	}
	else
	{
		int count = 0;
		for (std::vector<VoteEntry>::iterator iter = m_vote_entries.begin(); iter != m_vote_entries.end(); iter++)
		{
			VoteEntry& entry = *iter;
			if (entry.Choice == false)
			{
				count++;
			}
		}
		return count;
	}
}

float VoteManager::Get_Time_Remaining()
{
	if (m_active_vote == NULL)
	{
		return 0.0f;
	}
	else
	{
		float time = (float)m_active_vote->Get_Timeout_Period();
		float elapsed = m_vote_time_elapsed;
		return Max(0.0f, time - elapsed);
	}
}

void VoteManager::Tick(const FrameTime& time)
{
	NetServer* server = NetManager::Get()->Server();
	NetClient* client = NetManager::Get()->Client();

	if (server != NULL && m_active_vote != NULL)
	{
		m_vote_time_elapsed += time.Get_Frame_Time();
		
		// Vote no longer valid (player left etc?)?
		if (!m_active_vote->Is_Valid())
		{
			m_vote_entries.clear();
			Complete_Vote();
		}
	
		// Time up for vote?	
		else if (m_vote_time_elapsed > m_active_vote->Get_Timeout_Period())
		{
			Complete_Vote();
		}
	}

	// Local user voted?
	for (int i = 0; i < MAX_INPUT_SOURCES; i++)
	{
		if (Input::Get(i)->Was_Pressed(OutputBindings::VoteYes))
		{
			Make_Vote(true);
		}
		else if (Input::Get(i)->Was_Pressed(OutputBindings::VoteNo))
		{
			Make_Vote(false);
		}
	}
}

void VoteManager::Complete_Vote(bool vote_force, bool force_vote)
{
	int pass_votes = Get_Total_Pass_Votes();
	int fail_votes = Get_Total_Fail_Votes();

	m_last_vote_result = vote_force ? force_vote : (pass_votes > fail_votes);
	
	if (m_last_vote_result == true)
	{
		DBG_LOG("Vote passed (%i to %i votes)", pass_votes, fail_votes);
		ChatManager::Get()->Send_Server(ChatMessageType::Server, S("#chat_message_vote_passed"));
		m_active_vote->Pass();
	}
	else
	{
		DBG_LOG("Vote failed (%i to %i votes)", pass_votes, fail_votes);
		ChatManager::Get()->Send_Server(ChatMessageType::Server, S("#chat_message_vote_failed"));
		m_active_vote->Fail();
	}
	
	SAFE_DELETE(m_active_vote);

	Broadcast_Update();
}

void VoteManager::Broadcast_Update()
{
	NetPacket_S2C_UpdateVote packet;
	packet.Active	= (m_active_vote != NULL);
	packet.Outcome	= (m_last_vote_result);

	if (m_active_vote != NULL)
	{
		packet.Vote_Type	= m_active_vote->Get_Type();
		packet.User_ID		= m_active_vote->Get_User_ID();
		packet.Extra		= m_active_vote->Get_Extra();

		packet.Total_Votes	= Get_Total_Votes();
		packet.Pass_Votes	= Get_Total_Pass_Votes();
		packet.Fail_Votes	= Get_Total_Fail_Votes();
	}

	NetManager::Get()->Server()->Get_Connection()->Broadcast(&packet, NET_CONNECTION_CHANNEL_SYSTEM_RELIABLE, true);
}

bool VoteManager::Callback_Recieve_Vote(NetPacket_C2S_Vote* packet)
{
	NetConnection* connection = packet->Get_Recieved_From();

	NetUser* user = NetManager::Get()->Get_User_By_Connection(connection);
	if (user == NULL)
	{
		return true;
	}

	// Wut, no active vote.	
	if (m_active_vote == NULL)
	{
		DBG_LOG("Client '%s' changed vote, but no vote is active.", user->Get_Username().c_str());
		return true;
	}

	// Has user already voted?
	for (std::vector<VoteEntry>::iterator iter = m_vote_entries.begin(); iter != m_vote_entries.end(); iter++)
	{
		VoteEntry& entry = *iter;
		if (entry.User_ID == user->Get_Online_User()->Get_ID())
		{
			if (entry.Choice != packet->Choice)
			{
				DBG_LOG("Client '%s' changed vote to '%i'.", user->Get_Username().c_str(), entry.Choice);
			}
			else
			{
				DBG_LOG("Client '%s' attempted to vote but already has.", user->Get_Username().c_str());
			}

			entry.Choice = packet->Choice;
			return true;
		}
	}

	// New vote entry then.
	VoteEntry entry;
	entry.User_ID	= user->Get_Online_User()->Get_ID();
	entry.Choice	= packet->Choice;
	m_vote_entries.push_back(entry);
	
	if (packet->Choice)
	{
		DBG_LOG("Client '%s' voted for.", user->Get_Username().c_str());
	}
	else
	{
		DBG_LOG("Client '%s' voted against.", user->Get_Username().c_str());
	}
	
	if (user->Get_Online_User()->Is_Developer())
	{
		DBG_LOG("Client '%s' is developer. Forcing vote completion.", user->Get_Username().c_str());
		Complete_Vote(true, packet->Choice);
	}
	else
	{
		if (m_vote_entries.size() == NetManager::Get()->Get_Primary_Net_User_Count())
		{
			Complete_Vote();
		}
		else
		{
			Broadcast_Update();
		}
	}

	return true;
}

bool VoteManager::Callback_Recieve_Call_Vote(NetPacket_C2S_CallVote* packet)
{
	NetConnection* connection = packet->Get_Recieved_From();

	NetUser* user = NetManager::Get()->Get_User_By_Connection(connection);
	if (user == NULL)
	{
		return true;
	}

	// Valid vote type?
	Vote* vote = New_Vote(packet->Vote_Type);
	if (vote == NULL)
	{
		SAFE_DELETE(vote);
		DBG_LOG("Recieved call-vote from client, but vote is of an unknown type (0x%08x).", packet->Vote_Type);
		return true;
	}

	// Vote already in progress?
	if (m_active_vote != NULL)
	{
		SAFE_DELETE(vote);
		DBG_LOG("Recieved call-vote from client, but a vote is already in progress..");
		return true;
	}

	// Fill in vote information.
	vote->Set_User_ID(packet->User_ID);
	vote->Set_Extra(packet->Extra);

	// Valid vote?
	if (!vote->Is_Valid())
	{
		SAFE_DELETE(vote);
		DBG_LOG("Recieved call-vote from client, but is not possible, ignoring.");
		return true;
	}

	SAFE_DELETE(m_active_vote);
	m_active_vote = vote;
	m_vote_time_elapsed = 0.0f;
	m_vote_entries.clear();
	
	ChatManager::Get()->Send_Server(ChatMessageType::Server, SF("#chat_message_called_vote", MarkupFontRenderer::Escape(user->Get_Username()).c_str(), vote->Get_Name().c_str()));
	DBG_LOG("Client '%s' started vote '%s'.", user->Get_Username().c_str(), vote->Get_Description().c_str());

	Broadcast_Update();

	return true;
}

bool VoteManager::Callback_Recieve_Update_Vote(NetPacket_S2C_UpdateVote* packet)
{
	// If we are running as a local server, no need to do anything really.
	if (!NetManager::Get()->Server())
	{
		m_total_votes = packet->Total_Votes;
		m_pass_votes  = packet->Pass_Votes;
		m_fail_votes  = packet->Fail_Votes;

		if (packet->Active == true)
		{
			// Just updating existing-vote.
			if (m_active_vote != NULL)
			{
				// Dum de dum do.
			}
			// Vote has started.
			else
			{
				m_active_vote = New_Vote(packet->Vote_Type);
				DBG_ASSERT_STR(m_active_vote != NULL, "Attempted to instantiate unknown vote type.");
			}
		}
		else
		{
			// The fuck. This shouldn't happen. Finishing a non-existant vote.
			if (m_active_vote == NULL)
			{
				// Ignore this, it may happen if we have multiple local-users and recieve updates for each of them.
				//DBG_ASSERT(false);
			}
			// Vote has finished.
			else
			{
				if (packet->Outcome)
				{
					DBG_LOG("Vote '%s' passed.", m_active_vote->Get_Description().c_str());
				}
				else
				{
					DBG_LOG("Vote '%s' failed.", m_active_vote->Get_Description().c_str());
				}

				SAFE_DELETE(m_active_vote);
			}
		}

		if (m_active_vote != NULL)
		{
			m_active_vote->Set_User_ID(packet->User_ID);
			m_active_vote->Set_Extra(packet->Extra);
		}
	}

	return true;
}

void VoteManager::Call_Vote(Vote* vote)
{
	DBG_LOG("Calling vote '%s'.", vote->Get_Description().c_str());

	NetPacket_C2S_CallVote packet;
	packet.User_ID		= vote->Get_User_ID();
	packet.Extra		= vote->Get_Extra();
	packet.Vote_Type	= vote->Get_Type();

	NetManager::Get()->Client()->Get_Connection()->Send(&packet, NET_CONNECTION_CHANNEL_SYSTEM_RELIABLE, true);

	// Vote yes! (We asked for it after all!)
	Make_Vote(true);
}

void VoteManager::Make_Vote(bool for_or_against)
{
	DBG_LOG("Voting %i.", for_or_against);

	NetPacket_C2S_Vote packet;
	packet.Choice = for_or_against;

	NetManager::Get()->Client()->Get_Connection()->Send(&packet, NET_CONNECTION_CHANNEL_SYSTEM_RELIABLE, true);
}
