// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_NET_USER_
#define _GAME_NET_USER_

#include "Engine/Network/NetUser.h"

#include "Game/Network/GameNetClient.h"
#include "Game/Network/GameNetUserState.h"

#include "Game/Network/Packets/GamePackets.h"

#include "Generic/Types/CircleBuffer.h"

#include "Game/Scripts/ScriptEventListener.h"

class ScriptedActor;

class GameNetUser : public NetUser
{
	MEMORY_ALLOCATOR(GameNetUser, "Network");

private:
	GameNetClient_GameState::Type m_game_state;
	GameNetUserState* m_state;

	NetPacket_C2S_ClientUserStateUpdate m_last_client_state_packet;
	double m_last_client_state_recieve_time;

	enum 
	{
		viewport_estimate_window = 5,
		outlier_viewport_threshold = 128
	};

	CircleBuffer<Vector2, viewport_estimate_window> m_viewport_velocity_buffer;

	bool m_is_synced;

	int m_map_load_index;

	bool m_full_update_pending;

	CVMGCRoot				m_script_object;
	ScriptEventListener*	m_event_listener;

	Vector3 m_last_heading_position;
	Vector3 m_heading;
	Vector3 m_normalized_heading;
	float m_heading_lerp_delta;

	ScriptedActor* m_controller;
	int m_controller_id;

	Profile* m_profile;
	Profile* m_pending_profile;
	int m_profile_change_counter;

	double m_base_tick;

	int m_initial_level;

	int m_scoreboard_rank;

protected:

public:
	virtual ~GameNetUser();
	GameNetUser(NetConnection* connection, int local_user_index, OnlineUser* online_user);
	
	void Set_Game_State(GameNetClient_GameState::Type state, bool set_by_server = false);
	GameNetClient_GameState::Type Get_Game_State();

	virtual std::string Get_Display_Username(std::string username = "");

	Vector3 Get_Heading()
	{
		return m_heading;
	}

	Vector3 Get_Heading_Position()
	{
		return m_last_heading_position;
	}

	int Get_Initial_Level()
	{
		return m_initial_level;
	}

	void Set_Scoreboard_Rank(int rank)
	{
		m_scoreboard_rank = rank;
	}
	
	Rect2D Get_Estimated_Viewport();
	void Recieve_State_Update(NetPacket_C2S_ClientUserStateUpdate* packet);

	void Reset_On_Map_Load();
	void LeftGame();

	void Update_Heading(ScriptedActor* actor);

	void Apply_Pending_Profile();
	void Set_Profile(Profile* profile);
	void Set_Pending_Profile(Profile* profile);
	Profile* Get_Profile();
	Profile* Get_Local_Profile();
	int Get_Profile_Change_Counter();

	double Get_Base_Tick();
	void Set_Base_Tick(double value);

	CVMObjectHandle Get_Script_Object();

	int Get_Scoreboard_Rank();

	void Set_Map_Load_Index(int index);

	void Set_Full_Update_Pending(bool value);
	bool Get_Full_Update_Pending();

	GameNetUserState* Get_State();

	ScriptedActor* Get_Controller();
	void Set_Controller_ID(int id);

	void Restart();
	bool Is_Synced();
	
	virtual void Tick(const FrameTime& time);

};

#endif

