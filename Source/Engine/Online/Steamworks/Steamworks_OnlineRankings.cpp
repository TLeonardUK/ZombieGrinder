// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Online/Steamworks/Steamworks_OnlineRankings.h"

#include "Generic/Math/Math.h"

class Steamworks_OnlineRankingsEntries;

// ===================================================================
class Steamworks_OnlineRankingsSubmitTransaction : public Steamworks_OnlineRankingsTransaction
{
public:
	int32*																meta_data;
	int																	meta_data_size;
	Steamworks_OnlineRankingsBoard*										board;
	SteamAPICall_t														call;
	CCallResult<Steamworks_OnlineRankings, LeaderboardScoreUploaded_t>	call_result;

	~Steamworks_OnlineRankingsSubmitTransaction()
	{
		SAFE_DELETE(meta_data);
	}
};

// ===================================================================
class Steamworks_OnlineRankingsGetRankingsTransaction : public Steamworks_OnlineRankingsTransaction
{
public:
	CSteamID																		steam_id;
	Steamworks_OnlineRankingsBoard*													board;
	SteamAPICall_t																	call;
	CCallResult<Steamworks_OnlineRankingsEntries, LeaderboardScoresDownloaded_t>	call_result;

	~Steamworks_OnlineRankingsGetRankingsTransaction()
	{
	}
};

// ===================================================================
class Steamworks_OnlineRankingsEntry : public OnlineRankingsEntry
{
public:
	std::string name;
	int			score;
	int			rank;
	int32		meta_data[64];
	int			meta_data_count;

	std::string Get_Name()
	{
		return name;
	}

	int Get_Score()
	{
		return score;
	}
	
	int Get_Rank()
	{
		return rank;
	}

	void Get_Meta_Data(void* data, int* size, int max_length)
	{
		DBG_ASSERT(max_length >= meta_data_count * (int)sizeof(int32));
		int to_copy = Min(max_length, meta_data_count * (int)sizeof(int32));
		if (size != NULL)
		{
			*size = to_copy;
		}
		memcpy(data, meta_data, to_copy);
	}
};

// ===================================================================
class Steamworks_OnlineRankingsEntries : public OnlineRankingsEntries
{
public:
	Steamworks_OnlineRankingsEntry* entries;
	int max_entries;
	int entry_count;
	bool loaded;

	virtual ~Steamworks_OnlineRankingsEntries()
	{
		SAFE_DELETE(entries);
	}

	virtual bool Has_Loaded()
	{
		return loaded;
	}

	virtual int Entry_Count()
	{
		return entry_count;
	}

	virtual OnlineRankingsEntry* Get_Entry(int index)
	{
		return &entries[index];
	}

	void On_Downloaded_Entries(LeaderboardScoresDownloaded_t* params, bool bIOFailure)
	{
		if (bIOFailure) 
		{
			DBG_LOG("[Steamworks] Failed to download entries for board.");
			return;
		}
		
		Steamworks_OnlineRankings* rankings = static_cast<Steamworks_OnlineRankings*>(OnlineRankings::Get());

		const char* id = SteamUserStats()->GetLeaderboardName(params->m_hSteamLeaderboard);

		OnlineRankingsBoard* board = rankings->Get_Board(id, false, false);
		Steamworks_OnlineRankingsBoard* sw_board = dynamic_cast<Steamworks_OnlineRankingsBoard*>(board);
		DBG_ASSERT(sw_board != NULL);

		DBG_LOG("Recieved %i rankings for board '%s'.", params->m_cEntryCount, id);

		DBG_ASSERT(params->m_cEntryCount <= max_entries);

		for (int i = 0; i < params->m_cEntryCount; i++)
		{
			LeaderboardEntry_t entry;
			bool valid = true;

			// Get base entry information.
			if (SteamUserStats()->GetDownloadedLeaderboardEntry(params->m_hSteamLeaderboardEntries, i, &entry, (int32*)entries[i].meta_data, 64))
			{
				entries[i].meta_data_count = entry.m_cDetails;
				entries[i].name = SteamFriends()->GetFriendPersonaName(entry.m_steamIDUser);
				entries[i].rank = entry.m_nGlobalRank;
				entries[i].score = entry.m_nScore;
				//DBG_LOG("%i: %s %i", entries[i].rank, entries[i].name.c_str(), entries[i].score);
			}
			else
			{
				entries[i].meta_data_count = 0;
				entries[i].name = "";
				entries[i].rank = 0;
				entries[i].score = 0;
			}
		}

		entry_count = params->m_cEntryCount;
		loaded = true;
	}
};

// ===================================================================
Steamworks_OnlineRankings::~Steamworks_OnlineRankings()
{
	for (std::vector<Steamworks_OnlineRankingsBoard*>::iterator iter = m_boards.begin(); iter != m_boards.end(); iter++)
	{
		SAFE_DELETE(*iter);
	}
	for (std::vector<Steamworks_OnlineRankingsTransaction*>::iterator iter = m_transactions.begin(); iter != m_transactions.end(); iter++)
	{
		SAFE_DELETE(*iter);
	}
	m_boards.clear();
	m_transactions.clear();
}

bool Steamworks_OnlineRankings::Initialize()
{
	return true;
}

void Steamworks_OnlineRankings::Tick(const FrameTime& time)
{
	// Purge finished transactions.
	for (std::vector<Steamworks_OnlineRankingsTransaction*>::iterator iter = m_transactions.begin(); iter != m_transactions.end(); )
	{
		Steamworks_OnlineRankingsTransaction* transaction = *iter;
		if (transaction->completed == true)
		{
			SAFE_DELETE(*iter);
			iter = m_transactions.erase(iter);
		}
		else
		{
			iter++;
		}
	}
}

void Steamworks_OnlineRankings::On_Find_Leaderboard(LeaderboardFindResult_t* params, bool bIOFailure)
{	
	if (!params->m_bLeaderboardFound || bIOFailure) 
	{
		DBG_LOG("[Steamworks] Failed to find or create leaderboard.");
		return;
	}

	const char* id = SteamUserStats()->GetLeaderboardName(params->m_hSteamLeaderboard);

	OnlineRankingsBoard* board = Get_Board(id, false, false);
	Steamworks_OnlineRankingsBoard* sw_board = dynamic_cast<Steamworks_OnlineRankingsBoard*>(board);
	DBG_ASSERT(sw_board != NULL);
	
	DBG_LOG("[Steamworks] Leaderboard '%s' was found or created.", id);
	sw_board->is_ready = true;
	sw_board->handle = params->m_hSteamLeaderboard;
}

void Steamworks_OnlineRankings::On_Uploaded_Entries(LeaderboardScoreUploaded_t* params, bool bIOFailure)
{
	const char* id = SteamUserStats()->GetLeaderboardName(params->m_hSteamLeaderboard);

	if (!params->m_bSuccess || bIOFailure) 
	{
		DBG_LOG("[Steamworks] Failed to upload entry for board '%s'.", id);
		return;
	}

	OnlineRankingsBoard* board = Get_Board(id, false, false);
	Steamworks_OnlineRankingsBoard* sw_board = dynamic_cast<Steamworks_OnlineRankingsBoard*>(board);
	DBG_ASSERT(sw_board != NULL);
	
	DBG_LOG("[Steamworks] Score '%i' for leaderboard '%s' was uploaded. New global rank of %i.", params->m_nScore, id, params->m_nGlobalRankNew);

	sw_board->current_rank  = params->m_nGlobalRankNew;
	sw_board->current_score = params->m_nScore;
}

OnlineRankingsBoard* Steamworks_OnlineRankings::Get_Board(const char* id, bool replace_score, bool ascending)
{
	for (std::vector<Steamworks_OnlineRankingsBoard*>::iterator iter = m_boards.begin(); iter != m_boards.end(); iter++)
	{
		Steamworks_OnlineRankingsBoard* sw_board = *iter;
		if (sw_board->id == id)
		{
			return sw_board;
		}
	}

	DBG_LOG("[Steamworks] Attempting to find or create leaderboard '%s'.", id);

	Steamworks_OnlineRankingsBoard* board = new Steamworks_OnlineRankingsBoard();
	board->id = id;
	board->replace_score = replace_score;
	board->is_ready = false;
	board->has_rank = false;
	board->find_call = SteamUserStats()->FindOrCreateLeaderboard(id, 
																 ascending ? k_ELeaderboardSortMethodAscending : k_ELeaderboardSortMethodDescending, 
																 k_ELeaderboardDisplayTypeNumeric);
	board->find_call_result.Set(board->find_call, this, &Steamworks_OnlineRankings::On_Find_Leaderboard);

	m_boards.push_back(board);

	return board;
}

bool Steamworks_OnlineRankings::Is_Board_Ready(OnlineRankingsBoard* board)
{
	Steamworks_OnlineRankingsBoard* sw_board = static_cast<Steamworks_OnlineRankingsBoard*>(board);
	return sw_board->is_ready;
}

void Steamworks_OnlineRankings::Submit(OnlineRankingsBoard* board, int score, void* meta_data, int meta_data_size)
{
	Steamworks_OnlineRankingsBoard* sw_board = static_cast<Steamworks_OnlineRankingsBoard*>(board);
	DBG_ASSERT(sw_board->is_ready);

	int number_of_ints = (int)ceil(meta_data_size / 4.0f);
	DBG_ASSERT(number_of_ints <= 64); // 64 is max steam supports.

	DBG_LOG("[Steamworks] Submitted new score of '%i' to board '%s'.", score, sw_board->id.c_str());

	Steamworks_OnlineRankingsSubmitTransaction* transaction = new Steamworks_OnlineRankingsSubmitTransaction();
	transaction->meta_data		= new int32[number_of_ints];
	transaction->meta_data_size = number_of_ints;
	memcpy(transaction->meta_data, meta_data, meta_data_size);
	transaction->call = SteamUserStats()->UploadLeaderboardScore(sw_board->handle, 
														 		 !sw_board->replace_score ? k_ELeaderboardUploadScoreMethodKeepBest : k_ELeaderboardUploadScoreMethodForceUpdate,
																 score,
																 transaction->meta_data,
																 transaction->meta_data_size);
	transaction->call_result.Set(transaction->call, this, &Steamworks_OnlineRankings::On_Uploaded_Entries);
	transaction->completed	= false;
	transaction->board		= sw_board;

	m_transactions.push_back(transaction);
}

OnlineRankingsEntries* Steamworks_OnlineRankings::Get_Rankings(OnlineRankingsBoard* board, int offset, int count, bool friends_only)
{
	Steamworks_OnlineRankingsBoard* sw_board = static_cast<Steamworks_OnlineRankingsBoard*>(board);
	DBG_ASSERT(sw_board->is_ready);

	DBG_LOG("[Steamworks] Requesting rankings (%i-%i) for board (%s) '%s'.", offset, offset + count, friends_only ? "Friends Only" : "Global", sw_board->id.c_str());

	Steamworks_OnlineRankingsEntries* entries = new Steamworks_OnlineRankingsEntries();
	entries->entries = new Steamworks_OnlineRankingsEntry[count];
	entries->loaded = false;
	entries->max_entries = count;

	int range_start = offset;
	int range_end   = (offset + count) - 1;
	
	Steamworks_OnlineRankingsGetRankingsTransaction* transaction = new Steamworks_OnlineRankingsGetRankingsTransaction();
	transaction->call		= SteamUserStats()->DownloadLeaderboardEntries(sw_board->handle, friends_only ? k_ELeaderboardDataRequestFriends : k_ELeaderboardDataRequestGlobal, range_start, range_end);
	transaction->call_result.Set(transaction->call, entries, &Steamworks_OnlineRankingsEntries::On_Downloaded_Entries);
	transaction->completed	= false;
	transaction->board		= sw_board;

	return entries;
}

OnlineRankingsEntries* Steamworks_OnlineRankings::Get_Own_Ranking(OnlineRankingsBoard* board)
{
	Steamworks_OnlineRankingsBoard* sw_board = static_cast<Steamworks_OnlineRankingsBoard*>(board);
	DBG_ASSERT(sw_board->is_ready);

	DBG_LOG("[Steamworks] Requesting own ranking for board '%s'.", sw_board->id.c_str());

	Steamworks_OnlineRankingsEntries* entries = new Steamworks_OnlineRankingsEntries();
	entries->entries = new Steamworks_OnlineRankingsEntry[1];
	entries->loaded = false;
	entries->max_entries = 1;

	Steamworks_OnlineRankingsGetRankingsTransaction* transaction = new Steamworks_OnlineRankingsGetRankingsTransaction();
	transaction->steam_id	= SteamUser()->GetSteamID();
	transaction->call		= SteamUserStats()->DownloadLeaderboardEntriesForUsers(sw_board->handle, &transaction->steam_id, 1);
	transaction->call_result.Set(transaction->call, entries, &Steamworks_OnlineRankingsEntries::On_Downloaded_Entries);
	transaction->completed	= false;
	transaction->board		= sw_board;

	return entries;
}