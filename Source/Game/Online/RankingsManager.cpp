// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Online/RankingsManager.h"
#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMBinary.h"

#include "Engine/Online/OnlineUser.h"
#include "Engine/Online/OnlinePlatform.h"
#include "Engine/Online/OnlineRankings.h"

#include "Engine/Engine/GameEngine.h"
#include "Engine/Scripts/EngineVirtualMachine.h"

#include "Engine/Resources/ResourceFactory.h"
#include "Engine/Scene/Map/MapFile.h"
#include "Engine/Scene/Map/MapFileHandle.h"

#include "Engine/Localise/Locale.h"

#include "Engine/Engine/EngineOptions.h"

#include "Game/Runner/Game.h"

#include "Game/Scripts/ScriptEventListener.h"

RankingsManager::RankingsManager()
	: m_init(false)
{
}

RankingsManager::~RankingsManager()
{
}

bool RankingsManager::Init()
{	
	DBG_ASSERT(!m_init);
	DBG_LOG("Loading rankings ...");

	EngineVirtualMachine* vm = GameEngine::Get()->Get_VM();
	CVMLinkedSymbol* base_symbol = vm->Find_Class("Leaderboard");
	DBG_ASSERT(base_symbol != NULL);

	std::vector<CVMLinkedSymbol*> extended_classes = vm->Find_Derived_Classes(base_symbol);

	CVMContextLock lock = vm->Set_Context(vm->Get_Static_Context());

	int index = 0;
	for (std::vector<CVMLinkedSymbol*>::iterator iter = extended_classes.begin(); iter != extended_classes.end(); iter++, index++)
	{
		CVMLinkedSymbol* leaderboard_class = *iter;
		DBG_LOG("Loading Leaderboard Class: %s", leaderboard_class->symbol->name);

		Leaderboard* leaderboard = new Leaderboard();
		m_boards.push_back(leaderboard);

		leaderboard->script_object = vm->New_Object(leaderboard_class, true, leaderboard);
		vm->Set_Default_State(leaderboard->script_object);

		leaderboard->online_rankings_board = OnlineRankings::Get()->Get_Board(leaderboard->id.c_str(), leaderboard->replace_scores, leaderboard->ascending);
		leaderboard->total_entries = -1;
		leaderboard->highest_entry = 0;
	}

	// Create leaderboards for each map.
	ThreadSafeHashTable<MapFileHandle*, unsigned int>& files = ResourceFactory::Get()->Get_Map_Files();
	for (ThreadSafeHashTable<MapFileHandle*, unsigned int>::Iterator iter = files.Begin(); iter != files.End(); iter++)
	{
		MapFileHandle* handle = *iter;
		MapFile* file = handle->Get();

		// We don't care about internal map files.
		if (file->Is_Internal())
		{
			continue;
		}

		// TODO: Fix this shit. 
		// Skip PVP
		if (file->Is_Mod() ||
			file->Get_Header()->Short_Name.substr(0, 3) == "dm_" ||
			file->Get_Header()->Short_Name.substr(0, 4) == "ctb_" ||
			file->Get_Header()->Short_Name.substr(0, 3) == "gg_" ||
			file->Get_Header()->Short_Name.substr(0, 3) == "ru_" ||
			file->Get_Header()->Short_Name.substr(0, 4) == "tdm_")
		{
			continue;
		}

		Generate_Map_Leaderboard(file->Get_Header()->GUID);
	}

	m_init = true;
	return true;
}

Leaderboard* RankingsManager::Generate_Map_Leaderboard(std::string guid)
{
	std::string long_name = guid;

	MapFileHandle* handle = ResourceFactory::Get()->Get_Map_File_By_GUID(guid.c_str());
	if (handle != NULL)
	{
		long_name = handle->Get()->Get_Header()->Long_Name;
		guid = handle->Get()->Get_Header()->Short_Name;
	}

	Leaderboard* board = new Leaderboard();
	board->id					= StringHelper::Format("LEADERBOARD_MAP_SCORE_%s", guid.c_str());
	board->name					= SF("#leaderboad_auto_map", long_name.c_str());
	board->replace_scores		= false;
	board->ascending			= false;
	board->is_internal			= false;

	DBG_LOG("Auto generating map leaderboard: %s", guid.c_str());

	board->script_object			= NULL;
	board->online_rankings_board = OnlineRankings::Get()->Get_Board(board->id.c_str(), board->replace_scores, board->ascending);
	board->total_entries			= -1;
	board->highest_entry			= 0;

	m_boards.push_back(board);

	return board;
}

void RankingsManager::Tick(const FrameTime& time)
{
	if (!m_init)
		return;

	OnlinePlatform* platform = OnlinePlatform::Get();
	OnlineRankings* rankings = OnlineRankings::Get();

	// Submit transactions if their boards are ready.
	for (std::vector<LeaderboardTransaction>::iterator iter = m_transactions.begin(); iter != m_transactions.end(); )
	{
		LeaderboardTransaction& transaction = *iter;
		if (rankings->Is_Board_Ready(transaction.board->online_rankings_board))
		{
			bool complete = false;

			switch (transaction.type)
			{
			case LeaderboardTransactionType::Submit:
				{
					// Restricted mode prevents achievements.
					if (!Game::Get()->In_Restricted_Mode())
					{
						Ranking_Metadata data;
						data.country_id = platform->Get_Initial_User()->Get_GeoIP_Result().CountryID;
						data.real_score = transaction.real_score;
						rankings->Submit(transaction.board->online_rankings_board, transaction.score, &data, sizeof(Ranking_Metadata));
					}
					complete = true;
					break;
				}
			case LeaderboardTransactionType::Refresh_Range:
				{
					if (transaction.entries == NULL)
					{
						transaction.entries = rankings->Get_Rankings(transaction.board->online_rankings_board, transaction.offset, transaction.count, transaction.friends_only);		
					}
					else if (transaction.entries->Has_Loaded())
					{
						for (int i = 0; i < transaction.entries->Entry_Count(); i++)
						{
							OnlineRankingsEntry* entry = transaction.entries->Get_Entry(i);

							int rank = entry->Get_Rank();

							LeaderboardEntry* real_entry = NULL;
							if (rank >= 0 && transaction.board->entries.Get_Ptr(rank, real_entry))
							{
								Ranking_Metadata meta;
								memset(&meta, 0, sizeof(meta));
								entry->Get_Meta_Data(&meta, NULL, sizeof(Ranking_Metadata));

								real_entry->name = entry->Get_Name();
								real_entry->rank = rank;
								real_entry->score = entry->Get_Score();
								real_entry->real_score = meta.real_score;
								real_entry->country = meta.country_id;
								real_entry->status = LeaderboardEntryStatus::Loaded;

								transaction.board->highest_entry = Max(transaction.board->highest_entry, entry->Get_Rank());
							}
						}
						if (transaction.entries->Entry_Count() < transaction.count)
						{
							transaction.board->total_entries = (transaction.offset - 1) + transaction.entries->Entry_Count();
							transaction.board->highest_entry = transaction.board->total_entries;
						}
						complete = true;
					}
					break;
				}
			case LeaderboardTransactionType::Refresh_Own_Entry:
				{
					if (transaction.entries == NULL)
					{
						transaction.entries = rankings->Get_Own_Ranking(transaction.board->online_rankings_board);
					}
					else if (transaction.entries->Has_Loaded())
					{
						if (transaction.entries->Entry_Count() == 0)
						{
							transaction.board->own_entry.rank = -1;
						}
						else
						{
							OnlineRankingsEntry* entry = transaction.entries->Get_Entry(0);

							Ranking_Metadata meta;
							memset(&meta, 0, sizeof(meta));
							entry->Get_Meta_Data(&meta, NULL, sizeof(Ranking_Metadata));

							transaction.board->own_entry.name = entry->Get_Name();
							transaction.board->own_entry.rank = entry->Get_Rank();
							transaction.board->own_entry.score = entry->Get_Score();
							transaction.board->own_entry.real_score = meta.real_score;
							transaction.board->own_entry.country = meta.country_id;

							transaction.board->highest_entry = Max(transaction.board->highest_entry, entry->Get_Rank());

							ScriptEventListener::Fire_On_Rank_Updated(transaction.board->highest_entry);
						}
												
						transaction.board->own_entry.status = LeaderboardEntryStatus::Loaded;
						complete = true;
					}
					break;
				}
			}

			if (complete == false)
				iter++;
			else
				iter = m_transactions.erase(iter);
		}
		else
		{
			iter++;
		}
	}
}

bool RankingsManager::Serialize(BinaryStream* stream)
{
	DBG_ASSERT(m_init);

	// We could save best known high score, but do we really care?

	return true;
}

bool RankingsManager::Deserialize(BinaryStream* stream)
{
	// Read above comment.

	return true;
}

std::vector<Leaderboard*> RankingsManager::Get_Leaderboards()
{
	std::vector<Leaderboard*> boards;

	for (std::vector<Leaderboard*>::iterator iter = m_boards.begin(); iter != m_boards.end(); iter++)
	{
		boards.push_back((*iter));
	}

	return boards;
}

std::vector<Leaderboard*> RankingsManager::Get_Non_Internal_Leaderboards()
{
	std::vector<Leaderboard*> boards;

	for (std::vector<Leaderboard*>::iterator iter = m_boards.begin(); iter != m_boards.end(); iter++)
	{
		Leaderboard* b = *iter;
		if (!b->is_internal)
		{
			boards.push_back(b);
		}
	}

	return boards;
}


Leaderboard* RankingsManager::Get_Leaderboard(const char* name)
{
	for (std::vector<Leaderboard*>::iterator iter = m_boards.begin(); iter != m_boards.end(); iter++)
	{
		Leaderboard* board = *iter;
		if (board->id == name)
		{
			return board;
		}
	}
	return NULL;
}

Leaderboard* RankingsManager::Get_Map_Leaderboard(const char* map_name)
{
	std::string target_id = StringHelper::Format("LEADERBOARD_MAP_SCORE_%s", map_name);

	for (std::vector<Leaderboard*>::iterator iter = m_boards.begin(); iter != m_boards.end(); iter++)
	{
		Leaderboard* board = *iter;
		if (board->id == target_id)
		{
			return board;
		}
	}

	return Generate_Map_Leaderboard(map_name);
}

Leaderboard* RankingsManager::Get_Or_Create_Leaderboard(const char* name)
{
	Leaderboard* board = Get_Leaderboard(name);
	if (!board)
	{
		board = new Leaderboard();
		board->id = name;
		board->name = "Challenge Board";
		board->replace_scores = true;
		board->ascending = false;
		board->is_internal = true;

		DBG_LOG("Creating leaderboard: %s", name);

		board->script_object = NULL;
		board->online_rankings_board = OnlineRankings::Get()->Get_Board(board->id.c_str(), board->replace_scores, board->ascending);
		board->total_entries = -1;
		board->highest_entry = 0;

		m_boards.push_back(board);
	}
	return board;
}

void RankingsManager::Submit(Leaderboard* board, int value, int real_score)
{
	LeaderboardTransaction transaction;
	transaction.type  = LeaderboardTransactionType::Submit;
	transaction.board = board;
	transaction.score = value;
	transaction.real_score = real_score;
	transaction.entries = NULL;

	m_transactions.push_back(transaction);
}

void RankingsManager::Clear_Board(Leaderboard* board)
{
	board->entries.Clear();
	board->own_entry.status = LeaderboardEntryStatus::Unloaded;
	board->total_entries = -1;
}

void RankingsManager::Refresh_Entries(Leaderboard* board, int offset, int count, bool friends_only)
{
	LeaderboardTransaction transaction;
	transaction.type  = LeaderboardTransactionType::Refresh_Range;
	transaction.board = board;
	transaction.offset = offset;
	transaction.count  = count;
	transaction.friends_only = friends_only;
	transaction.entries = NULL;

	for (int i = offset; i < offset + count; i++)
	{
		if (!board->entries.Contains(i))
		{
			LeaderboardEntry entry;
			entry.status = LeaderboardEntryStatus::Loading;

			board->entries.Set(i, entry);
		}
		else
		{
			LeaderboardEntry* entry;
			board->entries.Get_Ptr(i, entry);

			if (entry->status == LeaderboardEntryStatus::Unloaded)
			{
				entry->status = LeaderboardEntryStatus::Loading;
			}
		}
	}
	
	transaction.board->highest_entry = Max(transaction.board->highest_entry, offset + count);

	m_transactions.push_back(transaction);
}

LeaderboardEntry* RankingsManager::Get_Own_Entry(Leaderboard* board)
{
	if (board->own_entry.status == LeaderboardEntryStatus::Unloaded)
	{
		LeaderboardTransaction transaction;
		transaction.type  = LeaderboardTransactionType::Refresh_Own_Entry;
		transaction.board = board;
		transaction.entries = NULL;

		m_transactions.push_back(transaction);

		board->own_entry.status = LeaderboardEntryStatus::Loading;
	}
	return &board->own_entry;
}

LeaderboardEntry* RankingsManager::Get_Board_Entry(Leaderboard* board, int rank)
{
	if (!board->entries.Contains(rank))
	{
		LeaderboardEntry entry;
		entry.status = LeaderboardEntryStatus::Unloaded;

		board->entries.Set(rank, entry);
	}
	
	LeaderboardEntry* entry = NULL;
	board->entries.Get_Ptr(rank, entry);

	return entry;
}
