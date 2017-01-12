// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Online/Steamworks/Steamworks_OnlinePlatform.h"
#include "Engine/Online/Steamworks/Steamworks_OnlineMatching.h"
#include "Engine/Online/Steamworks/Steamworks_OnlineUser.h"
#include "Engine/Online/Steamworks/Steamworks_OnlineClient.h"

#include "Engine/Input/Input.h"

#include "Engine/Network/NetManager.h"
#include "Engine/Network/Packets/EnginePackets.h"

#include "Engine/Engine/EngineOptions.h"

#include "Engine/Version.h"

extern "C" void __cdecl SteamAPI_ClientDebugCallback(int nSeverity, const char *pchDebugText)
{
	DBG_LOG("[Steam Client] %s", pchDebugText);
}

Steamworks_OnlineClient::Steamworks_OnlineClient(Steamworks_OnlinePlatform* platform)
	: m_platform(platform)
	, m_initialised(false)
	, m_old_client_state(NetClient_State::Idle)
{
} 

Steamworks_OnlineClient::~Steamworks_OnlineClient()
{
	DBG_LOG("Disposing of online client ...");

	if (m_initialised)
	{
		DBG_LOG("Terminating online client.");

		SteamClient()->SetWarningMessageHook(NULL);
		SteamAPI_Shutdown();
	}
}

bool Steamworks_OnlineClient::Init()
{
	DBG_LOG("Setting up online client");
		
	// Initialize steamworks! Wooooo
	if (!SteamAPI_Init())
	{
		DBG_LOG("SteamAPI_Init return failure.");
		return false;
	}
	
	SteamClient()->SetWarningMessageHook(&SteamAPI_ClientDebugCallback);
	SteamUtils()->SetOverlayNotificationPosition(k_EPositionTopLeft);
	m_initialised = true;

	return true;
}

void Steamworks_OnlineClient::Update_Presence()
{
	if (NetManager::Try_Get() == NULL)
	{
		return;
	}

	NetClient* client = NetManager::Get()->Client();
	if (client != NULL)
	{
		NetClient_State::Type state = client->Get_State();

		Steamworks_OnlineMatching* matching = Steamworks_OnlineMatching::Get();

		if (state != m_old_client_state)
		{
			switch (state)
			{
			case NetClient_State::Connected:
				{
					DBG_LOG("[Steam Client] Changing rich presence to show game info.");

					SteamFriends()->SetRichPresence("status", "In game server."); // Don't localize, we don't what language the person reading it will have.
					SteamFriends()->SetRichPresence("connect", StringHelper::Format("+connect_lobby %llu", matching->Get_Lobby_ID()).c_str());
					break;
				}
			default:
				{
					DBG_LOG("[Steam Client] Clearing rich presence.");

					SteamFriends()->ClearRichPresence();
					break;
				}
			}
		}
			
		m_old_client_state = client->Get_State();
	}
}

void Steamworks_OnlineClient::Tick(const FrameTime& time)
{
	// Update network presence status?
	Update_Presence();
	
	// Invoke steam callbacks.
	SteamAPI_RunCallbacks();
}

void Steamworks_OnlineClient::Set_Notification_Corner(int corner)
{
	SteamUtils()->SetOverlayNotificationPosition((ENotificationPosition)corner);
}

void Steamworks_OnlineClient::Show_Invite_Dialog()
{
	SteamFriends()->ActivateGameOverlayInviteDialog(Steamworks_OnlineMatching::Get()->Get_Lobby_SteamID());
}

void Steamworks_OnlineClient::Show_Profile_Dialog(OnlineUser* user)
{
	Steamworks_OnlineUser* sw_user = dynamic_cast<Steamworks_OnlineUser*>(user);
	SteamFriends()->ActivateGameOverlayToUser("steamid", sw_user->Get_SteamID());
}