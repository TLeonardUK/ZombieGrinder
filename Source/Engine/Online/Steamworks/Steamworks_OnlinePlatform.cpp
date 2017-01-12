// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Online/Steamworks/Steamworks_OnlinePlatform.h"
#include "Engine/Online/Steamworks/Steamworks_OnlineMatching.h"
#include "Engine/Online/Steamworks/Steamworks_OnlineUser.h"
#include "Engine/Online/Steamworks/Steamworks_OnlineServer.h"
#include "Engine/Online/Steamworks/Steamworks_OnlineClient.h"

#include "Engine/Profiling/ProfilingManager.h"

#include "Engine/Engine/EngineOptions.h"

#include "Engine/Version.h"

//#define OPT_FAST_CYCLE_SERVER_TIME

extern "C" void __cdecl SteamAPI_DebugCallback(int nSeverity, const char *pchDebugText)
{
	DBG_LOG("[Steam Server] %s", pchDebugText);
}

Steamworks_OnlinePlatform::Steamworks_OnlinePlatform()
	: m_client(NULL)
	, m_server(NULL)
	, m_persona_callback(this, &Steamworks_OnlinePlatform::Callback_PersonaStateChange)
	, m_overlay_callback(this, &Steamworks_OnlinePlatform::Callback_OverlayChange)
	, m_initial_user(NULL)
	, m_overlay_opened(false)
	, m_last_dialog_open_time(0.0f)
	, m_started_tracking_server_time(false)
{
} 

Steamworks_OnlinePlatform::~Steamworks_OnlinePlatform()
{
	DBG_LOG("Disposing of online platform...");

	SAFE_DELETE(m_server);
	SAFE_DELETE(m_client);

	for (std::vector<Steamworks_OnlineUser*>::iterator iter = m_users.begin(); iter != m_users.end(); iter++)
	{
		SAFE_DELETE(*iter);
	}

	m_users.clear();
	m_initial_user = NULL;
}

bool Steamworks_OnlinePlatform::Restart_Required()
{
#ifdef OPT_SUPRESS_STEAM_RESTART
	return false;
#else
	return SteamAPI_RestartAppIfNecessary(m_appid);
#endif
}

u64 Steamworks_OnlinePlatform::GetServerTime()
{
#ifdef OPT_FAST_CYCLE_SERVER_TIME
	static u64 pst_time = (u64)SteamUtils()->GetServerRealTime();
	pst_time += 3 * 30;
	return pst_time;
#else
	if (!m_started_tracking_server_time)
	{
		m_server_time = (double)SteamUtils()->GetServerRealTime();
		m_started_tracking_server_time = true;
	}

	u64 pst_time = (u64)m_server_time;
	return pst_time;// - (8 * 60 * 60); // -8hr to get GMT
#endif
}

bool Steamworks_OnlinePlatform::Is_Online()
{
	return SteamUser() ? SteamUser()->BLoggedOn() : true;
}

bool Steamworks_OnlinePlatform::Init_Server()
{
	DBG_ASSERT(m_server == NULL);
	
	m_server = new Steamworks_OnlineServer(this);
	if (!m_server->Init())
	{
		SAFE_DELETE(m_server);
		return false;
	}

	return true;
}

bool Steamworks_OnlinePlatform::Init_Client()
{
	DBG_ASSERT(m_client == NULL);
	
	m_client = new Steamworks_OnlineClient(this);
	if (!m_client->Init())
	{
		SAFE_DELETE(m_client);
		return false;
	}

	// Login initial user! We can do this now!
	if (m_initial_user == NULL)
	{
		m_initial_user = new Steamworks_OnlineUser(SteamUser()->GetSteamID(), 0, true);
		m_users.push_back(m_initial_user);

		// Some info please!
		DBG_LOG("Logged in as steam user %s", m_initial_user->Get_Username().c_str());
	}

	return true;
}

bool Steamworks_OnlinePlatform::Destroy_Server()
{
	if (m_server == NULL)
	{
		return true;
	}
	
	SAFE_DELETE(m_server);

	return true;
}

bool Steamworks_OnlinePlatform::Destroy_Client()
{
	DBG_ASSERT(m_client != NULL);
	
	SAFE_DELETE(m_client);

	return true;
}

void Steamworks_OnlinePlatform::Tick(const FrameTime& time)
{
	// Tick all users.
	{
		PROFILE_SCOPE("User Tick");

		std::vector<Steamworks_OnlineUser*> users = m_users;
		for (std::vector<Steamworks_OnlineUser*>::iterator iter = users.begin(); iter != users.end(); iter++)
		{
			Steamworks_OnlineUser* user = *iter;
			user->Tick(time);
		}
	}

	{
		PROFILE_SCOPE("Client Tick");
		if (m_client != NULL)
			m_client->Tick(time);
	}
	{
		PROFILE_SCOPE("Server Tick");		
		if (m_server != NULL)
			m_server->Tick(time);
	}

	if (m_started_tracking_server_time)
	{
		m_server_time += (double)time.Get_Delta_Seconds();
	}

	if (!m_overlay_opened)
	{
		m_last_dialog_open_time += time.Get_Frame_Time();
	}
}

bool Steamworks_OnlinePlatform::Check_Purchased(int dlc_id)
{
#ifdef OPT_ASSUME_NO_DLC
	return false;
#endif

	// No client? No DLC.
	if (m_client == NULL)
	{
		return false;
	}
	else
	{
		static int g_cache_id = 0;
		static bool g_cache_result = false;

		if (dlc_id != g_cache_id)
		{
			g_cache_result = SteamApps()->BIsDlcInstalled(dlc_id);
			g_cache_id = dlc_id;
		}

		// TODO: Cache result?
		//DBG_LOG("Checking purchase of dlc appid '%i', %s", dlc_id, result ? "SUCCESS" : "FAILED"); 
		return g_cache_result;
	}
}

void Steamworks_OnlinePlatform::Show_Purchase_Dialog(int dlc_id)
{
	DBG_LOG("Showing purchase dialog for dlc appid '%i'", dlc_id);
	SteamFriends()->ActivateGameOverlayToStore(dlc_id, k_EOverlayToStoreFlag_AddToCartAndShow);
	m_last_dialog_open_time = 0.0f;
}

bool Steamworks_OnlinePlatform::Is_Dialog_Open()
{
	return m_overlay_opened || (m_last_dialog_open_time < DIALOG_OPEN_FAKE_DELAY);
}

void Steamworks_OnlinePlatform::Show_Mod_Gallery()
{
	DBG_LOG("Showing workshop page.");
	SteamFriends()->ActivateGameOverlayToWebPage(StringHelper::Format("http://steamcommunity.com/app/%i/workshop/", m_appid).c_str());
	m_last_dialog_open_time = 0.0f;	
}

void Steamworks_OnlinePlatform::Show_Web_Browser(const char* url)
{
	DBG_LOG("Showing web page: %s", url);
	SteamFriends()->ActivateGameOverlayToWebPage(url);
	m_last_dialog_open_time = 0.0f;	
}

void Steamworks_OnlinePlatform::Show_Gamepad_Text_Dialog(std::string description, int max_length)
{
	// TODO
//	bool result = SteamUtils()->ShowGamepadTextInput(k_EGamepadTextInputModeNormal, k_EGamepadTextInputLineModeSingleLine, description.c_str(), max_length);
//	DBG_ASSERT(result == true);

	// Returns previously entered text & length
	//virtual uint32 GetEnteredGamepadTextLength() = 0;
	//virtual bool GetEnteredGamepadTextInput( char *pchText, uint32 cchText ) = 0;	

//	m_last_dialog_open_time = 0.0f;
}

bool Steamworks_OnlinePlatform::Get_Gamepad_Text_Dialog_Result(std::string& result)
{
	// TODO
	result = "";
	return false;
}

void Steamworks_OnlinePlatform::Set_AppID(int appid)
{
	m_appid = appid;
}

int Steamworks_OnlinePlatform::Get_AppID()
{
	return m_appid;
}

OnlineClient* Steamworks_OnlinePlatform::Client()
{
	return m_client;
}

OnlineServer* Steamworks_OnlinePlatform::Server()
{
	return m_server;
}

OnlineUser* Steamworks_OnlinePlatform::Get_User_By_ID(int id)
{
	for (std::vector<Steamworks_OnlineUser*>::iterator iter = m_users.begin(); iter != m_users.end(); iter++)
	{
		Steamworks_OnlineUser* user = *iter;
		if (user->Get_ID() == id)
		{
			return user;
		}
	}

	return NULL;
}

void Steamworks_OnlinePlatform::Callback_OverlayChange(GameOverlayActivated_t* pParam)
{
	if (pParam->m_bActive)
	{
		DBG_LOG("Steam overlay activated.");
	}
	else
	{
		DBG_LOG("Steam overlay closed.");
	}

	m_last_dialog_open_time = 0.0f;
	m_overlay_opened = (pParam->m_bActive != 0);
}

void Steamworks_OnlinePlatform::Callback_PersonaStateChange(PersonaStateChange_t* pParam)
{
	for (std::vector<Steamworks_OnlineUser*>::iterator iter = m_users.begin(); iter != m_users.end(); iter++)
	{
		Steamworks_OnlineUser* user = *iter;
		if (user->Get_SteamID().ConvertToUint64() == pParam->m_ulSteamID)
		{
			user->Update_Persona();

			DBG_LOG("Steam persona changed for %s.", user->Get_Username().c_str());
		}
	}
}

OnlineUser* Steamworks_OnlinePlatform::Get_Initial_User()
{
	return m_initial_user;
}

OnlineUser* Steamworks_OnlinePlatform::Get_Local_User_By_Index(int input_source)
{
	for (std::vector<Steamworks_OnlineUser*>::iterator iter = m_users.begin(); iter != m_users.end(); iter++)
	{
		Steamworks_OnlineUser* user = *iter;
		if (user->Is_Local() == true && user->Get_Local_Player_Index() == input_source)
		{
			return user;
		}
	}

	return NULL;
}

OnlineUser* Steamworks_OnlinePlatform::Login_Local_User(int input_source)
{
	// Already exists?
	for (std::vector<Steamworks_OnlineUser*>::iterator iter = m_users.begin(); iter != m_users.end(); iter++)
	{
		Steamworks_OnlineUser* user = *iter;
		if (user->Is_Local() == true && user->Get_Local_Player_Index() == input_source)
		{
			return user;
		}
	}

	DBG_LOG("Logged in local user index %i", input_source);

	// Create new user!
	Steamworks_OnlineUser* user = new Steamworks_OnlineUser(SteamUser()->GetSteamID(), input_source, true);
	m_users.push_back(user);

	return user;
}

void Steamworks_OnlinePlatform::Logout_Local_User(int input_source)
{
	for (std::vector<Steamworks_OnlineUser*>::iterator iter = m_users.begin(); iter != m_users.end(); iter++)
	{
		Steamworks_OnlineUser* user = *iter;
		if (user->Is_Local() == true && user->Get_Local_Player_Index() == input_source)
		{
			DBG_LOG("Logged out local user index %i", input_source);
			
			SAFE_DELETE(*iter);
			m_users.erase(iter);
			return;
		}
	}
}

int Steamworks_OnlinePlatform::Get_Local_User_Count()
{
	int count = 0;

	for (std::vector<Steamworks_OnlineUser*>::iterator iter = m_users.begin(); iter != m_users.end(); iter++)
	{
		if ((*iter)->Is_Local())
		{
			count++;
		}
	}

	return count;
}

std::vector<OnlineUser*> Steamworks_OnlinePlatform::Get_Local_Users()
{
	std::vector<OnlineUser*> users;

	for (std::vector<Steamworks_OnlineUser*>::iterator iter = m_users.begin(); iter != m_users.end(); iter++)
	{
		Steamworks_OnlineUser* user = *iter;
		if (user->Is_Local())
		{
			users.push_back(user);
		}
	}

	return users;
}

std::vector<OnlineUser*> Steamworks_OnlinePlatform::Get_Users()
{
	std::vector<OnlineUser*> users;

	for (std::vector<Steamworks_OnlineUser*>::iterator iter = m_users.begin(); iter != m_users.end(); iter++)
	{
		Steamworks_OnlineUser* user = *iter;
		users.push_back(*iter);
	}

	return users;
}

Steamworks_OnlineUser* Steamworks_OnlinePlatform::Register_Remote_User(CSteamID id)
{	
	for (std::vector<Steamworks_OnlineUser*>::iterator iter = m_users.begin(); iter != m_users.end(); iter++)
	{
		Steamworks_OnlineUser* user = *iter;
		DBG_ASSERT(id != user->Get_SteamID());
	}

	Steamworks_OnlineUser* user = new Steamworks_OnlineUser(id, 0, false);
	m_users.push_back(user);

	DBG_LOG("New remote user registered '%s'.", user->Get_Username().c_str());

	return user;
}

Steamworks_OnlineUser* Steamworks_OnlinePlatform::Get_User_By_SteamID(CSteamID id)
{
	for (std::vector<Steamworks_OnlineUser*>::iterator iter = m_users.begin(); iter != m_users.end(); iter++)
	{
		Steamworks_OnlineUser* user = *iter;
		if (user->Get_SteamID() == id)
		{
			return user;
		}
	}

	return NULL;
}
