// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _RESTART_VOTE_
#define _RESTART_VOTE_

#include "Game/Game/VoteManager.h"

class RestartVote : public Vote
{
	MEMORY_ALLOCATOR(RestartVote, "Game");

public:
	RestartVote();
	
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

