// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_ADMIN_SERVER_
#define _GAME_ADMIN_SERVER_

#include "Engine/Engine/FrameTime.h"

#include <map>

#include <mongoose/include/CivetServer.h>

class AdminOverviewController;
class AdminChatController;
class AdminSettingsController;
class AdminPlayersController;
class AdminKickUserController;
class AdminBanUserController;
class AdminUnbanUserController;
class AdminBansController;
class AdminShutdownController;
class AdminLoginController;
class AdminLogoutController;
class AdminShutdownWhenEmptyController;
class Mutex;

struct ActivePlayersSample
{
	std::string time;
	int value;
	bool bShouldLabel;
};

class AdminServer
{
	MEMORY_ALLOCATOR(AdminServer, "Game");

private:
	friend class AdminBaseController;
	friend class AdminOverviewController;
	friend class AdminChatController;
	friend class AdminSettingsController;
	friend class AdminPlayersController;
	friend class AdminKickUserController;
	friend class AdminBanUserController;
	friend class AdminUnbanUserController;
	friend class AdminBansController;
	friend class AdminShutdownController;
	friend class AdminLoginController;
	friend class AdminLogoutController;
	friend class AdminShutdownWhenEmptyController;

	Mutex* m_state_lock;
	std::map<std::string, std::string> m_state;

	double m_start_time;
	double m_active_time;
	double m_idle_time;
	int m_peak_players;

	std::vector<std::string> m_unique_users;

	CivetServer* m_server;
	AdminOverviewController* m_overview_controller;
	AdminChatController* m_chat_controller;
	AdminSettingsController* m_settings_controller;
	AdminPlayersController* m_players_controller;
	AdminKickUserController* m_kick_user_controller;
	AdminBanUserController* m_ban_user_controller;
	AdminUnbanUserController* m_unban_user_controller;
	AdminBansController* m_bans_controller;
	AdminShutdownController* m_shutdown_controller;
	AdminLoginController* m_login_controller;
	AdminLogoutController* m_logout_controller;
	AdminShutdownWhenEmptyController* m_shutdown_when_empty_controller;

	std::map<std::string, std::string> m_templates;

	int m_port;
	std::string m_username;
	std::string m_password;

	std::string m_root_path;

	int m_active_player_sample_last_time;

	std::vector<ActivePlayersSample> m_active_player_samples;

	enum 
	{
		active_player_minute = 15,			// Which minutes we sample on (mins%this) == 0
		active_player_max_samples = 5 * 24	// 24 hours
	};

public:
	AdminServer();
	~AdminServer();

	void Init();
	void Tick(const FrameTime& time);

	std::string GenerateTemplate(const char* filename, std::map<std::string, std::string>& replacements);

	bool validate_auth(CivetServer *server, struct mg_connection *conn, std::string username, std::string password);
	bool check_auth(CivetServer *server, struct mg_connection *conn, bool no_redirect = false);
	void redirect(CivetServer *server, struct mg_connection *conn, const char* destination);

};

#endif

