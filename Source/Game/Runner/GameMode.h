// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_GAMEMODE_
#define _GAME_GAMEMODE_

#include "Generic/Patterns/Singleton.h"

#include "Engine/Engine/FrameTime.h"
#include "Engine/Engine/GameRunner.h"

#include "Engine/Renderer/Canvas.h"

#include "Game/Runner/GameModeTeam.h"

#include "Game/Scripts/ScriptEventListener.h"

#include "Game/UI/Scenes/UIScene_GameOver.h"

#include "Engine/Online/OnlinePlatform.h"

#include <vector>

class ScriptedActor;

struct CVMLinkedSymbol;
class GameMode
{
	MEMORY_ALLOCATOR(GameMode, "Game");

private:
	std::vector<std::string>	m_scoreboard_columns;
	std::vector<GameModeTeam>	m_teams;

	std::string					m_stage_text;

	int							m_active_net_id;
	ScriptedActor*				m_game_mode_object;

	CVMLinkedSymbol*			m_symbol_is_pvp;
	CVMLinkedSymbol*			m_symbol_get_teams;
	CVMLinkedSymbol*			m_symbol_get_scoreboard_columns;
	CVMLinkedSymbol*			m_symbol_get_scoreboard_title;
	CVMLinkedSymbol*			m_symbol_create_camera;
	CVMLinkedSymbol*			m_symbol_create_hud;
	CVMLinkedSymbol*			m_symbol_persist_state;
	CVMLinkedSymbol*			m_symbol_restore_state;
	CVMLinkedSymbol*			m_team_class_symbol;
	CVMLinkedSymbol*			m_team_name_symbol;
	CVMLinkedSymbol*			m_team_primary_color_symbol;

	CVMLinkedSymbol*			m_old_camera_symbol;
	bool						m_old_camera_is_pvp;
	int							m_old_camera_map_load_index;

	CVMGCRoot					m_camera_instances[MAX_LOCAL_PLAYERS];
	CVMLinkedSymbol*			m_symbol_camera_get_viewport;
	ScriptEventListener*		m_camera_event_listeners[MAX_LOCAL_PLAYERS];
	
	CVMGCRoot					m_hud_instance;
	ScriptEventListener*		m_hud_event_listener;
	
	Canvas						m_canvas;
	CVMGCRoot					m_canvas_instance;

	bool						m_is_pvp;

	std::vector<GameOverBonus>  m_game_over_bonuses;


	CVMGCRoot					m_persistent_state_object;
	bool						m_persistent_state_stored;

	int							m_camera_count;

	int							m_game_over_score;

protected:
	void Sync(bool full);
	
public:
	GameMode();
	~GameMode();

	CVMObjectHandle				Get_Camera(int user_index);
	int							Get_Camera_Count();
	CVMObjectHandle				Get_HUD();
	ScriptedActor*				Get_Game_Mode();

	bool						Is_PVP();

	int							Get_Active_Net_ID();
	void						Set_Active_Net_ID(int id);
	void						Start_Map();
	void						Tick(const FrameTime& time);

	void						Reset_Game_Over();
	void						Add_Game_Over_Bonus(std::string name, int value);
	void						Show_Game_Over(std::string title, std::string destination);
	int							Get_Game_Over_Score();
	void						Set_Game_Over_Score(int val);

	std::vector<std::string>&	Get_Scoreboard_Columns();
	std::vector<GameModeTeam>&	Get_Teams();

	GameModeTeam*				Get_Team_By_Index(int index);

	std::string					Get_Stage_Text();

	void						Persist_State();
	void						Restore_State();

};

#endif

