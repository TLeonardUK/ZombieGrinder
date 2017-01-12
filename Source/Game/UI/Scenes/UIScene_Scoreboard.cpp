// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_Scoreboard.h"
#include "Engine/UI/Transitions/UISlideInTransition.h"
#include "Engine/UI/Transitions/UIFadeInTransition.h"
#include "Engine/UI/Transitions/UIFadeOutTransition.h"
#include "Engine/UI/Transitions/UIFadeTransition.h"
#include "Engine/UI/Layouts/UILayoutFactory.h"
#include "Engine/UI/Elements/UIPanel.h"
#include "Engine/UI/Elements/UILabel.h"
#include "Engine/UI/Elements/UISimpleListView.h"

#include "Engine/Platform/Platform.h"
#include "Engine/Display/GfxDisplay.h"

#include "Engine/Engine/GameEngine.h"
#include "Engine/Scene/Scene.h"

#include "Engine/Input/Input.h"

#include "Engine/UI/UIManager.h"

#include "Engine/Network/NetManager.h"
#include "Engine/Network/NetUser.h"

#include "Engine/Renderer/RenderPipeline.h"

#include "Game/UI/Scenes/Editor/UIScene_Editor.h"

#include "Game/Network/GameNetManager.h"
#include "Game/Network/GameNetUser.h"
#include "Game/Network/GameNetUserState.h"
#include "Game/Network/GameNetClient.h"

#include "Game/Scene/Actors/ScriptedActor.h"

#include "Engine/Localise/Locale.h"

#include "Game/Runner/Game.h"
#include "Game/Runner/GameMode.h"

UIScene_Scoreboard::UIScene_Scoreboard()
	: m_update_timer(0.0f)
{
	Set_Layout("Scoreboard");
}

UIScene_Scoreboard::~UIScene_Scoreboard()
{
}

bool UIScene_Scoreboard::Should_Tick_When_Not_Top()
{
	return false;
}

const char* UIScene_Scoreboard::Get_Name()
{
	return "UIScene_Scoreboard";
}

bool UIScene_Scoreboard::Should_Render_Lower_Scenes()
{
	return true;
}

bool UIScene_Scoreboard::Should_Render_Lower_Scenes_Background()
{
	return true;
}

bool UIScene_Scoreboard::Is_Focusable()
{
	return false;
}

bool UIScene_Scoreboard::Should_Display_Cursor()
{
	return false;
}

void UIScene_Scoreboard::Enter(UIManager* manager)
{
	Update_Map_Details();

	UISimpleListView* list_view = Find_Element<UISimpleListView*>("list_view");
	Update_Scoreboard(list_view);
}	

void UIScene_Scoreboard::Update_Map_Details()
{
	NetManager*			net_manager	 = NetManager::Get();
	GameNetClient*		net_client	 = static_cast<GameNetClient*>(net_manager->Client());
	GameNetServerState	server_state = net_client->Get_Server_State();
	GameMode*			mode		 = Game::Get()->Get_Game_Mode();

	UILabel* map_detail_label	= Find_Element<UILabel*>("map_detail_label");
	UILabel* map_state_label	= Find_Element<UILabel*>("map_stage_label");

	// Update label detail.
	map_detail_label->Set_Value(StringHelper::Format("%s  -  %s  -  %s", 
		net_manager->Get_Current_Short_Map_Name().c_str(), 
		server_state.Name.c_str(), 
		StringHelper::Format_Time("hh:mm:ss", (int)server_state.Elapsed_Map_Time).c_str()));

	map_state_label->Set_Value(S(mode->Get_Stage_Text().c_str()));
}

void UIScene_Scoreboard::Update_Scoreboard(UISimpleListView* list_view)
{
	NetManager*		net_manager					 = NetManager::Get();
	GameNetClient*  net_client					 = static_cast<GameNetClient*>(net_manager->Client());
	OnlineMatching_Server* lobby_server			 = OnlineMatching::Get()->Get_Lobby_Server();

	GameNetServerState server_state			 = net_client->Get_Server_State();

	GameMode* mode								 = Game::Get()->Get_Game_Mode();
	std::vector<std::string>  scoreboard_columns = mode->Get_Scoreboard_Columns();

	ScriptedActor* game_mode_actor				= mode->Get_Game_Mode();

	// Get VM ready for some calls we are about to make.
	CVirtualMachine* vm = GameEngine::Get()->Get_VM();
	CVMContextLock lock = vm->Set_Context(game_mode_actor->Get_Script_Context());

	CVMLinkedSymbol* game_mode_symbol = vm->Find_Class("Game_Mode");
	CVMLinkedSymbol* get_scoreboard_value_symbol = vm->Find_Function(game_mode_symbol, "Get_Scoreboard_Value", 2, "NetUser", "int"); 
	CVMLinkedSymbol* get_scoreboard_sort_value_symbol = vm->Find_Function(game_mode_symbol, "Get_Scoreboard_Sort_Value", 1, "NetUser");
	CVMLinkedSymbol* get_scoreboard_dead_value_symbol = vm->Find_Function(game_mode_symbol, "Get_Scoreboard_Is_Dead", 1, "NetUser");
	CVMLinkedSymbol* get_scoreboard_team_score_symbol = vm->Find_Function(game_mode_symbol, "Get_Scoreboard_Team_Score", 1, "int"); 
	
	// Clear existing columns/items.
	list_view->Clear_Columns();
	list_view->Clear_Items();

	// Add all the columns.
	list_view->Add_Column("", 0.05f, false, true); // Avatar
	list_view->Add_Column("", 0.056f, false, true); // Flag
	list_view->Add_Column("", 0.046f, false, true); // Voice Chat
	list_view->Add_Column("", 0.046f, false, true); // Premium Pip
	list_view->Add_Column(S("#scoreboard_header_level"), 0.05f, false, false); // Level
	list_view->Add_Column(S("#scoreboard_header_name"), 0.290f, false, true); // Name

	float max_width  = 0.392f;
	float per_column = max_width / scoreboard_columns.size();
	for (std::vector<std::string>::iterator iter = scoreboard_columns.begin(); iter != scoreboard_columns.end(); iter++)
	{
		std::string& name = *iter;
		list_view->Add_Column(S(name.c_str()), per_column, false, false); // Score
	}

	list_view->Add_Column(S("#scoreboard_header_ping"), 0.07f, false, true); // Ping
	
	// Sort users!
	std::vector<ScoreboardNetUser> users;
	std::vector<NetUser*> net_users = NetManager::Get()->Get_Net_Users();
	
	for (std::vector<NetUser*>::iterator useriter = net_users.begin(); useriter != net_users.end(); useriter++)
	{
		GameNetUser* user = static_cast<GameNetUser*>((*useriter));

		ScoreboardNetUser sb_user;
		sb_user.user = user;

		// Ask for sort value.
		{
			CVMValue value = user->Get_Script_Object();
			vm->Push_Parameter(value);

			CVMValue instance = game_mode_actor->Get_Script_Object();
			vm->Invoke(get_scoreboard_sort_value_symbol, instance);

			CVMValue retval;
			vm->Get_Return_Value(retval);
			sb_user.sort_value = retval.int_value;
		}

		// Ask for dead value.
		{
			CVMValue value = user->Get_Script_Object();
			vm->Push_Parameter(value);

			CVMValue instance = game_mode_actor->Get_Script_Object();
			vm->Invoke(get_scoreboard_dead_value_symbol, instance);

			CVMValue retval;
			vm->Get_Return_Value(retval);
			sb_user.is_dead = (retval.int_value != 0);
		}

		users.push_back(sb_user);
	}

	std::sort(users.begin(), users.end(), &ScoreboardNetUser::Sort_Predicate);

	// Sort teams!
	std::vector<ScoreboardNetTeam> teams;
	std::vector<GameModeTeam>& net_teams	= mode->Get_Teams();
	int team_index = 0;

	for (std::vector<GameModeTeam>::iterator useriter = net_teams.begin(); useriter != net_teams.end(); useriter++, team_index++)
	{
		GameModeTeam& team = (*useriter);

		ScoreboardNetTeam sb_user;
		sb_user.team = &team;
		sb_user.team_index = team_index;

		// Ask for info from script.
		CVMValue team_index_value(team_index);
		CVMValue instance_value = game_mode_actor->Get_Script_Object();
		vm->Push_Parameter(team_index_value);
		vm->Invoke(get_scoreboard_team_score_symbol, instance_value);

		CVMValue retval;
		vm->Get_Return_Value(retval);
		sb_user.score = retval.int_value;

		teams.push_back(sb_user);
	}

	std::sort(teams.begin(), teams.end(), &ScoreboardNetTeam::Sort_Predicate);

	// Add all the teams.
	std::vector<std::string> item_values;
	for (std::vector<ScoreboardNetTeam>::iterator iter = teams.begin(); iter != teams.end(); iter++)
	{
		ScoreboardNetTeam& team = *iter;

		bool bUsersInTeam = false;
		for (std::vector<ScoreboardNetUser>::iterator useriter = users.begin(); useriter != users.end(); useriter++)
		{
			GameNetUser* user = static_cast<GameNetUser*>((*useriter).user);
			GameNetUserState* state	= user->Get_State();
			if (state->Team_Index == team.team_index)
			{
				bUsersInTeam = true;
				break;
			}
		}

		if (teams.size() > 1 && bUsersInTeam)
		{
			std::string name  = S(team.team->Name.c_str());
			Color		color = team.team->Primary_Color;

			item_values.clear();
			item_values.push_back(StringHelper::Format("[c=%i,%i,%i]%s[/c]", color.R, color.G, color.B, name.c_str()));

			item_values.push_back("");
			item_values.push_back("");
			item_values.push_back("");
			item_values.push_back("");
			item_values.push_back("");
			item_values.push_back(StringHelper::To_String<int>(team.score));

			for (unsigned int i = 0; i < scoreboard_columns.size(); i++)
			{
				item_values.push_back("");
			}

			list_view->Add_Item(item_values);
		}
	
		// Add all users in the team.
		for (std::vector<ScoreboardNetUser>::iterator useriter = users.begin(); useriter != users.end(); useriter++)
		{
			ScoreboardNetUser& scoreboard_user = *useriter;
			GameNetUser*	  user		  = static_cast<GameNetUser*>((*useriter).user);
			OnlineUser*		  online_user = user->Get_Online_User();
			GameNetUserState* state		  = user->Get_State();
			Profile*		  profile	  = user->Get_Profile();

			if (state->Team_Index != team.team_index && teams.size() > 1)
			{
				continue;
			}

			item_values.clear();

			item_values.push_back(StringHelper::Format("[avatar=%i,1,1,0,0]",					online_user->Get_ID()));
			item_values.push_back(StringHelper::Format("[img=flag_%s,1.0,1.0,0,0]",					StringHelper::Lowercase(online_user->Get_GeoIP_Result().CountryShortName).c_str()));
			if (user->Get_Local_User_Index() != 0)
			{
				item_values.push_back("");
			}
			else
			{
				item_values.push_back(StringHelper::Format("[img=voice_chat_indicator_%i,1.0,1.0,0,0]", online_user->Is_Talking() ? 1 : 0));
			}

			std::string emblem = "";

			if (online_user->Is_Developer())
			{
				emblem = "[img=game_hud_emblem_developer,1.0,1.0,0,0]";
			}
#ifdef OPT_PREMIUM_ACCOUNTS
			else if (online_user->Get_Premium())
			{
				emblem = "[img=game_hud_emblem_large_donator,1.0,1.0,0,0]";
			}
#endif

			item_values.push_back(emblem);
			item_values.push_back(StringHelper::To_String(profile == NULL ? 0 : profile->Level));

			std::string escape_username = MarkupFontRenderer::Escape(user->Get_Username());

			if (scoreboard_user.is_dead)
			{
				escape_username = SF("#scoreboard_column_username_dead", escape_username.c_str());
			}

#ifdef OPT_PREMIUM_ACCOUNTS
			if (user->Get_Online_User()->Get_Premium())
			{
				item_values.push_back(StringHelper::Format("[c=255,194,14,255]%s[/c]", escape_username.c_str()));
			}
			else
			{
#endif
				item_values.push_back(escape_username);
#ifdef OPT_PREMIUM_ACCOUNTS		
			}
#endif

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
					item_values.push_back(retval.string_value.C_Str());
				}
			}
		
			// Good ping.
			if (state->Ping < 100)
			{
				item_values.push_back(StringHelper::Format("[c=0,255,0]%i[/c]", state->Ping));
			}
			// Eh ping.
			else if (state->Ping < 150)
			{
				item_values.push_back(StringHelper::Format("[c=255,200,0]%i[/c]", state->Ping));
			}
			// Bad ping.
			else 
			{
				item_values.push_back(StringHelper::Format("[c=200,0,0]%i[/c]", state->Ping));
			}

			list_view->Add_Item(item_values, user);
		}
	}
}

void UIScene_Scoreboard::Exit(UIManager* manager)
{
}	

void UIScene_Scoreboard::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	// Close sub menu?
	if (!Input::Get()->Is_Down_By_Any(OutputBindings::Scoreboard))
	{
		manager->Play_UI_Sound(UISoundType::Small_Click);
		manager->Go(UIAction::Pop(new UIFadeOutTransition()));
		return;
	}

	// Update the scoreboard periodically.
	m_update_timer += time.Get_Frame_Time();
	if (m_update_timer > update_interval)
	{
		UISimpleListView* list_view = Find_Element<UISimpleListView*>("list_view");
		Update_Scoreboard(list_view);
		Update_Map_Details();

		m_update_timer = 0.0f;
	}

	UIScene::Tick(time, manager, scene_index);
}

void UIScene_Scoreboard::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Draw(time, manager, scene_index);
}



