// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _KICK_VOTE_
#define _KICK_VOTE_

#include "Game/Game/VoteManager.h"

class KickVote : public Vote
{
	MEMORY_ALLOCATOR(KickVote, "Game");

public:
	KickVote();
	
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

