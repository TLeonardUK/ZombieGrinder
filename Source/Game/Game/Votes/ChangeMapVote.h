// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _CHANGE_MAP_VOTE_
#define _CHANGE_MAP_VOTE_

#include "Game/Game/VoteManager.h"

class ChangeMapVote : public Vote
{
	MEMORY_ALLOCATOR(ChangeMapVote, "Game");

public:
	ChangeMapVote();
	
	int			Get_Type();
	std::string Get_Name();
	std::string Get_Description();
	int			Get_Timeout_Period();
	Vote*		Clone();
	
	bool		Is_Valid();

	void		Pass();
	void		Fail();
};

#endif

