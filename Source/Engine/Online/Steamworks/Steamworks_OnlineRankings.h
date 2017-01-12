// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_STEAMWORKS_ONLINE_RANKINGS_
#define _ENGINE_STEAMWORKS_ONLINE_RANKINGS_

#include "Engine/Online/OnlineRankings.h"
#include "Engine/Online/Steamworks/Steamworks_OnlinePlatform.h"

class Steamworks_OnlineRankings;

class Steamworks_OnlineRankingsBoard : public OnlineRankingsBoard
{
public:
	std::string														id;
	SteamAPICall_t													find_call;
	CCallResult<Steamworks_OnlineRankings, LeaderboardFindResult_t>	find_call_result;
	SteamLeaderboard_t												handle;
	bool															is_ready;
	bool															replace_score;

	bool															has_rank;
	int																current_rank;
	int																current_score;
};


class Steamworks_OnlineRankingsTransaction 
{
public:
	bool completed;

	virtual ~Steamworks_OnlineRankingsTransaction()
	{
	};
};

class Steamworks_OnlineRankings : public OnlineRankings
{
	MEMORY_ALLOCATOR(Steamworks_OnlineRankings, "Network");

private:
	std::vector<Steamworks_OnlineRankingsBoard*> m_boards;
	std::vector<Steamworks_OnlineRankingsTransaction*> m_transactions;
	
private:
	void On_Find_Leaderboard(LeaderboardFindResult_t *pFindLeaderboardResult, bool bIOFailure);
	void On_Uploaded_Entries(LeaderboardScoreUploaded_t *pLeaderboardScoresUploaded, bool bIOFailure);

public:
	~Steamworks_OnlineRankings();

	bool Initialize();

	void Tick(const FrameTime& time);

	OnlineRankingsBoard* Get_Board(const char* id, bool replace_score, bool ascending);
	bool Is_Board_Ready(OnlineRankingsBoard* board);

	void Submit(OnlineRankingsBoard* board, int score, void* meta_data, int meta_data_size);
	
	OnlineRankingsEntries* Get_Rankings(OnlineRankingsBoard* board, int offset, int count, bool friends_only);
	OnlineRankingsEntries* Get_Own_Ranking(OnlineRankingsBoard* board);

};

#endif

