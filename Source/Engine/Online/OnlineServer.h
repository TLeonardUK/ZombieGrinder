// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_ONLINE_SERVER_
#define _ENGINE_ONLINE_SERVER_

#include "Generic/Patterns/Singleton.h"

#include "Engine/Engine/FrameTime.h"

#include <string>
#include <vector>

class OnlineUser;

struct OnlineServer_State
{
	enum Type
	{
		Idle,
		Initializing,
		Securing,

		Error,
		
		Active,
		
		COUNT
	};
};

struct OnlineServer_Error
{
	enum Type
	{
		NONE,
		Invalid_State,
		Offline,
		
		Unknown
	};
};

class OnlineServer 
{
	MEMORY_ALLOCATOR(OnlineServer, "Network");

private:

public:
	virtual void Tick(const FrameTime& time) = 0;
	
	// State settings.
	virtual OnlineServer_State::Type Get_State() = 0;
	virtual OnlineServer_Error::Type Get_Error() = 0;
	virtual std::string Get_Error_Message(OnlineServer_Error::Type error) = 0;
	
	virtual void Set_Server_Info(int max_players, const char* map_name, bool private_game) = 0;

	// Grab server information.
	virtual std::string Get_Server_Name() = 0;
	virtual std::string Get_Host_Name() = 0;
	virtual void Flag_Server_Info_Dirty() = 0;

};

#endif

