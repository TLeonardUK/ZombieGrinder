// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_ONLINE_RANKINGS_MANAGER_
#define _GAME_ONLINE_RANKINGS_MANAGER_

#include "Engine/IO/BinaryStream.h"
#include "Engine/Engine/FrameTime.h"

#include "Generic/Types/HashTable.h"

#include "XScript/VirtualMachine/CVMObject.h"
#include "XScript/VirtualMachine/CGarbageCollector.h"

#include "Generic/Patterns/Singleton.h"

struct AtlasFrame;
class OnlineRankingsBoard;
struct Leaderboard;
class OnlineRankingsEntries;

struct LeaderboardEntryStatus
{
	enum Type
	{
		Unloaded,
		Loading,
		Loaded
	};
};

struct LeaderboardEntry
{
	std::string						name;
	int								rank;
	int								score;
	int								country;
	int								real_score;
	LeaderboardEntryStatus::Type	status;

	LeaderboardEntry()
		: name("")
		, rank(0)
		, score(0)
		, country(0)
		, status(LeaderboardEntryStatus::Unloaded)
		, real_score(0)
	{
	}

	static bool Sort_By_Score_Predicate(const LeaderboardEntry& A, const LeaderboardEntry& B)
	{
		return A.score > B.score;
	}
};

struct LeaderboardTransactionType
{
	enum Type
	{
		Submit,
		Refresh_Range,
		Refresh_Own_Entry,
	};
};

struct LeaderboardTransaction
{
public:
	LeaderboardTransactionType::Type	type;
	Leaderboard*						board;
	int									offset;
	int									count;
	int									score;
	int									real_score;
	bool								friends_only;
	OnlineRankingsEntries*				entries;
};

struct Leaderboard
{
public:
	CVMGCRoot						script_object;
	OnlineRankingsBoard*				online_rankings_board;
			
	std::string							id;
	std::string							name;
	bool								replace_scores;
	bool								ascending;
	bool								is_internal;

	int									total_entries;
	int									highest_entry;

	LeaderboardEntry					own_entry;
	HashTable<LeaderboardEntry, int>	entries;
};

struct Ranking_Metadata
{
	int country_id;
	int real_score;
};

struct RankingsManager : public Singleton<RankingsManager>
{
	MEMORY_ALLOCATOR(RankingsManager, "Game");

private:
	bool m_init;

	std::vector<Leaderboard*> m_boards;
	std::vector<LeaderboardTransaction> m_transactions;

	Leaderboard* Generate_Map_Leaderboard(std::string name);

public:
	RankingsManager();
	~RankingsManager();

	bool Init();

	bool Serialize(BinaryStream* stream);
	bool Deserialize(BinaryStream* stream);

	void Tick(const FrameTime& time);

	std::vector<Leaderboard*> Get_Leaderboards();
	std::vector<Leaderboard*> Get_Non_Internal_Leaderboards();
	Leaderboard* Get_Leaderboard(const char* name);
	Leaderboard* Get_Or_Create_Leaderboard(const char* name);
	Leaderboard* Get_Map_Leaderboard(const char* map_name);

	void Submit(Leaderboard* board, int value, int real_score = 0);

	void Clear_Board(Leaderboard* board);
	void Refresh_Entries(Leaderboard* board, int offset, int count, bool friends_only);

	LeaderboardEntry* Get_Own_Entry(Leaderboard* board);
	LeaderboardEntry* Get_Board_Entry(Leaderboard* board, int rank);

};

#endif

