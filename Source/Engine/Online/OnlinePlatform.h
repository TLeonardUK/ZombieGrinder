// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_ONLINE_PLATFORM_
#define _ENGINE_ONLINE_PLATFORM_

#include "Generic/Patterns/Singleton.h"

#include "Engine/Engine/FrameTime.h"

#include <string>
#include <vector>

class OnlineUser;
class OnlineClient;
class OnlineServer;

#define MAX_LOCAL_PLAYERS 4

class OnlinePlatform : public Singleton<OnlinePlatform>
{
	MEMORY_ALLOCATOR(OnlinePlatform, "Network");

private:

public:
	virtual ~OnlinePlatform() {} 

	static OnlinePlatform* Create();

	// Returns true if we need to restart because the online platform isn't running or something silly :).
	virtual bool Restart_Required() = 0;

	// Returns true if we are online and can use online functionality.
	virtual bool Is_Online() = 0;

	// Initializes client-side online platform.
	virtual bool Init_Client() = 0;
	virtual bool Destroy_Client() = 0;

	// Initializes server-side online platform.
	virtual bool Init_Server() = 0;
	virtual bool Destroy_Server() = 0;

	// Retrieves online platform client.
	virtual OnlineClient* Client() = 0;

	// Retrieves online platform client.
	virtual OnlineServer* Server() = 0;

	// Tick tock tick tock
	virtual void Tick(const FrameTime& time) = 0;

	// Time.
	virtual u64 GetServerTime() = 0;

	// DLC management.
	virtual bool Check_Purchased(int dlc_id) = 0;

	// TODO: This is a dickbutt way of doing this, wrap in a Platform_Dialog style interface.
	virtual void Show_Purchase_Dialog(int dlc_id) = 0;
	virtual bool Is_Dialog_Open() = 0;
	virtual void Show_Gamepad_Text_Dialog(std::string description, int max_length) = 0;
	virtual bool Get_Gamepad_Text_Dialog_Result(std::string& result) = 0;
	
	// Mods
	virtual void Show_Mod_Gallery() = 0;
	virtual void Show_Web_Browser(const char* url) = 0;

	// User management.
	virtual OnlineUser* Get_Initial_User() = 0;
	virtual OnlineUser* Get_Local_User_By_Index(int input_source) = 0;
	virtual OnlineUser* Get_User_By_ID(int id) = 0;

	virtual OnlineUser* Login_Local_User(int input_source) = 0;
	virtual void Logout_Local_User(int input_source) = 0;
	virtual int Get_Local_User_Count() = 0;

	virtual std::vector<OnlineUser*> Get_Local_Users() = 0;
	virtual std::vector<OnlineUser*> Get_Users() = 0;

};

#endif

