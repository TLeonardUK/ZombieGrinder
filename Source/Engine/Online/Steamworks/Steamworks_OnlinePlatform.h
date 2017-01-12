// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_STEAMWORKS_ONLINE_PLATFORM_
#define _ENGINE_STEAMWORKS_ONLINE_PLATFORM_

#include "Generic/Patterns/Singleton.h"

#include "Engine/Online/OnlinePlatform.h"
#include "Engine/Online/OnlineUser.h"

#include "public/steam/steam_api.h"
#include "public/steam/steam_gameserver.h"

#include <string>
#include <vector>

class Steamworks_OnlineUser;
class Steamworks_OnlineServer;
class Steamworks_OnlineClient;

class Steamworks_OnlinePlatform : public OnlinePlatform
{
	MEMORY_ALLOCATOR(Steamworks_OnlinePlatform, "Network");

private:
	int m_appid;

	bool m_overlay_opened;
	float m_last_dialog_open_time;

	bool m_started_tracking_server_time;
	double m_server_time;

	Steamworks_OnlineServer* m_server;
	Steamworks_OnlineClient* m_client;
	
	Steamworks_OnlineUser*				m_initial_user;
	std::vector<Steamworks_OnlineUser*>	m_users;

	STEAM_CALLBACK(Steamworks_OnlinePlatform, Callback_PersonaStateChange, PersonaStateChange_t, m_persona_callback);
	STEAM_CALLBACK(Steamworks_OnlinePlatform, Callback_OverlayChange, GameOverlayActivated_t, m_overlay_callback);
	
protected:
	enum
	{
		DIALOG_OPEN_FAKE_DELAY = 1000,
	};

public:

	// Saves some type-casting.
	static Steamworks_OnlinePlatform* Get()
	{
		return static_cast<Steamworks_OnlinePlatform*>(OnlinePlatform::Get());
	}
	
	Steamworks_OnlinePlatform();
	~Steamworks_OnlinePlatform();
	
	void Set_AppID(int appid);
	int  Get_AppID();

	bool Restart_Required();

	bool Is_Online();

	bool Init_Client();
	bool Destroy_Client();

	bool Init_Server();
	bool Destroy_Server();

	OnlineClient* Client();
	OnlineServer* Server();
	
	void Tick(const FrameTime& time);

	// Time.
	u64 GetServerTime();

	// DLC management.	
	bool Check_Purchased(int dlc_id);
	void Show_Purchase_Dialog(int dlc_id);
	bool Is_Dialog_Open();
	void Show_Gamepad_Text_Dialog(std::string description, int max_length);
	bool Get_Gamepad_Text_Dialog_Result(std::string& result);
	void Show_Mod_Gallery();
	void Show_Web_Browser(const char* url);
	
	// User management.
	OnlineUser* Get_Initial_User();
	OnlineUser* Get_Local_User_By_Index(int input_source);
	OnlineUser* Login_Local_User(int input_source);
	void Logout_Local_User(int input_source);

	int Get_Local_User_Count();
	std::vector<OnlineUser*> Get_Local_Users();
	std::vector<OnlineUser*> Get_Users();

	Steamworks_OnlineUser* Get_User_By_SteamID(CSteamID id);
	Steamworks_OnlineUser* Register_Remote_User(CSteamID id);
	
	OnlineUser* Get_User_By_ID(int id);

};

#endif

