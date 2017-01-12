// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Network/GameNetManager.h"
#include "Game/Network/GameNetClient.h"
#include "Game/Network/GameNetServer.h"
#include "Game/Network/GameNetUser.h"

#include "Game/Game/EditorManager.h"

#include "Generic/Helper/VectorHelper.h"

#include "Game/Network/Packets/GamePackets.h"

#include "Engine/Engine/GameEngine.h"

#include "Game/Runner/Game.h"

#include "Game/Scene/Actors/ScriptedActor.h"

#include "Engine/Scene/Map/Map.h"
#include "Engine/Scene/Map/MapFile.h"
#include "Game/Scene/Map/MapSerializer.h"
#include "Game/Scene/Map/Blocks/MapFileLayerBlock.h"
#include "Game/Scene/Map/Blocks/MapFileObjectBlock.h"
#include "Engine/Scene/Map/Blocks/MapFileHeaderBlock.h"

#include "Game/UI/Scenes/UIScene_Scoreboard.h"

#include "Game/Scene/Map/MapSerializer.h"

#include "Game/Version.h"

GameNetManager::GameNetManager()
	: m_client(NULL)
	, m_server(NULL)
{
	Register_Game_Packets();
}

GameNetManager::~GameNetManager()
{
}

void GameNetManager::DeInit()
{
	SAFE_DELETE(m_client);
	SAFE_DELETE(m_server);

	NetManager::DeInit();
}

bool GameNetManager::Init()
{	
	if (!NetManager::Init())
	{
		return false;
	}

	return true;
}

void GameNetManager::Tick(const FrameTime& time)
{
	NetManager::Tick(time);
}

void GameNetManager::Update_User_Scoreboard_Ranks()
{
	GameMode* mode = Game::Get()->Get_Game_Mode();
	if (mode)
	{
		std::vector<std::string>  scoreboard_columns = mode->Get_Scoreboard_Columns();
		ScriptedActor* game_mode_actor = mode->Get_Game_Mode();
		if (game_mode_actor)
		{
			// Get VM ready for some calls we are about to make.
			CVirtualMachine* vm = GameEngine::Get()->Get_VM();
			CVMContextLock lock = vm->Set_Context(game_mode_actor->Get_Script_Context());

			CVMLinkedSymbol* game_mode_symbol = vm->Find_Class("Game_Mode");
			CVMLinkedSymbol* get_scoreboard_sort_value_symbol = vm->Find_Function(game_mode_symbol, "Get_Scoreboard_Sort_Value", 1, "NetUser");

			// Update scoreboard ranks for net users.
			std::vector<ScoreboardNetUser> users;
			std::vector<NetUser*> net_users = NetManager::Get()->Get_Net_Users();

			for (std::vector<NetUser*>::iterator useriter = net_users.begin(); useriter != net_users.end(); useriter++)
			{
				GameNetUser* user = static_cast<GameNetUser*>((*useriter));

				ScoreboardNetUser sb_user;
				sb_user.user = user;

				// Ask for info from script.
				CVMValue value = user->Get_Script_Object();
				vm->Push_Parameter(value);

				CVMValue instance = game_mode_actor->Get_Script_Object();
				if (instance.object_value.Get() != NULL)
				{
					vm->Invoke(get_scoreboard_sort_value_symbol, instance);

					CVMValue retval;
					vm->Get_Return_Value(retval);
					sb_user.sort_value = retval.int_value;

					users.push_back(sb_user);
				}
			}

			std::sort(users.begin(), users.end(), &ScoreboardNetUser::Sort_Predicate);

			int rank = 0;
			for (std::vector<ScoreboardNetUser>::iterator iter = users.begin(); iter != users.end(); iter++, rank++)
			{
				ScoreboardNetUser& user = *iter;
				GameNetUser* u = static_cast<GameNetUser*>(user.user);
				u->Set_Scoreboard_Rank(rank);
			}
		}
	}
}

bool GameNetManager::Init_Client()
{
	DBG_ASSERT(m_client == NULL);

	m_client = new GameNetClient();
	if (!m_client->Init())
	{
		SAFE_DELETE(m_client);
		return false;
	}

	return true;
}

bool GameNetManager::Init_Server()
{
	DBG_ASSERT(m_server == NULL);
	
	m_server = new GameNetServer();
	if (!m_server->Init())
	{
		SAFE_DELETE(m_server);
		return false;
	}

	return true;
}

bool GameNetManager::Destroy_Client()
{	
	if (m_client == NULL)
	{
		return true;
	}
	SAFE_DELETE(m_client);
	return true;
}

bool GameNetManager::Destroy_Server()
{
	if (m_server == NULL)
	{
		return true;
	}
	SAFE_DELETE(m_server);
	return true;
}

NetClient* GameNetManager::Client()
{
	return m_client;
}

NetServer* GameNetManager::Server()
{
	return m_server;
}

GameNetClient* GameNetManager::Game_Client()
{
	return static_cast<GameNetClient*>(Client());
}

GameNetServer* GameNetManager::Game_Server()
{
	return static_cast<GameNetServer*>(Server());
}

bool GameNetManager::Is_Game_Active()
{
	if (m_server != NULL)
	{
		std::vector<GameNetUser*> game_users = Get_Game_Net_Users();
		for (std::vector<GameNetUser*>::iterator iter = game_users.begin(); iter != game_users.end(); iter++)
		{
			GameNetUser* user = *iter;
			if (user->Get_Game_State() == GameNetClient_GameState::SyncingWithServer ||
				user->Get_Game_State() == GameNetClient_GameState::InCutscene ||
				user->Get_Game_State() == GameNetClient_GameState::InGame)
			{
				return true;
			}
		}
		return false;
	}
	return true;
}

std::vector<GameNetUser*> GameNetManager::Get_Game_Net_Users()
{
	std::vector<NetUser*> net_users = NetManager::Get()->Get_Net_Users();
	std::vector<GameNetUser*> game_users = VectorHelper::Cast<GameNetUser*>(net_users);
	return game_users;
}

unsigned int GameNetManager::Get_Net_Version()
{
//#ifndef MASTER_BUILD
//	return 0;
//#else
	unsigned int gameVersion = VersionInfo::TOTAL_CHANGELISTS;
	return gameVersion;
//#endif
}

NetUser* GameNetManager::Create_Net_User(NetConnection* connection, int local_user_index, OnlineUser* user)
{
	if (user == NULL)
	{
		user = connection->Get_Online_User();
	}

	// Have to match up correct local-user-index's for local users so we
	// have correct input-source/etc data.
	if (user->Is_Local())
	{
		user = OnlinePlatform::Get()->Get_Local_User_By_Index(local_user_index);
	}

	GameNetUser* netuser = new GameNetUser(connection, local_user_index, user);
	m_net_users.push_back(netuser);

	return netuser;
}

GameNetUser* GameNetManager::Get_Game_Net_User_By_Connection(NetConnection* connection)
{
	std::vector<NetUser*> game_users = Get_Net_Users();
	for (std::vector<NetUser*>::iterator iter = game_users.begin(); iter != game_users.end(); iter++)
	{
		NetUser* user = *iter;
		if (user->Get_Connection() == connection)
		{
			return static_cast<GameNetUser*>(user);
		}
	}

	return NULL;
}

GameNetUser* GameNetManager::Get_Game_User_By_Net_ID(int net_id)
{
	std::vector<NetUser*> game_users = Get_Net_Users();
	for (std::vector<NetUser*>::iterator iter = game_users.begin(); iter != game_users.end(); iter++)
	{
		NetUser* user = *iter;
		if (user->Get_Net_ID() == net_id)
		{
			return static_cast<GameNetUser*>(user);
		}
	}
	return NULL;
}

bool GameNetManager::Any_Users_In_Cutscene()
{
	std::vector<GameNetUser*> game_users = Get_Game_Net_Users();
	for (std::vector<GameNetUser*>::iterator iter = game_users.begin(); iter != game_users.end(); iter++)
	{
		GameNetUser* user = *iter;
		if (user->Get_Game_State() == GameNetClient_GameState::InCutscene)
		{
			return true;
		}
	}
	return false;
}

void GameNetManager::Leave_Network_Game()
{
	// Disable update till we dispose of everything correctly.
	Game::Get()->Set_Update_Game(false);

	// Leave the network.
	NetManager::Leave_Network_Game();
}

void GameNetManager::Editor_Mode_Changed()
{
	DBG_LOG("Editor mode has changed. Making map custom.");

	// Serialize the editor map so we can do reloads etc.
	Map* map = Game::Get()->Get_Map();

	BinaryStream stream;
	MapSerializer serializer(map);
	serializer.Serialize(&stream);
	EditorManager::Get()->Update_Editor_Map_File(DataBuffer(stream.Data(), stream.Length()));

	// Any local clients that are "in-cutscenes" are taken straight to in-game.
	std::vector<GameNetUser*> game_users = Get_Game_Net_Users();
	for (std::vector<GameNetUser*>::iterator iter = game_users.begin(); iter != game_users.end(); iter++)
	{
		GameNetUser* user = *iter;
		if (user->Get_Online_User()->Is_Local() && user->Get_Game_State() == GameNetClient_GameState::InCutscene)
		{
			user->Set_Game_State(GameNetClient_GameState::InGame);
		}
	}

	// Set map name to internal editor file name so we reload that
	// when restarting, not the originating map. Also forces users
	// who attempt to join to download the new map.
	NetManager::Get()->Set_Map_GUID(INTERNAL_EDITOR_MAP_GUID, 0);
}

GameNetServerState& GameNetManager::Get_Server_State()
{
	if (Game_Server() != NULL)
	{
		return Game_Server()->Get_Server_State();
	}
	else
	{
		return Game_Client()->Get_Server_State();
	}
}