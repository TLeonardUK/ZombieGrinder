// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Online/OnlineUser.h"

OnlineUser::OnlineUser()
{
	static int id_tracker = 0;
	m_id = id_tracker++;
}

int OnlineUser::Get_ID()
{
	return m_id;
}