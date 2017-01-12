// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Admin/AdminServer.h"
#include "Game/Runner/GameOptions.h"
#include "Game/Runner/GameMode.h"
#include "Game/Runner/Game.h"
#include "Engine/Engine/EngineOptions.h"
#include "Engine/Engine/GameEngine.h"
#include "Engine/Resources/ResourceFactory.h"
#include "Engine/IO/StreamFactory.h"
#include "Generic/Threads/Mutex.h"
#include "Game/Network/GameNetManager.h"
#include "Engine/Network/NetServer.h"
#include "Game/Network/GameNetUser.h"
#include "Game/Game/ChatManager.h"
#include "Game/Network/GameNetServer.h"
#include "Engine/Localise/Locale.h"
#include "Game/Scene/Actors/ScriptedActor.h"
#include "Engine/Scene/Map/MapFile.h"
#include "Engine/Scene/Map/MapFileHandle.h"

#ifdef OPT_STEAM_PLATFORM
#include "Engine/Online/Steamworks/Steamworks_OnlineUser.h"
#endif

#include <mongoose/include/CivetServer.h>
#include <mongoose/include/civetweb.h>

#include <algorithm>
#include <ctime>
#include <time.h>    

class AdminOverviewController : public CivetHandler
{
private:
	AdminServer* m_server;

public: 
	AdminOverviewController(AdminServer* server)
		: m_server(server)
	{
	}

	bool handleGet(CivetServer *server, struct mg_connection *conn) 
	{
		if (!m_server->check_auth(server, conn))
		{
			return true;
		}

		std::map<std::string, std::string> options;
		mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
		mg_printf(conn, "%s", m_server->GenerateTemplate("index.html", options).c_str());
		return true;
	}
};

class AdminChatController : public CivetHandler
{
private:
	AdminServer* m_server;

public: 
	AdminChatController(AdminServer* server)
		: m_server(server)
	{
	}

	bool handleGet(CivetServer *server, struct mg_connection *conn) 
	{
		if (!m_server->check_auth(server, conn))
		{
			return true;
		}

		std::map<std::string, std::string> options;
		mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
		mg_printf(conn, "%s", m_server->GenerateTemplate("chat.html", options).c_str());
		return true;
	}

	bool handlePost(CivetServer *server, struct mg_connection *conn) 
	{
		if (!m_server->check_auth(server, conn))
		{
			return true;
		}

		std::string message = "";
		CivetServer::getParam(conn, "message", message);

		ChatManager::Get()->Send_Server(ChatMessageType::Server, message);

		m_server->redirect(server, conn, "/chat");
		return true;
	}
};

class AdminSettingsController : public CivetHandler
{
private:
	AdminServer* m_server;

public: 
	AdminSettingsController(AdminServer* server)
		: m_server(server)
	{
	}

	bool handleGet(CivetServer *server, struct mg_connection *conn) 
	{
		if (!m_server->check_auth(server, conn))
		{
			return true;
		}

		std::map<std::string, std::string> options;
		mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
		mg_printf(conn, "%s", m_server->GenerateTemplate("settings.html", options).c_str());
		return true;
	}

	bool handlePost(CivetServer *server, struct mg_connection *conn) 
	{
		if (!m_server->check_auth(server, conn))
		{
			return true;
		}

		std::string map = "";
		std::string name = "";
		std::string server_secure = "";

		CivetServer::getParam(conn, "server_map", map);
		CivetServer::getParam(conn, "server_name", name);
		CivetServer::getParam(conn, "server_secure", server_secure);

		*EngineOptions::server_secure = (server_secure != "");
		*EngineOptions::server_name = name;

		if (map != "no_change" && map != "")
		{
			MapFileHandle* handle = ResourceFactory::Get()->Get_Map_File_By_Short_Name(map.c_str());
			if (handle != NULL)
			{
				GameNetManager::Get()->Game_Server()->Force_Map_Change(handle->Get()->Get_Header()->GUID.c_str(), handle->Get()->Get_Header()->Workshop_ID, Random::Static_Next(), 0);
			}
		}

		m_server->redirect(server, conn, "/settings");
		return true;
	}
};

class AdminPlayersController : public CivetHandler
{
private:
	AdminServer* m_server;

public: 
	AdminPlayersController(AdminServer* server)
		: m_server(server)
	{
	}

	bool handleGet(CivetServer *server, struct mg_connection *conn) 
	{
		if (!m_server->check_auth(server, conn))
		{
			return true;
		}

		std::map<std::string, std::string> options;
		mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
		mg_printf(conn, "%s", m_server->GenerateTemplate("players.html", options).c_str());
		return true;
	}
};

class AdminKickUserController : public CivetHandler
{
private:
	AdminServer* m_server;

public: 
	AdminKickUserController(AdminServer* server)
		: m_server(server)
	{
	}

	bool handleGet(CivetServer *server, struct mg_connection *conn) 
	{
		if (!m_server->check_auth(server, conn))
		{
			return true;
		}

		std::string user = "-1";
		CivetServer::getParam(conn, "user", user);

		int user_id = atoi(user.c_str());

		NetUser* netuser = GameNetManager::Get()->Get_User_By_Net_ID(user_id);
		if (netuser != NULL)
		{
			NetManager::Get()->Server()->Kick_User(netuser);
		}

		m_server->redirect(server, conn, "/players");
		return true;
	}
};

class AdminBanUserController : public CivetHandler
{
private:
	AdminServer* m_server;

public: 
	AdminBanUserController(AdminServer* server)
		: m_server(server)
	{
	}

	bool handleGet(CivetServer *server, struct mg_connection *conn) 
	{
		if (!m_server->check_auth(server, conn))
		{
			return true;
		}

		std::string user = "-1";
		CivetServer::getParam(conn, "user", user);

		int user_id = atoi(user.c_str());

		NetUser* netuser = GameNetManager::Get()->Get_User_By_Net_ID(user_id);
		if (netuser != NULL)
		{
			NetManager::Get()->Server()->Ban_User(netuser);
		}

		m_server->redirect(server, conn, "/players");
		return true;
	}
};

class AdminUnbanUserController : public CivetHandler
{
private:
	AdminServer* m_server;

public: 
	AdminUnbanUserController(AdminServer* server)
		: m_server(server)
	{
	}

	bool handleGet(CivetServer *server, struct mg_connection *conn) 
	{
		if (!m_server->check_auth(server, conn))
		{
			return true;
		}

		std::string user = "-1";
		CivetServer::getParam(conn, "user", user);

#if defined(LANGUAGE_STANDARD_CPP11) || defined(PLATFORM_WIN32)
		u64 id = std::stoull(user.c_str());
#else
		u64 id = strtoull(user.c_str(), NULL, 10);
#endif

		NetManager::Get()->Server()->Unban_User(id);

		m_server->redirect(server, conn, "/bans");
		return true;
	}
};

class AdminBansController : public CivetHandler
{
private:
	AdminServer* m_server;

public: 
	AdminBansController(AdminServer* server)
		: m_server(server)
	{
	}

	bool handleGet(CivetServer *server, struct mg_connection *conn) 
	{
		if (!m_server->check_auth(server, conn))
		{
			return true;
		}

		std::map<std::string, std::string> options;
		mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
		mg_printf(conn, "%s", m_server->GenerateTemplate("bans.html", options).c_str());
		return true;
	}
};

class AdminShutdownController : public CivetHandler
{
private:
	AdminServer* m_server;

public: 
	AdminShutdownController(AdminServer* server)
		: m_server(server)
	{
	}

	bool handleGet(CivetServer *server, struct mg_connection *conn) 
	{
		if (!m_server->check_auth(server, conn))
		{
			return true;
		}

		std::map<std::string, std::string> options;
		options["{{MESSAGE}}"] = "Server has been shutdown.";

		mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
		mg_printf(conn, "%s", m_server->GenerateTemplate("auth.html", options).c_str());

		GameEngine::Get()->Stop();

		return true;
	}
};

class AdminShutdownWhenEmptyController : public CivetHandler
{
private:
	AdminServer* m_server;

public:
	AdminShutdownWhenEmptyController(AdminServer* server)
		: m_server(server)
	{
	}

	bool handleGet(CivetServer *server, struct mg_connection *conn)
	{
		if (!m_server->check_auth(server, conn))
		{
			return true;
		}

		m_server->redirect(server, conn, "/");

		GameEngine::Get()->Stop_When_Empty();

		return true;
	}
};

class AdminLoginController : public CivetHandler
{
private:
	AdminServer* m_server;

public: 
	AdminLoginController(AdminServer* server)
		: m_server(server)
	{
	}

	bool handleGet(CivetServer *server, struct mg_connection *conn) 
	{
		if (m_server->check_auth(server, conn, true))
		{
			m_server->redirect(server, conn, "/overview");
			return true;
		}

		std::map<std::string, std::string> options;
		options["{{MESSAGE}}"] = "";

		mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
		mg_printf(conn, "%s", m_server->GenerateTemplate("login.html", options).c_str());
		return true;
	}

	bool handlePost(CivetServer *server, struct mg_connection *conn) 
	{
		if (m_server->check_auth(server, conn, true))
		{
			m_server->redirect(server, conn, "/overview");
			return true;
		}

		std::string username = "";
		std::string password = "";

		CivetServer::getParam(conn, "username", username);
		CivetServer::getParam(conn, "password", password);

		if (!m_server->validate_auth(server, conn, username, password))
		{
			std::map<std::string, std::string> options;
			options["{{MESSAGE}}"] = "<br/><span style=\"color:red;\">Your login was invalid.</span></br><br/>";

			mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
			mg_printf(conn, "%s", m_server->GenerateTemplate("login.html", options).c_str());
		}

		return true;
	}
};

class AdminLogoutController : public CivetHandler
{
private:
	AdminServer* m_server;

public: 
	AdminLogoutController(AdminServer* server)
		: m_server(server)
	{
	}

	bool handleGet(CivetServer *server, struct mg_connection *conn) 
	{
		mg_printf(conn, "HTTP/1.1 302 Found\r\nLocation: %s\r\nSet-Cookie: login_hash=\r\n\r\n", "/login");
		return true;
	}
};

AdminServer::AdminServer()
	: m_server(NULL)
{
}

AdminServer::~AdminServer()
{
	SAFE_DELETE(m_server);
}

void AdminServer::redirect(CivetServer *server, struct mg_connection *conn, const char* destination)
{
	mg_printf(conn, "HTTP/1.1 302 Found\r\nLocation: %s\r\n\r\n", destination);
}

bool AdminServer::validate_auth(CivetServer *server, struct mg_connection *conn, std::string username, std::string password)
{
	if (username == m_username && password == m_password)
	{
		char md5_hash[33];
		mg_md5(md5_hash, username.c_str(), password.c_str(), NULL);
		
		mg_printf(conn, "HTTP/1.1 302 Found\r\nLocation: %s\r\nSet-Cookie: login_hash=%s\r\n\r\n", "/overview", md5_hash);

		return true;
	}

	return false;
}

bool AdminServer::check_auth(CivetServer *server, struct mg_connection *conn, bool no_redirect)
{
	char md5_hash[33];
	mg_md5(md5_hash, m_username.c_str(), m_password.c_str(), NULL);

	std::string user_hash = "";
	CivetServer::getCookie(conn, "login_hash", user_hash);
	
	if (user_hash != std::string(md5_hash))
	{
		if (!no_redirect)
		{
			redirect(server, conn, "/login");
		}
		return false;
	}

	return true;
}

void AdminServer::Init()
{
	m_state_lock = ::Mutex::Create();

	m_port = *GameOptions::admin_port;
	m_username = *GameOptions::admin_username;
	m_password = *GameOptions::admin_password;
	m_root_path = ResourceFactory::Get()->Get_Root_Directory() + "Web/";

	DBG_LOG("Setting up admin server at http://localhost:%i", m_port);
	DBG_LOG("Static Files Directory: %s", m_root_path.c_str());

	m_overview_controller = new AdminOverviewController(this);
	m_chat_controller = new AdminChatController(this);
	m_settings_controller = new AdminSettingsController(this);
	m_players_controller = new AdminPlayersController(this);
	m_kick_user_controller = new AdminKickUserController(this);
	m_ban_user_controller = new AdminBanUserController(this);
	m_unban_user_controller = new AdminUnbanUserController(this);
	m_bans_controller = new AdminBansController(this);
	m_shutdown_controller = new AdminShutdownController(this);
	m_shutdown_when_empty_controller = new AdminShutdownWhenEmptyController(this);
	m_login_controller = new AdminLoginController(this);
	m_logout_controller = new AdminLogoutController(this);

	std::string port_string = StringHelper::To_String(m_port);

	const char * options[] = {
		"document_root", m_root_path.c_str(),
		"listening_ports", port_string.c_str(), 
		NULL
	};

	m_server = new CivetServer(options);

	m_server->addHandler("/$", m_overview_controller);
	m_server->addHandler("/overview$", m_overview_controller);
	m_server->addHandler("/chat$", m_chat_controller);
	m_server->addHandler("/settings$", m_settings_controller);
	m_server->addHandler("/players$", m_players_controller);
	m_server->addHandler("/kick_user$", m_kick_user_controller);
	m_server->addHandler("/ban_user$", m_ban_user_controller);
	m_server->addHandler("/unban_user$", m_unban_user_controller);
	m_server->addHandler("/bans$", m_bans_controller);
	m_server->addHandler("/shutdown$", m_shutdown_controller);
	m_server->addHandler("/shutdown_when_empty$", m_shutdown_when_empty_controller);
	m_server->addHandler("/login$", m_login_controller);
	m_server->addHandler("/logout$", m_logout_controller);

	m_start_time = Platform::Get()->Get_Ticks();
	m_active_time = 0.0f;
	m_idle_time = 0.0;
	m_peak_players = 0;
	m_active_player_sample_last_time = 0;
}

void AdminServer::Tick(const FrameTime& t)
{
	MutexLock lock(m_state_lock);

	GameNetManager* net_manager = (GameNetManager*)GameNetManager::Try_Get();
	if (net_manager == NULL)
	{
		return;
	}

	GameNetServer* server = net_manager->Game_Server();
	if (server == NULL)
	{
		return;
	}

	int up_time_seconds = (int)((Platform::Get()->Get_Ticks() - m_start_time) / 1000);
	int active_time_seconds = (int)(m_active_time / 1000);
	int idle_time_seconds = (int)(m_idle_time / 1000);

	if (net_manager->Get_Net_Users().size() > 0)
	{
		m_active_time += t.Get_Frame_Time();
	}
	else
	{
		m_idle_time += t.Get_Frame_Time();
	}

	std::vector<GameNetUser*> users = net_manager->Get_Game_Net_Users();

	if ((int)users.size() > m_peak_players)
	{
		m_peak_players = users.size();
	}

	for (std::vector<GameNetUser*>::iterator iter = users.begin(); iter != users.end(); iter++)
	{
		GameNetUser* user = *iter;
		std::string username = user->Get_Username();

		if (std::find(m_unique_users.begin(), m_unique_users.end(), username) == m_unique_users.end())
		{
			m_unique_users.push_back(username);
		}
	}

	time_t current_time;
	time(&current_time);

	struct tm* real_time = localtime(&current_time);

	if (m_active_player_samples.size() == 0 || ((real_time->tm_min % active_player_minute) == 0 && real_time->tm_min != m_active_player_sample_last_time))
	{
		ActivePlayersSample sample;
		sample.time = StringHelper::Format("%02i:%02i", real_time->tm_hour, real_time->tm_min);
		sample.value = users.size();
		sample.bShouldLabel = (real_time->tm_min == 0);

		m_active_player_samples.push_back(sample);
		m_active_player_sample_last_time = real_time->tm_min;

		if (m_active_player_samples.size() > active_player_max_samples)
		{
			m_active_player_samples.erase(m_active_player_samples.begin());
		}
	}

	std::string x_axis = "";
	std::string y_axis = "";

	for (std::vector<ActivePlayersSample>::iterator iter = m_active_player_samples.begin(); iter != m_active_player_samples.end(); iter++)
	{
		ActivePlayersSample& sample = *iter;
		if (x_axis != "")
		{
			x_axis += ", ";
			y_axis += ", ";
		}
		if (sample.bShouldLabel == true)
		{
			x_axis += StringHelper::Format("\"%s\"", sample.time.c_str());
		}
		else
		{
			x_axis += "\"\"";
		}
		y_axis += StringHelper::To_String(sample.value);
	}

	std::vector<ChatMessage> messages = ChatManager::Get()->Get_History();
	std::string chat_history = "";
	for (std::vector<ChatMessage>::iterator iter = messages.begin(); iter != messages.end(); iter++)
	{
		ChatMessage& chat = *iter;
		std::string output = chat.Display_String;
		output = StringHelper::Replace(output.c_str(), "<", "&lt;");
		output = StringHelper::Replace(output.c_str(), ">", "&gt;");
		output = StringHelper::Replace(output.c_str(), "[/c]", "</span>");
		output = StringHelper::Replace(output.c_str(), "[c=", "<span style=\"color: rgb(");
		output = StringHelper::Replace(output.c_str(), "]", ")\">");
		chat_history += output + "</br>";
	}

	GameMode* mode = Game::Get()->Get_Game_Mode();
	std::string player_table_columns = "";
	std::string player_table_rows = "";
	bool bUseAlt = false;
	if (mode != NULL)
	{
		ScriptedActor* game_mode_actor = mode->Get_Game_Mode();
		if (game_mode_actor != NULL)
		{
			CVirtualMachine* vm = GameEngine::Get()->Get_VM();
			CVMContextLock scriptlock = vm->Set_Context(game_mode_actor->Get_Script_Context());

			CVMLinkedSymbol* game_mode_symbol = vm->Find_Class("Game_Mode");
			CVMLinkedSymbol* get_scoreboard_value_symbol = vm->Find_Function(game_mode_symbol, "Get_Scoreboard_Value", 2, "NetUser", "int"); 

			std::vector<std::string>  scoreboard_columns = mode->Get_Scoreboard_Columns();
			for (std::vector<std::string>::iterator iter = scoreboard_columns.begin(); iter != scoreboard_columns.end(); iter++)
			{
				std::string& name = *iter;
				name = StringHelper::Replace(name.c_str(), "<", "&lt;");
				name = StringHelper::Replace(name.c_str(), ">", "&gt;");
				player_table_columns += StringHelper::Format("<td><b>%s</b></td>\n", S(name));
			}

			for (std::vector<GameNetUser*>::iterator iter = users.begin(); iter != users.end(); iter++, bUseAlt = !bUseAlt)
			{
				GameNetUser*	  user		  = *iter;
				OnlineUser*		  online_user = user->Get_Online_User();
				GameNetUserState* state		  = user->Get_State();
				Profile*		  profile	  = user->Get_Profile();

				std::string id = "";

	#ifdef OPT_STEAM_PLATFORM
				Steamworks_OnlineUser* sw_user = static_cast<Steamworks_OnlineUser*>(online_user);
				id = StringHelper::Format("%llu", sw_user->Get_SteamID().ConvertToUint64());
	#endif

				player_table_rows += bUseAlt ? "<tr class=\"item_list_table_item_row_alt\">" : "<tr class=\"item_list_table_item_row\">";

				std::string name = user->Get_Username();
				name = StringHelper::Replace(name.c_str(), "<", "&lt;");
				name = StringHelper::Replace(name.c_str(), ">", "&gt;");

				player_table_rows += StringHelper::Format("<td><a href=\"http://steamcommunity.com/profiles/%s\">%s</a></td>", id.c_str(), name.c_str());

				// Add game-mode specific columns.
				{
					int column_index = 0;
					for (std::vector<std::string>::iterator columniter = scoreboard_columns.begin(); columniter != scoreboard_columns.end(); columniter++, column_index++)
					{
						// Ask for info from script.
					
						CVMValue param_1 = user->Get_Script_Object();
						CVMValue param_2 = column_index;
						CVMValue instance = game_mode_actor->Get_Script_Object();
						vm->Push_Parameter(param_1);
						vm->Push_Parameter(param_2);
						vm->Invoke(get_scoreboard_value_symbol, instance);

						CVMValue retval;
						vm->Get_Return_Value(retval);

						player_table_rows += StringHelper::Format("<td>%s</td>", retval.string_value.C_Str());
					}
				}

				// Good ping.
				if (state->Ping < 100)
				{			
					player_table_rows += StringHelper::Format("<td style='color:rgb(0,255,0)'>%i ms</td>", state->Ping);
				}
				// Eh ping.
				else if (state->Ping < 150)
				{
					player_table_rows += StringHelper::Format("<td style='color:rgb(255,0,0)'>%i ms</td>", state->Ping);
				}
				// Bad ping.
				else 
				{
					player_table_rows += StringHelper::Format("<td style='color:rgb(200,0,0)'>%i ms</td>", state->Ping);
				}
			
				player_table_rows += StringHelper::Format("<td>"
									 "<div class=\"button\" onclick=\"Redirect('kick_user?user=%i');\">Kick</div>"
									 "<div class=\"button\" onclick=\"Redirect('ban_user?user=%i');\">Ban</div>"
									 "</td>", user->Get_Net_ID(), user->Get_Net_ID());

				player_table_rows += "</tr>";
			}
		}
	}

	std::vector<NetServerBan> bans = server->Get_Bans();
	std::string ban_table_rows = "";
	for (std::vector<NetServerBan>::iterator iter = bans.begin(); iter != bans.end(); iter++, bUseAlt = !bUseAlt)
	{
		NetServerBan& ban = *iter;

		std::string name = ban.username;
		name = StringHelper::Replace(name.c_str(), "<", "&lt;");
		name = StringHelper::Replace(name.c_str(), ">", "&gt;");

		ban_table_rows += bUseAlt ? "<tr class=\"item_list_table_item_row_alt\">" : "<tr class=\"item_list_table_item_row\">";
		ban_table_rows += StringHelper::Format("<td><a href=\"http://steamcommunity.com/profiles/%llu\">%s</a></td>", ban.id, name.c_str());

		ban_table_rows += StringHelper::Format("<td>"
			"<div class=\"button\" onclick=\"Redirect('unban_user?user=%llu');\">Revoke</div>"
			"</td>", ban.id);

		ban_table_rows += "</tr>";
	}

	std::string is_secure_checked = "";
	std::string map_selection_options = "<option value=\"no_change\">No Change</option>";

	std::vector<MapFileHandle*> maps = ResourceFactory::Get()->Get_Sorted_Map_Files();
	for (std::vector<MapFileHandle*>::iterator iter = maps.begin(); iter != maps.end(); iter++)
	{
		MapFileHandle* map = *iter;
		map_selection_options += StringHelper::Format("<option value=\"%s\">%s</option>", 
			map->Get()->Get_Header()->Short_Name.c_str(), 
			map->Get()->Get_Header()->Long_Name.c_str());
	}

	if (*EngineOptions::server_secure)
	{
		is_secure_checked = "checked=\"checked\"";
	}

	m_state["{{IS_SECURE_CHECKED}}"]			= is_secure_checked;
	m_state["{{MAP_SELECTION_OPTIONS}}"]		= map_selection_options;
	m_state["{{BAN_TABLE_ROWS}}"]				= ban_table_rows;
	m_state["{{PLAYER_TABLE_COLUMN_HEADERS}}"]	= player_table_columns;
	m_state["{{PLAYER_TABLE_ROWS}}"]			= player_table_rows;
	m_state["{{CHAT_HISTORY}}"]					= chat_history;
	m_state["{{ACTIVE_PLAYERS_GRAPH_X_AXIS}}"]	= x_axis;
	m_state["{{ACTIVE_PLAYERS_GRAPH_Y_AXIS}}"]	= y_axis;
	m_state["{{UP_TIME}}"]						= StringHelper::Format("%i Days, %i Hours, %i Minutes", ((up_time_seconds / 60) / 60) / 24, ((up_time_seconds / 60) / 60) % 24, (up_time_seconds / 60) % 60);
	m_state["{{ACTIVE_TIME}}"]					= StringHelper::Format("%i Days, %i Hours, %i Minutes", ((active_time_seconds / 60) / 60) / 24, ((active_time_seconds / 60) / 60) % 24, (active_time_seconds / 60) % 60);
	m_state["{{IDLE_TIME}}"]					= StringHelper::Format("%i Days, %i Hours, %i Minutes", ((idle_time_seconds / 60) / 60) / 24, ((idle_time_seconds / 60) / 60) % 24, (idle_time_seconds / 60) % 60);
	m_state["{{SERVER_NAME}}"]					= *EngineOptions::server_name;
	m_state["{{TOTAL_UNIQUE_PLAYERS}}"]			= StringHelper::To_String(m_unique_users.size());
	m_state["{{PEAK_PLAYERS}}"]					= StringHelper::To_String(m_peak_players);
	m_state["{{MAPS_PLAYED}}"]					= StringHelper::To_String(server->Map_Load_Count());
	m_state["{{CURRENT_MAP}}"]					= net_manager->Get_Current_Short_Map_Name();
	m_state["{{CURRENT_PLAYER_COUNT}}"]			= StringHelper::To_String(users.size());
	m_state["{{CURRENT_MAX_PLAYER_COUNT}}"]		= StringHelper::To_String(server->Total_Player_Slots());
	m_state["{{SHUTDOWN_WHEN_EMPTY_TEXT}}"]		= GameEngine::Get()->Is_Stop_When_Empty_Pending() ? "Shutdown Pending" : "Shutdown When Empty";
}

std::string AdminServer::GenerateTemplate(const char* filename, std::map<std::string, std::string>& replacements)
{
	DBG_LOG("[Admin] Generating template: %s", filename);

	// Load template if required.
#ifdef MASTER_BUILD
	if (m_templates.find(filename) == m_templates.end())
#endif
	{
		std::string path = m_root_path + "templates/" + filename;

		if (Platform::Get()->Is_File(path.c_str()))
		{
			DBG_LOG("[Admin] Loading template: %s from %s", filename, path.c_str());

			std::string temp = "";
			StreamFactory::Load_Text(path.c_str(), &temp);

			m_templates[filename] = temp;
		}
		else
		{
			DBG_LOG("[Admin] Could not load template (file not found): %s from %s", filename, path.c_str());
			return StringHelper::Format("Unable to load template: %s", filename);
		}
	}

	std::string result = m_templates[filename];

	// Replace standard targets.
	{
		MutexLock lock(m_state_lock);
		for (std::map<std::string, std::string>::iterator iter = m_state.begin(); iter != m_state.end(); iter++)
		{
			result = StringHelper::Replace(result.c_str(), iter->first.c_str(), iter->second.c_str());
		}
	}

	// Do page-specific replacements.
	for (std::map<std::string, std::string>::iterator iter = replacements.begin(); iter != replacements.end(); iter++)
	{
		result = StringHelper::Replace(result.c_str(), iter->first.c_str(), iter->second.c_str());
	}

	return result;
}