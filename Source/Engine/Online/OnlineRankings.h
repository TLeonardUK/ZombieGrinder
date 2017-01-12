// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_ONLINE_RANKINGS_
#define _ENGINE_ONLINE_RANKINGS_

#include "Generic/Patterns/Singleton.h"

#include "Engine/Engine/FrameTime.h"

#include <string>

class OnlineRankingsBoard
{
public:
	virtual ~OnlineRankingsBoard() { }
};

class OnlineRankingsEntry
{
public:
	virtual std::string Get_Name() = 0;
	virtual int Get_Score() = 0;
	virtual int Get_Rank() = 0;
	virtual void Get_Meta_Data(void* data, int* size, int max_length) = 0;
};

class OnlineRankingsEntries
{
public:
	virtual ~OnlineRankingsEntries() { }
	virtual bool				 Has_Loaded() = 0;
	virtual int					 Entry_Count() = 0;
	virtual OnlineRankingsEntry* Get_Entry(int index) = 0;
};

class OnlineRankings : public Singleton<OnlineRankings>
{
	MEMORY_ALLOCATOR(OnlineRankings, "Network");

private:

public:

public:
	virtual ~OnlineRankings() {} 

	static OnlineRankings* Create();
	
	// Connects to online platform.
	virtual bool Initialize() = 0;

	// Tick tock tick tock
	virtual void Tick(const FrameTime& time) = 0;

	// Gets a board that will be used for rankings. Board will be created if dosen't already exist (if possible).
	virtual OnlineRankingsBoard* Get_Board(const char* id, bool replace_score, bool ascending) = 0;

	// Returns true if the board is ready for uploads.
	virtual bool Is_Board_Ready(OnlineRankingsBoard* board) = 0;

	// Submit rankings.
	virtual void Submit(OnlineRankingsBoard* board, int score, void* meta_data, int meta_data_size) = 0;

	// Get rankings.
	virtual OnlineRankingsEntries* Get_Rankings(OnlineRankingsBoard* board, int offset, int count, bool friends_only = false) = 0;
	virtual OnlineRankingsEntries* Get_Own_Ranking(OnlineRankingsBoard* board) = 0;

};

#endif

