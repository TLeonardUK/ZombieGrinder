// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_Game.h"
#include "Game/UI/Scenes/UIScene_SubMenu.h"
#include "Game/UI/Scenes/UIScene_Chat.h"
#include "Game/UI/Scenes/UIScene_VoiceChat.h"
#include "Game/UI/Scenes/UIScene_Scoreboard.h"
#include "Game/UI/Scenes/UIScene_VoteOverlay.h"
#include "Game/UI/Scenes/UIScene_TimeoutOverlay.h"
#include "Game/UI/Scenes/UIScene_GameOver.h"
#include "Game/UI/Scenes/UIScene_ConfirmDialog.h"
#include "Game/UI/Scenes/UIScene_Moderation.h"
#include "Game/UI/Scenes/UIScene_ModerationJudgement.h"

#include "Game/UI/Scenes/Profiler/UIScene_Profiler.h"
#include "Game/UI/Scenes/UIScene_MapLoading.h"
#include "Engine/UI/Transitions/UISlideInTransition.h"
#include "Engine/UI/Transitions/UISlideInTopLevelTransition.h"
#include "Engine/UI/Transitions/UIFadeInTransition.h"
#include "Engine/UI/Transitions/UIFadeOutTransition.h"
#include "Engine/UI/Transitions/UIFadeTransition.h"
#include "Engine/UI/Layouts/UILayoutFactory.h"

#include "Engine/Localise/Locale.h"

#include "Engine/Engine/EngineOptions.h"

#include "Game/Scene/Actors/DemoProxyActor.h"

#include "Engine/Online/OnlineVoiceChat.h"

#include "Engine/Audio/AudioRenderer.h"

#include "Game/Network/GameNetManager.h"
#include "Game/Network/GameNetClient.h"

#include "Game/Scripts/GameVirtualMachine.h"
#include "Game/Scripts/ScriptEventListener.h"

#include "Engine/Scene/Pathing/PathManager.h"

#include "Game/Game/ChatManager.h"
#include "Game/Game/VoteManager.h"
#include "Game/Game/Votes/EditModeVote.h"

#include "Engine/Platform/Platform.h"
#include "Engine/Display/GfxDisplay.h"

#include "Engine/Engine/GameEngine.h"
#include "Engine/Scene/Scene.h"

#include "Engine/Input/Input.h"
#include "Engine/UI/UIManager.h"

#include "Engine/Renderer/RenderPipeline.h"

#include "Game/UI/Scenes/Editor/UIScene_Editor.h"
#include "Game/Network/ActorReplicator.h"
#include "Game/Runner/Game.h"
#include "Game/Runner/GameMode.h"

UIScene_Game::UIScene_Game()
	: m_closing(false)
	, m_hud_alpha(1.0f)
	, m_overlay_enabled(true)
	, m_demo_spectate_platform_id(-1)
	, m_demo_spectate_view(0.0f, 0.0f, 0.0f, 0.0f)
	, m_demo_target_view(0.0f, 0.0f, 0.0f, 0.0f)
	, m_demo_freecam_acceleration(0.0f)
	, m_demo_initial_camera_snap(false)
	, m_demo_finish_dialog_open(false)
{
	Set_Layout("game");

	// Setup overlays.
	m_chat_overlay = new UIScene_Chat(this, true);
	m_vote_overlay = new UIScene_VoteOverlay(this);
	m_timeout_overlay = new UIScene_TimeoutOverlay();
	m_voice_chat_overlay = new UIScene_VoiceChat();
	//m_hud_canvas = new Canvas();

	Game::Get()->Set_Game_Input_Enabled(true);
	Game::Get()->Set_Update_Game(true);
}

UIScene_Game::~UIScene_Game()
{
	// Unload the map.
	ActorReplicator::Get()->Restart();
	GameEngine::Get()->Get_Scene()->Unload();

	// Stop all in-game tracked audio.
	AudioRenderer::Get()->Stop_All_Tracked();

	// Pop off the maps BGM.
	GameEngine::Get()->Pop_Map_BGM();

	// Kill off overlays.
	SAFE_DELETE(m_chat_overlay);
	SAFE_DELETE(m_vote_overlay);
	SAFE_DELETE(m_timeout_overlay);
	SAFE_DELETE(m_voice_chat_overlay);
	//SAFE_DELETE(m_hud_canvas);

	Game::Get()->Set_Update_Game(false);
	Game::Get()->Set_Map_Restricted_Mode(false);
}

void UIScene_Game::Set_Overlay_Enabled(bool val)
{
	m_overlay_enabled = val;
}

bool UIScene_Game::Should_Tick_When_Not_Top()
{
	return true;
}

UIScene_Chat* UIScene_Game::Get_Chat_Overlay()
{
	return m_chat_overlay;
}

const char* UIScene_Game::Get_Name()
{
	return "UIScene_Game";
}

bool UIScene_Game::Should_Render_Lower_Scenes()
{
	return false;
}

bool UIScene_Game::Should_Render_Lower_Scenes_Background()
{
	return false;
}

UIScene* UIScene_Game::Get_Background(UIManager* manager)
{
	return NULL;//manager->Retrieve_Persistent_Scene("UIScene_GameBackground");
}

void UIScene_Game::Enter(UIManager* manager)
{
}	

void UIScene_Game::Exit(UIManager* manager)
{
//	GameCamera* camera = static_cast<GameCamera*>(Game::Get()->Get_Camera(CameraID::Game));
//	camera->Set_Enabled(false);
}	


void UIScene_Game::Recieve_Event(UIManager* manager, UIEvent e)
{
	switch (e.Type)
	{
	case UIEventType::Dialog_Close:
	{
		if (m_demo_finish_dialog_open)
		{
			UIScene_ConfirmDialog* dialog = dynamic_cast<UIScene_ConfirmDialog*>(e.Scene);
			if (dialog)
			{
				if (dialog->Get_Selected_Index() == 0)
				{
					DemoManager::Get()->Restart();
				}
				else
				{
					manager->Go(UIAction::Push(new UIScene_ModerationJudgement(), new UISlideInTopLevelTransition()));
				}
				m_demo_finish_dialog_open = false;
			}
		}
		break;
	}
	}
}

void UIScene_Game::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	GameNetClient* client = GameNetManager::Get()->Game_Client();
	if (client != NULL)
	{
		bool is_topmost			= (manager->Get_Top_Scene_Index() == scene_index);
		bool is_editor_mode		= client->Get_Server_State().In_Editor_Mode;
		bool in_editor_top_most	= stricmp(manager->Get_Scene_From_Top()->Get_Name(), "UIScene_Editor") == 0;//(manager->Get_Scene_By_Name("UIScene_Editor") != NULL);

		// Lost connection to server?
		if (Game::Get()->Check_Network_Disconnect())
		{
			return;
		}

		// Sub menus?
		if (is_topmost)
		{
			//static int msgidx = 0; 
			//std::string msg = StringHelper::Format("Message %i - Derp Derp Derp", msgidx++);
			//ChatManager::Get()->Parse(msg);

			// Open the main sub-menu.
			if (Input::Get()->Was_Pressed_By_Any(OutputBindings::Escape) ||
				Input::Get()->Was_Pressed(InputBindings::Keyboard_Escape))
			{
				manager->Play_UI_Sound(UISoundType::Enter_SubMenu);
				manager->Go(UIAction::Push(new UIScene_SubMenu(), new UISlideInTransition()));
				return;
			}

			if (!DemoManager::Get()->Is_Playing())
			{
				// Open the chat menu.
				if (Input::Get()->Was_Pressed_By_Any(OutputBindings::Chat))
				{
					manager->Play_UI_Sound(UISoundType::Small_Click);
					manager->Go(UIAction::Push(new UIScene_Chat(this, false), new UIFadeInTransition()));
					return;
				}

				// Open the scoreboard.
				if (Input::Get()->Was_Pressed_By_Any(OutputBindings::Scoreboard))
				{
					manager->Play_UI_Sound(UISoundType::Small_Click);
					manager->Go(UIAction::Push(new UIScene_Scoreboard(), new UIFadeInTransition()));
					return;
					}

				// Open the profiler.
	#ifndef MASTER_BUILD
				if (Input::Get()->Was_Pressed(InputBindings::Keyboard_F1))
				{
					manager->Play_UI_Sound(UISoundType::Small_Click);
					manager->Go(UIAction::Push(new UIScene_Profiler()));
					return;
				}
				if (Input::Get()->Was_Pressed(InputBindings::Keyboard_F2))
				{
					manager->Play_UI_Sound(UISoundType::Small_Click);
					VoteManager::Get()->Call_Vote(new EditModeVote());
					return;
				}
				if (Input::Get()->Was_Pressed(InputBindings::Keyboard_F3))
				{
					manager->Play_UI_Sound(UISoundType::Small_Click);
					CollisionManager::Get()->Set_Show_Collision(!CollisionManager::Get()->Get_Show_Collision());
					return;
				}
				if (Input::Get()->Was_Pressed(InputBindings::Keyboard_F4))
				{
					manager->Play_UI_Sound(UISoundType::Small_Click);
					PathManager::Get()->Set_Show_Paths(!PathManager::Get()->Get_Show_Paths());
					return;
				}
	#endif
			}

			// Switch to editor?
			if (is_editor_mode)
			{
				manager->Go(UIAction::Push(new UIScene_Editor()));
			}
		}

		// Has map changed? Do we need to load the next one?
		if (NetManager::Get()->Is_Map_Load_Pending() && !m_closing && manager->Get_Scene_By_Type<UIScene_MapLoading*>() == NULL)
		{
			DBG_LOG("Map load pending - replacing game scene with loading scene.");

			// Pop all scenes above us.
			manager->Go(UIAction::Pop(NULL, "UIScene_Game"));

			// Get loadin'
			manager->Go(UIAction::Replace(new UIScene_MapLoading(), new UISlideInTransition()));

			m_closing = true;

			return;
		}

		// Tick overlays.
		m_chat_overlay->Tick(time, manager, scene_index);
	
		if (VoteManager::Get()->Get_Active_Vote() != NULL)
		{
			m_vote_overlay->Tick(time, manager, scene_index);
		}

		m_voice_chat_overlay->Tick(time, manager, scene_index);
		m_timeout_overlay->Tick(time, manager, scene_index);
	}

	UIScene::Tick(time, manager, scene_index);
}

void UIScene_Game::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	// Draw background.
//	manager->Retrieve_Persistent_Scene("UIScene_GameBackground")->Draw(time, manager, scene_index);

	UIScene::Draw(time, manager, scene_index);
	
	RenderPipeline* pipeline = RenderPipeline::Get();

	// Skip all this if in editor.
	GameNetClient* client	= GameNetManager::Get()->Game_Client();
	bool in_editor_mode		= client != NULL && client->Get_Server_State().In_Editor_Mode;
	bool in_game_over		= manager->Get_Scene_By_Type<UIScene_GameOver*>() != NULL;

	// Overlays are not effects by global alpha.
	Color original = pipeline->Get_Render_Batch_Global_Color();
	pipeline->Set_Render_Batch_Global_Color(Color::White);

	float target_hud_alpha = 1.0f;
	if (in_game_over == true)
	{
		target_hud_alpha = 0.0f;
	}

	m_hud_alpha = Math::Lerp(m_hud_alpha, target_hud_alpha, 2.0f * time.Get_Delta());
	
	if (!in_editor_mode)
	{
		CVMObjectHandle canvas_instance = Game::Get()->Get_Script_Canvas();
		Canvas* canvas = reinterpret_cast<Canvas*>(canvas_instance.Get()->Get_Meta_Data());

		canvas->Set_Screen_Camera(Game::Get()->Get_Camera(CameraID::UI));

		bool bInReply = DemoManager::Get()->Is_Playing();

		// Draw HUD for each camera.
		int camera_count = bInReply ? 1 :Game::Get()->Get_Game_Mode()->Get_Camera_Count();
		for (int i = 0; i < camera_count; i++)
		{
			Camera* camera = Game::Get()->Get_Camera((CameraID::Type)(CameraID::Game1 + i));
			Rect2D screen_viewport = camera->Get_Screen_Viewport();			

			// Draw the HUD.
			canvas->Set_Camera(camera);
			canvas->Set_Bounds(screen_viewport);
			canvas->Set_Viewport(screen_viewport);

			pipeline->Set_Render_Batch_Global_Color(Color(255.0f, 255.0f, 255.0f, 255.0f * m_hud_alpha));
			pipeline->Set_Render_Batch_Scissor_Rectangle(screen_viewport);
			pipeline->Set_Render_Batch_Scissor_Test(true);

			if (bInReply)
			{
				Draw_Demo_Replay_Camera(time, canvas);
			}
			else
			{
				ScriptEventListener::Fire_On_Draw_Camera_HUD(canvas_instance, i);
			}

			pipeline->Set_Render_Batch_Global_Color(Color::White);
			pipeline->Set_Render_Batch_Scissor_Test(false);
		}
		
		// Draw the HUD.
		Camera* hud_cam = Game::Get()->Get_Camera(CameraID::UI);
		Rect2D original_viewport = hud_cam->Get_Screen_Viewport();
		hud_cam->Set_Screen_Viewport(Rect2D(0, 0, GfxDisplay::Get()->Get_Width(), GfxDisplay::Get()->Get_Height()));

		canvas->Set_Camera(hud_cam);
		canvas->Set_Bounds(Rect2D(0, 0, GfxDisplay::Get()->Get_Width(), GfxDisplay::Get()->Get_Height()));
		canvas->Set_Viewport(Rect2D(0, 0, GfxDisplay::Get()->Get_Width(), GfxDisplay::Get()->Get_Height()));

		pipeline->Set_Render_Batch_Global_Color(Color(255.0f, 255.0f, 255.0f, 255.0f * m_hud_alpha));
		pipeline->Set_Render_Batch_Scissor_Rectangle(hud_cam->Get_Screen_Viewport());
		pipeline->Set_Render_Batch_Scissor_Test(true);

		if (bInReply)
		{
			Draw_Demo_Replay_Screen(time, canvas);
		}
		else
		{
			ScriptEventListener::Fire_On_Draw_HUD(canvas_instance);
		}

		pipeline->Set_Render_Batch_Global_Color(Color::White);
		pipeline->Set_Render_Batch_Scissor_Test(false);

		hud_cam->Set_Screen_Viewport(original_viewport);
	}

	// Draw overlays.
	if (m_overlay_enabled == true)
	{
		Draw_Overlays(time, manager, scene_index);
	}

	// Reset alpha.
	pipeline->Set_Render_Batch_Global_Color(original);
}

void UIScene_Game::Draw_Demo_Replay_Camera(const FrameTime& time, Canvas* canvas)
{
	const float Name_Bar_Height = 0.03f;
	const float Name_Bar_Padding = 0.01f;

	Vector2 ui_scale = GameEngine::Get()->Get_UIManager()->Get_UI_Scale();
	float gfx_width = (float)GfxDisplay::Get()->Get_Width();
	float gfx_height = (float)GfxDisplay::Get()->Get_Height();

	// Draw usernames over each player.
	std::vector<Actor*> actors = Game::Get()->Get_Scene()->Get_Actors();
	for (std::vector<Actor*>::iterator iter = actors.begin(); iter != actors.end(); iter++)
	{
		DemoProxyActor* actor = dynamic_cast<DemoProxyActor*>(*iter);
		if (actor && actor->Get_Demo_Player().Username != "")
		{
			float padding = 8.0f * ui_scale.Y;
			Rect2D world_bb = actor->Get_World_Bounding_Box();
			Rect2D screen_bb = canvas->World_To_Screen(world_bb);

			float font_height = gfx_height * Name_Bar_Height;
			float font_padding = gfx_width * Name_Bar_Padding;
			float font_vert_padding = font_padding * 0.25f;
			float pip_size = font_height * 0.5f;

			Vector2 name_pos = Vector2(screen_bb.X + (screen_bb.Width * 0.5f), screen_bb.Y);
			name_pos.Y += padding;

			std::string username = actor->Get_Demo_Player().Username;
			Vector2 text_size = canvas->Calculate_Text_Size(username.c_str(), 16.0f);
			Rect2D name_box = Rect2D
			(
				name_pos.X - (text_size.X * 0.5f) - font_padding, 
				name_pos.Y - (text_size.Y * 0.5f),
				text_size.X + (font_padding * 2.0f), 
				text_size.Y
			);
			Rect2D pip_box = Rect2D
			(
				name_box.X + (name_box.Width * 0.5f) - (pip_size * 0.5f), 
				name_box.Y + name_box.Height - (pip_size * 0.4f), 
				pip_size, 
				pip_size
			);
			Rect2D text_box = Rect2D
			(
				name_box.X, 
				name_box.Y + font_vert_padding, 
				name_box.Width, 
				name_box.Height - (font_vert_padding * 2.0f)
			);

			canvas->Set_Color(Color::White);
			canvas->Draw_Frame_Box("game_hud_messagebubble_borders_#", name_box, ui_scale.Y);
			canvas->Draw_Frame("game_hud_messagebubble_arrow", pip_box);
			canvas->Set_Font_Shadowed(true);
			canvas->Draw_Text(username.c_str(), text_box, font_height, TextAlignment::Center, TextAlignment::ScaleToFit);
		}
	}
}

void UIScene_Game::Draw_Demo_Replay_Screen(const FrameTime& time, Canvas* canvas)
{
	// This is all pretty blah, this should go in game_hud or somewhere else.

	// Do update.
	std::vector<DemoPlayer> players = DemoManager::Get()->Get_Active_Players();
	bool bSnapView = false;
	bool bForceChange = false;

	// If player has died, move to next.
	if (m_demo_spectate_platform_id != 0)
	{
		bool bFound = false;

		u64 available_id = 0;

		for (std::vector<DemoPlayer>::iterator iter = players.begin(); iter != players.end(); iter++)
		{
			DemoPlayer& player = *iter;
			if (player.Platform_ID == m_demo_spectate_platform_id)
			{
				bFound = true;
				break;
			}

			available_id = player.Platform_ID;
		}

		if (!bFound)
		{
			m_demo_spectate_platform_id = available_id;
			bSnapView = true;
		}
	}
	else
	{
		if (players.size() > 0 && !m_demo_initial_camera_snap)
		{
			bForceChange = true;
			m_demo_initial_camera_snap = true;
			bSnapView = true;
		}
	}

	// Switch camera.
	if (Input::Get()->Was_Pressed_By_Any(OutputBindings::Fire) || bForceChange)
	{
		// Freecam -> Player Cam
		if (m_demo_spectate_platform_id == 0)
		{
			if (players.size() > 0)
			{
				m_demo_spectate_platform_id = players[0].Platform_ID;
			}
		}
		// Player cam -> Next player or freecam.
		else
		{
			unsigned int index = 0;
			for (unsigned int i = 0; i < players.size(); i++)
			{
				DemoPlayer& player = players[i];
				if (player.Platform_ID == m_demo_spectate_platform_id)
				{
					index = i;
					break;
				}
			}

			if (index < players.size() - 1)
			{
				m_demo_spectate_platform_id = players[index + 1].Platform_ID;
			}
			else
			{
				m_demo_spectate_platform_id = 0;
			}
		}

		bSnapView = true;
		DBG_LOG("Cycled to %i", m_demo_spectate_platform_id);
	}
	else if (Input::Get()->Was_Pressed_By_Any(OutputBindings::AltFire))
	{
		DemoManager::Get()->Restart();
	}
	else if (Input::Get()->Was_Pressed_By_Any(OutputBindings::Interact))
	{
		DemoManager::Get()->Toggle_Pause();
	}
	else if (Input::Get()->Is_Down_By_Any(OutputBindings::Left))
	{
		DemoManager::Get()->Scrub(-1);
	}
	else if (Input::Get()->Is_Down_By_Any(OutputBindings::Right))
	{
		DemoManager::Get()->Scrub(1);
	}

	if (!DemoManager::Get()->Is_Paused() && DemoManager::Get()->Get_Current_Time() == DemoManager::Get()->Get_Total_Time() && Game::Get()->Get_Active_Demo() != NULL && !m_demo_finish_dialog_open)
	{
		GameEngine::Get()->Get_UIManager()->Go(UIAction::Push(new UIScene_ConfirmDialog(S("#moderation_download_judgement_dialog"), S("#menu_no"), S("#menu_yes")), new UIFadeInTransition()));
		m_demo_finish_dialog_open = true;
	}

	DemoPlayer spectate_player;
	for (std::vector<DemoPlayer>::iterator iter = players.begin(); iter != players.end(); iter++)
	{
		DemoPlayer& player = *iter;
		if (player.Platform_ID == m_demo_spectate_platform_id)
		{
			spectate_player = player;			
			break;
		}
	}

	// Do render.
	float gfx_width = (float)GfxDisplay::Get()->Get_Width();
	float gfx_height = (float)GfxDisplay::Get()->Get_Height();
	Vector2 ui_scale = GameEngine::Get()->Get_UIManager()->Get_UI_Scale();
	float delta_t = time.Get_Delta();
	float delta_seconds_t = time.Get_Delta_Seconds();

	float aspect = gfx_width / gfx_height;

	float res_y = ((float)*EngineOptions::render_game_height);
	float res_x = res_y * aspect;

	const float Freecam_Acceleration_Max = 20.0;
	const float Freecam_Acceleration_Lerp = 0.6;

	// Move freecam.
	if (m_demo_spectate_platform_id == 0)
	{
		float accel_x = (Input::Get()->Is_Down(OutputBindings::Right) ? 1.0f : 0.0f) - (Input::Get()->Is_Down(OutputBindings::Left) ? 1.0f : 0.0f);
		float accel_y = (Input::Get()->Is_Down(OutputBindings::Down) ? 1.0f : 0.0f) - (Input::Get()->Is_Down(OutputBindings::Up) ? 1.0f : 0.0f);

		float target_acceleration = 0.0;
		if (accel_x != 0.0f || accel_y != 0.0f)
		{
			target_acceleration = Freecam_Acceleration_Max;
		}

		m_demo_freecam_acceleration = Math::Lerp(m_demo_freecam_acceleration, target_acceleration, Freecam_Acceleration_Lerp * delta_t);

		float move_x = (accel_x * m_demo_freecam_acceleration) * delta_t;
		float move_y = (accel_y * m_demo_freecam_acceleration) * delta_t;

		m_demo_target_view.X += move_x;
		m_demo_target_view.Y += move_y;
	}
	// Center on player.
	else
	{
		if (spectate_player.Active_Actor != NULL)
		{
			Vector3 center = spectate_player.Active_Actor->Get_World_Center();
			m_demo_target_view.X = center.X - (res_x * 0.5f);
			m_demo_target_view.Y = center.Y - (res_y * 0.5f);
			m_demo_target_view.Width = res_x;
			m_demo_target_view.Height = res_y;
		}
	}

	// Lock to map bounds.
	Rect2D scene_bounds = GameEngine::Get()->Get_Scene()->Get_Boundries();
	m_demo_target_view = m_demo_target_view.ConstrainInside(scene_bounds);

	// Lerp to new viewport.
	float lerp_factor = 0.8f * delta_t;
	if (bSnapView)
	{
		lerp_factor = 1.0f;
	}
	m_demo_spectate_view.X = Math::Lerp(m_demo_spectate_view.X, m_demo_target_view.X, lerp_factor);
	m_demo_spectate_view.Y = Math::Lerp(m_demo_spectate_view.Y, m_demo_target_view.Y, lerp_factor);
	m_demo_spectate_view.Width = Math::Lerp(m_demo_spectate_view.Width, m_demo_target_view.Width, lerp_factor);
	m_demo_spectate_view.Height = Math::Lerp(m_demo_spectate_view.Height, m_demo_target_view.Height, lerp_factor);

	// Setup audio.
	AudioRenderer::Get()->Set_Listener_Count(1);
	AudioRenderer::Get()->Set_Listener_Position(0, Vector3(m_demo_spectate_view.Center().X, m_demo_spectate_view.Center().Y, 0.0f));

	// Center camera on player.
	for (int i = 1; i < MAX_LOCAL_PLAYERS; i++)
	{
		Camera* camera = Game::Get()->Get_Camera((CameraID::Type)(CameraID::Game1 + i));
		camera->Set_Enabled(false);
	}

	Camera* camera = Game::Get()->Get_Camera(CameraID::Game1);
	camera->Set_Screen_Viewport(Rect2D(0.0f, 0.0f, gfx_width, gfx_height));
	camera->Set_Position(Vector3(m_demo_spectate_view.X, m_demo_spectate_view.Y, 0.0f));
	camera->Set_Viewport(Rect2D(0.0f, 0.0f, m_demo_spectate_view.Width, m_demo_spectate_view.Height));
	camera->Set_Enabled(true);

	// Draw playing information.
	Rect2D top_bar(0.0f, 0.0f, gfx_width, gfx_height * 0.1f);
	Rect2D bottom_bar(0.0f, gfx_height - (gfx_height * 0.1f), gfx_width, gfx_height * 0.1f);
	Rect2D top_bar_text(top_bar.X, top_bar.Y + (top_bar.Height * 0.25f), top_bar.Width, top_bar.Height * 0.5f);
	Rect2D bottom_bar_text(bottom_bar.X, bottom_bar.Y + (bottom_bar.Height * 0.25f), bottom_bar.Width, bottom_bar.Height * 0.5f);

	canvas->Set_Color(Color::Black);
	canvas->Draw_Rect(top_bar);
	canvas->Draw_Rect(bottom_bar);
	canvas->Set_Color(Color::White);

	if (m_demo_spectate_platform_id != 0)
	{
		canvas->Draw_Markup_Text(StringHelper::Format(S("#hud_spectating"), canvas->Escape_Markup(spectate_player.Username.c_str()).c_str()).c_str(), top_bar_text, 16.0f, TextAlignment::Center, TextAlignment::ScaleToFit);
	}
	else
	{
		canvas->Draw_Markup_Text(S("#hud_free_camera"), top_bar_text, 16.0f, TextAlignment::Center, TextAlignment::ScaleToFit);
	} 

	int current_min = DemoManager::Get()->Get_Current_Time() / 60;
	int current_sec = DemoManager::Get()->Get_Current_Time() % 60;
	int total_min = DemoManager::Get()->Get_Total_Time() / 60;
	int total_sec = DemoManager::Get()->Get_Total_Time() % 60;

	if (!DemoManager::Get()->Is_Paused())
	{
		canvas->Draw_Markup_Text(StringHelper::Format(S("#hud_cycle_cameras_demo_pause"), current_min, current_sec, total_min, total_sec).c_str(), bottom_bar_text, 16.0f, TextAlignment::Center, TextAlignment::ScaleToFit);
	}
	else
	{
		canvas->Draw_Markup_Text(StringHelper::Format(S("#hud_cycle_cameras_demo_resume"), current_min, current_sec, total_min, total_sec).c_str(), bottom_bar_text, 16.0f, TextAlignment::Center, TextAlignment::ScaleToFit);
	}
}

void UIScene_Game::Draw_Overlays(const FrameTime& time, UIManager* manager, int scene_index)
{
	if (VoteManager::Get()->Get_Active_Vote() != NULL)
	{
		m_vote_overlay->Draw(time, manager, scene_index);
	}

	// Draw overlays.
	m_voice_chat_overlay->Draw(time, manager, scene_index);
	m_timeout_overlay->Draw(time, manager, scene_index);

	// Only draw chat overlay if there is not already a chat window pushed onto the top of the scene stack.
	bool bChatOpen = true;

	if (dynamic_cast<UIScene_Chat*>(manager->Get_Scene_From_Top()) == NULL)
	{
		m_chat_overlay->Draw(time, manager, scene_index);
		bChatOpen = false;
	}
	
	OnlineVoiceChat::Get()->Set_Disabled(bChatOpen);
}
