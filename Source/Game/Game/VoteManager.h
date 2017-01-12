// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_VOTE_MANAGER_
#define _GAME_VOTE_MANAGER_

#include "Engine/Network/NetClient.h"

#include "Game/Network/Packets/GamePackets.h"

class Vote
{
	MEMORY_ALLOCATOR(Vote, "Game");

private:
	int m_user_id;
	std::string m_extra;

public:
	Vote();
	
	int	 Get_User_ID();
	void Set_User_ID(int id);

	std::string	Get_Extra();
	void Set_Extra(std::string id);

	virtual int			Get_Type() = 0;
	virtual std::string Get_Name() = 0;
	virtual std::string Get_Description() = 0;
	virtual int			Get_Timeout_Period() = 0;

	virtual Vote* Clone() = 0;

	virtual bool Is_Valid() = 0;

	virtual void Pass() = 0;
	virtual void Fail() = 0;

};

struct VoteEntry
{
public:
	int		User_ID;
	bool	Choice;
};

class VoteManager : public Singleton<VoteManager>
{
	MEMORY_ALLOCATOR(VoteManager, "Game");

private:
	std::vector<Vote*> m_types;

	Vote* m_active_vote;
	float m_vote_time_elapsed;
	bool  m_last_vote_result;

	int m_total_votes;
	int m_pass_votes;
	int m_fail_votes;

	std::vector<VoteEntry> m_vote_entries;

private:
	PACKET_RECIEVE_SERVER_CALLBACK(VoteManager, Callback_Recieve_Call_Vote, NetPacket_C2S_CallVote, m_callback_server_call_vote_recieved);
	PACKET_RECIEVE_SERVER_CALLBACK(VoteManager, Callback_Recieve_Vote, NetPacket_C2S_Vote, m_callback_server_vote_recieved);
	PACKET_RECIEVE_CLIENT_CALLBACK(VoteManager, Callback_Recieve_Update_Vote, NetPacket_S2C_UpdateVote, m_callback_client_vote_update_recieved);

public:
	VoteManager();
	~VoteManager();

	Vote* Get_Active_Vote();

	void Register_Vote_Type(Vote* type);
	Vote* New_Vote(int type);

	int Get_Total_Votes();
	int Get_Total_Pass_Votes();
	int Get_Total_Fail_Votes();

	float Get_Time_Remaining();

	void Tick(const FrameTime& time);

	void Call_Vote(Vote* vote);
	void Make_Vote(bool for_or_against);
	
	void Broadcast_Update();
	void Complete_Vote(bool vote_force = false, bool force_vote = false);

};

#endif

