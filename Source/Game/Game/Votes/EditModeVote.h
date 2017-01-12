// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _EDIT_MODE_VOTE_
#define _EDIT_MODE_VOTE_

#include "Game/Game/VoteManager.h"

class EditModeVote : public Vote
{
	MEMORY_ALLOCATOR(EditModeVote, "Game");

public:
	EditModeVote();
	
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

