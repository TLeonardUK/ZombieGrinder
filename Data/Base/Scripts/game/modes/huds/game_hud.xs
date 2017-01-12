 // -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.log;
using system.hud;
using system.scene;
using system.canvas;
using system.locale;
using game.actors.decoration.movable.turrets.turret;
using game.actors.decoration.movable.turrets.love_pylon;
using game.modes.huds.compass;
using game.modes.huds.minimap;

// TODO: Refactor this whole sorry mess. The constant recalculation of vectors is slow as fuck.

[
	Placeable(false), 
	Name("Game HUD"), 
	Description("The primary game HUD for ZG.") 
]
public class Game_HUD : HUD
{
	const float Spectate_Bar_Height = 0.1;

	const float Name_Bar_Height = 0.03;
	const float Name_Bar_Padding = 0.01;

	const float Score_Bar_Height = 0.04;
	const float Score_Bar_Padding = 0.01;

	const float Objective_Bar_Height = 0.035;
	const float Objective_Bar_Padding = 0.01;

	const float Player_HUD_Width = 0.23;
	const float Player_HUD_Height = 0.13;
	const float Player_HUD_Padding = 0.01;

	const float Message_Bar_Height = 0.125f;
	const float Message_Bar_Lerp = 15.0f;
	const float Message_Bar_Time = 5.0f;
	
	const float Boss_Bar_Width = 0.40;

	private bool bWasTryingToActivateMapLastFrame = false;

	private string m_objective_text = "";
	private string m_old_objective_text = "";
	private float m_objective_alpha = 0.0f;
	private float m_old_objective_alpha = 0.0f;
	private Vec4 m_objective_box;
	private bool m_use_new_objective_box = false;
	private bool m_objective_show_when_dead = false;

	private string m_message = "";
	private string m_message_title = "";
	private string m_message_icon = "";
	private float m_message_timer = 0.0f;
	private float m_message_height = 0.0f;
	private bool m_message_active = false;
		
	const float BOB_MIN_HEIGHT = 3.0f;
	const float BOB_HEIGHT = 32.0f;
	const float BOB_INTERVAL = 1.5f;
	const float BOB_SCALE_REDUCTION = 0.2f;
	private float m_bob_time = 0.0f;
	private int m_bob_direction = 0;
	private Vec2 m_bob_scale = new Vec2(1.0f, 1.0f);

    private float m_cutscene_intro_alpha = 0.0f;
    const float CUTSCENE_INTRO_FADE_SPEED = 10.0f;

    private float m_boss_timer = 0.0f;
    private float m_boss_gentle_movement = 0.0f;
    const float BOSS_INTRO_GENTLE_MOVEMENT_SPEED = 15.0f;

    const float BOSS_NAME_INTRO_DELAY = 1.5f;
    const float BOSS_NAME_LERP_DURATION = 0.4f;
    const float BOSS_NAME_OUTRO_DELAY = 4.0f;

    private bool m_shown_restricted_mode_message = false;
    
    private float m_map_prompt_time_since_shown = 0.0f;
    const float MAP_PROMPT_SHOW_INTERVAL = 4.0f;

    private bool m_players_have_existed = false;

    Boss Find_Boss_Pawn()
    {
        Actor[] result = Scene.Find_Actors(typeof(Boss));
        if (result.Length() > 0)
        {
            return <Boss>result[0];
        }
        else
        {
            return null;
        }
    }

	default state Idle
	{
		event void On_Draw_Camera_HUD(Canvas canvas, int camera_index)
		{ 			
            if (Cheat_Manager.No_HUD)
            {
                return;
            }

			Camera camera = Scene.Active_Game_Mode.Get_Camera(camera_index);
			bool spectating = camera.Is_Spectating();
            
            canvas.Global_Color = Vec4(255.0f, 255.0f, 255.0f, 255.0f * (1.0f - m_cutscene_intro_alpha));

			if (spectating)
			{
				Vec4 canvas_bounds 		= canvas.Bounds;
				float spectate_bar_px 	= canvas.Bounds.Height * Spectate_Bar_Height;

				// Need to boost height to improve readability in split screen.
				if (Scene.Get_Camera_Count() > 1)
				{
					spectate_bar_px *= 1.5f;
				}
		
				Vec4 top_bar_bounds 		= Vec4(canvas_bounds.X, canvas_bounds.Y, canvas_bounds.Width, spectate_bar_px);
				Vec4 bottom_bar_bounds	 	= Vec4(canvas_bounds.X, canvas_bounds.Y + (canvas_bounds.Height - spectate_bar_px), canvas.Bounds.Width, spectate_bar_px);
				Vec4 top_bar_text_bounds 	= Vec4(canvas_bounds.X, canvas_bounds.Y + (spectate_bar_px * 0.25), canvas_bounds.Width, spectate_bar_px * 0.5);
				Vec4 bottom_bar_text_bounds = Vec4(canvas_bounds.X, canvas_bounds.Y + (canvas_bounds.Height - (spectate_bar_px * 0.75)), canvas.Bounds.Width, spectate_bar_px * 0.5);
				
				// Render pip above the player.
				Draw_Usernames(canvas);
			
				// Draw turret health bars - This should be moved into turret class maybe?
				Draw_Turret_Infos(canvas);

				// Top/Bottom Bars.
				canvas.Color = Color.Black;
				canvas.Draw_Rect(top_bar_bounds);
				canvas.Draw_Rect(bottom_bar_bounds);
 
				Actor target = camera.Get_Spectate_Target();
				string target_text;

				if (target == null)
				{
					target_text = Locale.Get("#hud_free_camera");
				}
				else
				{
                    NetUser user = target.Owner;
					if (user != null)
						target_text = Locale.Get("#hud_spectating").Format(new object[] { user.Display_Username });
					else
						target_text = Locale.Get("#hud_free_camera");
				}
			  
				canvas.Color = Color.White; 
				canvas.Draw_Markup_Text(target_text, top_bar_text_bounds, 16.0, Text_Alignment.Center, Text_Alignment.ScaleToFit);
                
                string center_text = Locale.Get("#hud_cycle_cameras");

                NetUser local_user = Network.Get_Local_User_By_Index(camera_index);
                if (local_user != null && Network.Get_Local_Users().Length() <= 1)// && local_user.Is_Premium)
                {
		            Base_Game_Mode mode = <Base_Game_Mode>Scene.Active_Game_Mode;
                    if (mode.Can_Spectators_Control_Zombies)
                    {
                        if (local_user.Waiting_For_Zombie_Control)
                        {
			                center_text += "    " + Locale.Get("#hud_press_to_control_zombie_waiting");
                        }
                        else
                        {
			                center_text += "    " + Locale.Get("#hud_press_to_control_zombie");
                        }
                        Check_Control_Zombie(local_user);
                    }
                }
                 
				canvas.Draw_Markup_Text(center_text, bottom_bar_text_bounds, 16.0, Text_Alignment.Center, Text_Alignment.ScaleToFit);
            }
			else
			{
				// Render pip above the player.
				Draw_Usernames(canvas);
			
				// Draw turret health bars - This should be moved into turret class maybe?
				Draw_Turret_Infos(canvas);
				
				// Draw interaction pips.
				if (Options.Get_Bool("interaction_arrows_enabled"))
				{
					Draw_Interaction_Pips(canvas, camera);
				}

				// Draw the comapss pips.
				Compass.Draw(canvas, camera_index, m_bob_scale);
			}

            canvas.Global_Color = Vec4(255.0f, 255.0f, 255.0f, 255.0f);
		}	

        event void On_Draw_HUD(Canvas canvas)
		{ 			
            if (Cheat_Manager.No_HUD)
            {
                return;
            }

			Base_Game_Mode mode = <Base_Game_Mode>Scene.Active_Game_Mode;
			bool is_pvp = mode != null && mode.Is_PVP();
			
			bool spectating = Scene.Active_Game_Mode.Get_Camera(0).Is_Spectating();
			bool hide_general = (spectating == true && Scene.Get_Camera_Count() == 1); 

            Boss boss_pawn = Find_Boss_Pawn();

            // Work out alpha
            bool bDrawCutsceneIntro = false;
            bool bDrawBossName = false;
                  
            if (mode.bBossIntroActive)
            {
                if (!Scene.Is_Spawned(mode.BossIntroActor))
                {
                    mode.bBossIntroActive = false;
                }
                else
                {
                    bDrawBossName = true;
                }
            }

            // If intro tagged player spawns exist and
            // no players are currently spawned, then assume a cutscene is incoming.
            if (!m_players_have_existed)
            {
                if (Scene.Count_Actors_By_Type(typeof(Human_Player)) > 0)
                {
                    m_players_have_existed = true;
                }
                
        		Actor[] actors = Scene.Find_Actors(typeof(Player_Spawn));
                foreach (Player_Spawn spawn in actors)
                {
                    if (spawn.Intro_Point_Link != "")
                    {
                        bDrawCutsceneIntro = true;
                        hide_general = false;
                        m_cutscene_intro_alpha = 1.0f;
                        break;
                    }
                }
            }


            if (mode.In_Cutscene_Mode() || bDrawCutsceneIntro)
            {
                bDrawCutsceneIntro = true;
                m_cutscene_intro_alpha = Math.Lerp(m_cutscene_intro_alpha, 1.0f, CUTSCENE_INTRO_FADE_SPEED * Time.Get_Delta_Seconds());
            }
            else
            {
                m_cutscene_intro_alpha = Math.Lerp(m_cutscene_intro_alpha, 0.0f, CUTSCENE_INTRO_FADE_SPEED * Time.Get_Delta_Seconds());
				bDrawCutsceneIntro = (m_cutscene_intro_alpha > 0.01f);
            }	
            
            if (bDrawBossName)
            {
                m_boss_timer += Time.Get_Delta_Seconds();
                m_boss_gentle_movement += BOSS_INTRO_GENTLE_MOVEMENT_SPEED * Time.Get_Delta_Seconds();
            }

            canvas.Global_Color = Vec4(255.0f, 255.0f, 255.0f, 255.0f * (1.0f - m_cutscene_intro_alpha));

			// We always want to show objective when dead if tagged.
			if ((m_objective_show_when_dead && hide_general) || boss_pawn != null)
			{
				Draw_Objective(canvas, false, true, boss_pawn);
			}

			if (hide_general)
			{
                canvas.Global_Color = Vec4(255.0f, 255.0f, 255.0f, 255.0f);
				return;
			}
    
            // Show message the first time we start an unapproved map.
            if (!m_shown_restricted_mode_message)
            {                
                if (Engine.In_Map_Restricted_Mode())
                {
                    Show_Message(Locale.Get("#restricted_mode_ingame_warning"), Locale.Get("#restricted_mode_ingame_warning_title"), "tutorial_info", 3);
                    m_shown_restricted_mode_message = true;
                }
            }
			
			// Challenge completed?
			if (Challenge.Was_Completed(ChallengeTimeframe.Daily))
			{     
				Show_Message(Locale.Get("#daily_challenge_completed_description"), Locale.Get("#daily_challenge_completed_name"), "tutorial_info", 2);             
			}
			else if (Challenge.Was_Completed(ChallengeTimeframe.Weekly))
			{     
				Show_Message(Locale.Get("#weekly_challenge_completed_description"), Locale.Get("#weekly_challenge_completed_name"), "tutorial_info", 2);             
			}
			else if (Challenge.Was_Completed(ChallengeTimeframe.Monthly))
			{     
				Show_Message(Locale.Get("#monthly_challenge_completed_description"), Locale.Get("#monthly_challenge_completed_name"), "tutorial_info", 2);             
			}

			// Score only rendered in non-pvp.
			if (!is_pvp)
			{
				// Draw score at top of screen.
				Draw_Score(canvas); //4m!!!?
			}

			// Draw message HUD's
			if (Is_Message_Visible())
			{
				Draw_Message(canvas);
			}

			// Draw player HUD's
			Draw_Player_HUDS(canvas, is_pvp);

			// Draw objective text.
			if (Is_Objective_Visible() || boss_pawn != null)
			{
				Draw_Objective(canvas, !is_pvp, false, boss_pawn);
			}

			// Draw the minimap if required.
			Minimap.Draw(canvas);

			Minimap.Tick();
			Compass.Tick();
            
            canvas.Global_Color = Vec4(255.0f, 255.0f, 255.0f, 255.0f * m_cutscene_intro_alpha);

            // Draw boss intro. 
            if (bDrawCutsceneIntro)
            {
				Vec4 canvas_bounds 		    = canvas.Bounds;
				float spectate_bar_px 	    = canvas.Bounds.Height * Spectate_Bar_Height;

				Vec4 top_bar_bounds 		= Vec4(canvas_bounds.X, canvas_bounds.Y, canvas_bounds.Width, spectate_bar_px);
				Vec4 bottom_bar_bounds	 	= Vec4(canvas_bounds.X, canvas_bounds.Y + (canvas_bounds.Height - spectate_bar_px), canvas.Bounds.Width, spectate_bar_px);
				
				// Top/Bottom Bars.
				canvas.Color = Color.Black;
				canvas.Draw_Rect(top_bar_bounds);
				canvas.Draw_Rect(bottom_bar_bounds);

                // Draw the "Introducing xxx...." message.
                if (bDrawBossName)
                {
                    float boss_text_delta = Math.Clamp((m_boss_timer - BOSS_NAME_INTRO_DELAY) / BOSS_NAME_LERP_DURATION, 0.0f, 1.0f);

                    if (m_boss_timer >= BOSS_NAME_OUTRO_DELAY)
                    {
                        boss_text_delta = Math.Clamp((m_boss_timer - BOSS_NAME_OUTRO_DELAY) / BOSS_NAME_LERP_DURATION, 0.0f, 1.0f);
                    }
    
                    string boss_name = mode.BossIntroActor.Get_Boss_Intro_Text();
                    Vec2 boss_text_size = canvas.Calculate_Markup_Text_Size(boss_name, 12.0f * canvas.UI_Scale);

                    Vec4 target_area = Vec4(
                        canvas_bounds.X + (canvas_bounds.Width * 0.5f) - (boss_text_size.X * 0.5f) + m_boss_gentle_movement, 
                        bottom_bar_bounds.Y - boss_text_size.Y - (canvas.Bounds.Height * 0.1f),
                        boss_text_size.X, 
                        boss_text_size.Y
                    );

                    Vec4 text_pos = Math.SmoothStep(Vec4(-target_area.Z, target_area.Y, target_area.Z, target_area.W), target_area, boss_text_delta);
                
                    if (m_boss_timer >= BOSS_NAME_OUTRO_DELAY)
                    {
                        text_pos = Math.SmoothStep(target_area, Vec4(canvas.Bounds.X + canvas.Bounds.Width, target_area.Y, target_area.Z, target_area.W), boss_text_delta);
                    }

                    canvas.Color = Color.White;
                    canvas.Draw_Markup_Text(boss_name, text_pos, 12.0f * canvas.UI_Scale);
                }
            }

            canvas.Global_Color = Vec4(255.0f, 255.0f, 255.0f, 255.0f);
		}
	}

    public void Check_Control_Zombie(NetUser master_user)
    {
        // Attempt to control zombie.            
        if (master_user != null)// && master_user.Is_Premium)
        {
			if (Input.Was_Pressed(master_user, OutputBindings.Interact))
			{
		        Base_Game_Mode mode = <Base_Game_Mode>Scene.Active_Game_Mode;
                if (mode.Can_Spectators_Control_Zombies)
                {
                    if (!master_user.Waiting_For_Zombie_Control)
                    {
                        mode.Request_Zombie_Control(master_user);
                    }
                }
            }
        }
    }

	public void Set_Objective_Text(string text, bool use_checkoff_animation = false, bool show_when_dead = false)
	{
		if (use_checkoff_animation == true)
		{
			m_old_objective_text = m_objective_text;
			m_old_objective_alpha = 1.0f;
		}
		if (m_objective_text == "")
		{
			m_use_new_objective_box = true;
		}
		m_objective_text = text;
		m_objective_show_when_dead = show_when_dead;
	}
	
	private void Draw_Objective(Canvas canvas, bool score_visible, bool when_dead = false, Boss boss_pawn = null)
	{
		float delta_t = Time.Get_Delta();
		Base_Game_Mode mode = <Base_Game_Mode>Scene.Active_Game_Mode;
		 
		float font_height = canvas.Bounds.Height * Objective_Bar_Height;
		float font_padding = canvas.Bounds.Width * Objective_Bar_Padding;
		float font_vert_padding = font_padding * 0.1f;
		float y_offset = 0.0f;

        Text_Alignment text_align = Text_Alignment.Center;

		if (score_visible)
		{
			y_offset = (canvas.Bounds.Height * Score_Bar_Height) * 1.75f;
		}

		if (when_dead)
		{
			float spectate_bar_px 	= canvas.Bounds.Height * Spectate_Bar_Height;
			y_offset = spectate_bar_px;
		}

        string new_text = m_objective_text;
        string old_text = m_old_objective_text;

        if (boss_pawn != null)
        {
            old_text = new_text;
            new_text = Locale.Get("#hud_boss"); // todo
            text_align = Text_Alignment.Left;
        }

		Vec2 score_text_size = canvas.Calculate_Markup_Text_Size(new_text, font_height);

		float name_box_width = score_text_size.X + (font_padding * 2.0f);

        if (boss_pawn != null)
        {
            name_box_width += (canvas.Bounds.Width * Boss_Bar_Width); // todo
        }

		Vec4 name_box = Vec4((canvas.Bounds.Width * 0.5) - (name_box_width * 0.5f), y_offset, name_box_width, score_text_size.Y);
		
		// Lerp alpha depending on if objective is set or not.
		float target_alpha = (new_text == "" && boss_pawn == null ? 0.0f : 1.0f);
		m_objective_alpha = Math.Lerp(m_objective_alpha, target_alpha, Math.Clamp(1.0f * delta_t, 0.0, 1.0f));
		m_objective_box = Math.Lerp(m_objective_box, name_box, Math.Clamp(2.0f * delta_t, 0.0, 1.0f));
		
		if (m_use_new_objective_box)
		{
			m_objective_box = name_box;
			m_use_new_objective_box = false;
		}

		Vec4 name_text_box = Vec4(m_objective_box.X + font_padding, m_objective_box.Y + (font_vert_padding * 0.5f), m_objective_box.Width - (font_padding * 2.0f), m_objective_box.Height - (font_vert_padding * 2.0f));
		
		// Fade old objective out and new one in.
		float text_alpha = m_objective_alpha;
		if (old_text != "")
		{
			m_old_objective_alpha = Math.Lerp(m_old_objective_alpha, 0.0f, Math.Clamp(1.0f * delta_t, 0.0, 1.0f));
			text_alpha = 1.0f - m_old_objective_alpha;

			if (m_old_objective_alpha <= 0.01f)
			{
				m_old_objective_text = "";
			}
		}

		// Drrraaaaaw.
		canvas.Color = Vec4(255.0f, 255.0f, 255.0f, 255.0f * m_objective_alpha);
		if (score_visible)
		{
			canvas.Draw_Frame_Box("game_hud_score_bg_full_#", m_objective_box, canvas.UI_Scale);
		}
		else
		{
			canvas.Draw_Frame_Box("game_hud_score_bg_#", m_objective_box, canvas.UI_Scale);
		}
		canvas.Color = Vec4(255.0f, 255.0f, 255.0f, 255.0f * text_alpha);
		canvas.Font_Shadowed = true;
		canvas.Draw_Markup_Text(new_text, name_text_box, font_height, text_align, Text_Alignment.ScaleToFit);
		
        // Boss healthbar.
        if (boss_pawn != null)
        {
            float health_delta = boss_pawn.Health / boss_pawn.Max_Health;
            Vec4 health_box = Vec4(m_objective_box.X + score_text_size.X + (font_padding * 1.2f), m_objective_box.Y + (font_padding * 0.75f), m_objective_box.Width - score_text_size.X - (font_padding * 2.4f), m_objective_box.Height - (font_padding * 1.5f));

		    canvas.Color = Vec4(255.0f, 255.0f, 255.0f, 255.0f);
		    canvas.Draw_Frame_Box("game_hud_messagebubble_borders_#", health_box, canvas.UI_Scale);

		    if (health_delta > 0.0f)
		    {
			    canvas.Color = Vec4(255.0f, 14.0f, 14.0f, 255.0f);
			    canvas.Draw_Frame_Box("game_hud_messagebubble_borders_#", Vec4(health_box.X, health_box.Y, health_box.Width * health_delta, health_box.Height), canvas.UI_Scale);
		    }
        }

		if (old_text != "")
		{
			canvas.Color = Vec4(255.0f, 255.0f, 255.0f, 255.0f * m_old_objective_alpha);
			canvas.Draw_Markup_Text(old_text, name_text_box, font_height, Text_Alignment.Center, Text_Alignment.ScaleToFit);
		}
	}

	public bool Is_Objective_Visible()
	{
		return (m_old_objective_text != "" || m_objective_text != ""); 
	}

    private bool Screen_Rect_Overlaps_Pawns(Canvas canvas, Vec4 bbox)
    {
        Actor[] pawns = Scene.Find_Actors_In_Area(typeof(Pawn), canvas.Screen_BBox_To_World(bbox).Inflate(2.0f, 2.0f));

        foreach (Pawn pwn in pawns)
        {
            Vec4 poss_bbox = canvas.World_BBox_To_Screen(pwn.World_Bounding_Box).Inflate(0.5f, 0.5f);
            if (poss_bbox.Intersects(bbox))
            {
                return true;
            }
        }

        return false;
    }

	private void Draw_Score(Canvas canvas)
	{
		Base_Game_Mode mode = <Base_Game_Mode>Scene.Active_Game_Mode;
		//Log.Write("Draw_Score("+mode.GetType().Name+")="+mode.Score);

		string score_text = (<string>mode.Display_Score);
		if (score_text.Length() < 9)
		{
			int extra_padding = 9 - score_text.Length();
			score_text = "[c=128,128,128]" + ("".PadLeft(extra_padding, "0")) + "[/c]" + score_text;
		}

		string multiplier_text = "x %.1f".Format(new object[] { mode.Display_Multiplier });
		
		float font_height = canvas.Bounds.Height * Score_Bar_Height;
		float mult_font_height = font_height * 0.75f;
		float font_padding = canvas.Bounds.Width * Score_Bar_Padding;
		float font_vert_padding = font_padding * 0.1f;

		Vec2 score_text_size = canvas.Calculate_Markup_Text_Size(score_text, font_height);
		Vec2 multiplier_text_size = canvas.Calculate_Text_Size(multiplier_text, mult_font_height);

		float name_box_width = score_text_size.X + multiplier_text_size.X + (font_padding * 1.5f);
		Vec4 name_box = Vec4((canvas.Bounds.Width * 0.5) - (name_box_width * 0.5f), 0.0f, name_box_width, score_text_size.Y);
		Vec4 score_text_box = Vec4(name_box.X + font_padding, name_box.Y + (font_vert_padding * 0.5f), name_box.Width - (font_padding * 2.0f), name_box.Height - (font_vert_padding * 2.0f));
		Vec4 multiplier_text_box = Vec4(name_box.X + font_padding, name_box.Y + (font_vert_padding * 0.5f) + (name_box.Height * 0.25f), name_box.Width - (font_padding * 2.0f), (name_box.Height * 0.75f) - (font_vert_padding * 4.0f));
		
		canvas.Color = Vec4(255.0f, 255.0f, 255.0f, 255.0f);
		canvas.Draw_Frame_Box("game_hud_score_bg_#", name_box, canvas.UI_Scale);		
		canvas.Font_Shadowed = true;
		canvas.Draw_Markup_Text(score_text, score_text_box, font_height, Text_Alignment.Left, Text_Alignment.ScaleToFit);
		canvas.Color = mode.Multiplier_Color;
		canvas.Draw_Text(multiplier_text, multiplier_text_box, mult_font_height, Text_Alignment.Right, Text_Alignment.ScaleToFit);
	}

	private void Draw_Username(Canvas canvas, string name, Vec2 position, float health_progress, float armor_progress, float reload_progress, bool is_premium, Pawn pwn)
	{
		float font_height = canvas.Display_Bounds.Height * Name_Bar_Height;
		float font_padding = canvas.Display_Bounds.Width * Name_Bar_Padding;
		float font_vert_padding = font_padding * 0.25f;

        float scale = canvas.UI_Scale;

        Vec2 pip_size = canvas.Get_Frame_Size("game_hud_messagebubble_arrow") * scale;

		Vec2 text_size = canvas.Calculate_Text_Size(name, font_height);
		Vec4 name_box = Vec4(position.X - (text_size.X * 0.5) - font_padding, position.Y - (text_size.Y * 0.5), text_size.X + (font_padding * 2.0f), text_size.Y);
		Vec4 pip_box = Vec4(name_box.X + (name_box.Width * 0.5f) - (pip_size.X * 0.5f), name_box.Y + name_box.Height - (2.0f * scale), pip_size.X, pip_size.Y);
		Vec4 text_box = Vec4(name_box.X, name_box.Y + font_vert_padding, name_box.Width, name_box.Height - (font_vert_padding * 2.0f));	
		Vec4 health_box = Vec4(position.X - (text_size.X * 0.5) - font_padding, position.Y - (text_size.Y * 0.5) - (font_padding), text_size.X + (font_padding * 2.0f), font_padding);
		Vec4 reload_box = Vec4(position.X - (text_size.X * 0.5) - font_padding, position.Y - (text_size.Y * 0.5) - (font_padding * 2), text_size.X + (font_padding * 2.0f), font_padding * 1);

        float alpha = 1.0f;
        if (Screen_Rect_Overlaps_Pawns(canvas, name_box))
        {
            alpha = 0.15f;
        }

        float FADE_SPEED = 10.0f;    
		float delta_t = Time.Get_Delta_Seconds();

        pwn.Hud_Element_Alpha = Math.Lerp(pwn.Hud_Element_Alpha, alpha, FADE_SPEED * delta_t);
        alpha = pwn.Hud_Element_Alpha;

		if (reload_progress > 0.0f && reload_progress < 1.0f)
		{
			canvas.Color = Vec4(255.0f, 255.0f, 255.0f, 255.0f * alpha);
			canvas.Draw_Frame_Box("game_hud_messagebubble_borders_#", reload_box, canvas.UI_Scale);
			
			canvas.Color = Vec4(14.0f, 128.0f, 255.0f, 255.0f * alpha);
			canvas.Draw_Frame_Box("game_hud_messagebubble_borders_#", Vec4(reload_box.X, reload_box.Y, reload_box.Width * reload_progress, reload_box.Height), canvas.UI_Scale);
		}

		canvas.Color = Vec4(255.0f, 255.0f, 255.0f, 255.0f * alpha);
		canvas.Draw_Frame_Box("game_hud_messagebubble_borders_#", health_box, canvas.UI_Scale);

		if (health_progress > 0.0f)
		{
			canvas.Color = Vec4(255.0f, 14.0f, 14.0f, 255.0f * alpha);
			canvas.Draw_Frame_Box("game_hud_messagebubble_borders_#", Vec4(health_box.X, health_box.Y, health_box.Width * health_progress, health_box.Height), canvas.UI_Scale);
		}
        
		if (armor_progress > 0.0f)
		{
			canvas.Color = Vec4(64.0f, 64.0f, 64.0f, 255.0f * alpha);
			canvas.Draw_Frame_Box("game_hud_messagebubble_borders_#", Vec4(health_box.X, health_box.Y, health_box.Width * armor_progress, health_box.Height), canvas.UI_Scale);
		}

		canvas.Color = Vec4(255.0f, 255.0f, 255.0f, 255.0f * alpha);
		canvas.Draw_Frame_Box("game_hud_messagebubble_borders_#", name_box, canvas.UI_Scale);
		canvas.Draw_Frame("game_hud_messagebubble_arrow", pip_box);
		
        if (is_premium)
        {
    		canvas.Color = Vec4(255.0f, 194.0f, 14.0f, 255.0f * alpha);
        }
        else
        {
    		canvas.Color = Vec4(255.0f, 255.0f, 255.0f, 255.0f * alpha);
        }
		canvas.Font_Shadowed = true;
		canvas.Draw_Text(name, text_box, font_height, Text_Alignment.Center, Text_Alignment.ScaleToFit);
	}
	
	private void Draw_Interact(Canvas canvas, bool active, float progress, string prompt, Vec2 position, float health_progress, float armor_progress, float reload_progress)
	{
		float font_height = canvas.Display_Bounds.Height * Name_Bar_Height;
		float font_padding = canvas.Display_Bounds.Width * Name_Bar_Padding;
		float font_vert_padding = font_padding * 0.25f;
		float pip_size = font_height * 0.5f;

		Vec2 text_size = canvas.Calculate_Markup_Text_Size(prompt, font_height);
		Vec4 name_box = Vec4(position.X - (text_size.X * 0.5) - font_padding, position.Y - (text_size.Y * 0.5), text_size.X + (font_padding * 2.0f), text_size.Y);
		Vec4 pip_box = Vec4(name_box.X + (name_box.Width * 0.5f) - (pip_size * 0.5f), name_box.Y + name_box.Height - (pip_size * 0.4f), pip_size, pip_size);
		Vec4 text_box = Vec4(name_box.X, name_box.Y + font_vert_padding, name_box.Width, name_box.Height - (font_vert_padding * 2.0f));
		Vec4 health_box = Vec4(position.X - (text_size.X * 0.5) - font_padding, position.Y - (text_size.Y * 0.5) - (font_padding), text_size.X + (font_padding * 2.0f), font_padding);
		Vec4 reload_box = Vec4(position.X - (text_size.X * 0.5) - font_padding, position.Y - (text_size.Y * 0.5) - (font_padding * 1.5), text_size.X + (font_padding * 2.0f), font_padding * 0.5);
	
		if (reload_progress > 0.0f && reload_progress < 1.0f)
		{
			canvas.Color = Vec4(255.0f, 255.0f, 255.0f, 255.0f);
			canvas.Draw_Frame_Box("game_hud_messagebubble_borders_#", reload_box, canvas.UI_Scale);

			canvas.Color = Vec4(14.0f, 128.0f, 255.0f, 255.0f);
			canvas.Draw_Frame_Box("game_hud_messagebubble_borders_#", Vec4(reload_box.X, reload_box.Y, reload_box.Width * reload_progress, reload_box.Height), canvas.UI_Scale);
		}
		
		canvas.Color = Vec4(255.0f, 255.0f, 255.0f, 255.0f);
		canvas.Draw_Frame_Box("game_hud_messagebubble_borders_#", health_box, canvas.UI_Scale);

		if (health_progress > 0.0f)
		{
			canvas.Color = Vec4(255.0f, 14.0f, 14.0f, 255.0f);
			canvas.Draw_Frame_Box("game_hud_messagebubble_borders_#", Vec4(health_box.X, health_box.Y, health_box.Width * health_progress, health_box.Height), canvas.UI_Scale);
		}

		if (armor_progress > 0.0f)
		{
			canvas.Color = Vec4(64.0f, 64.0f, 64.0f, 255.0f);
			canvas.Draw_Frame_Box("game_hud_messagebubble_borders_#", Vec4(health_box.X, health_box.Y, health_box.Width * armor_progress, health_box.Height), canvas.UI_Scale);
		}

		canvas.Color = Vec4(255.0f, 255.0f, 255.0f, 255.0f);
		canvas.Draw_Frame_Box("game_hud_messagebubble_borders_#", name_box, canvas.UI_Scale);

		if (progress > 0.0f)
		{
			canvas.Color = Vec4(255.0f, 194.0f, 14.0f, 255.0f);
			canvas.Draw_Frame_Box("game_hud_messagebubble_borders_#", Vec4(name_box.X, name_box.Y, name_box.Width * progress, name_box.Height), canvas.UI_Scale);
		}

		canvas.Color = Vec4(255.0f, 255.0f, 255.0f, 255.0f);
		canvas.Font_Shadowed = true;
		canvas.Draw_Markup_Text(prompt, text_box, font_height, Text_Alignment.Center, Text_Alignment.ScaleToFit);
	}

    static string[] Random_Names = new string[] {
        "Infinitus",
        "Jordizzle",
        "Travis",
        "Delko",
        "Tim"
    };

	private void Draw_Usernames(Canvas canvas)
	{
		NetUser[] users = Network.Get_Users();			

		Inventory_Pickup map_pickup = Inventory_Pickup.Get_Held_Inventory_Item_By_Type(typeof(Map_Pickup));
    
		float scale = canvas.UI_Scale;

		bool bShowingMap = false;
		//bool bWasMapButtonDown = false;

        int Random_Name_Index = 0;

		foreach (NetUser user in users)
		{
			Controller controller = user.Controller;
			if (controller != null)
			{
				Pawn possessed = controller.Possessed;
				if (possessed != null)
				{
					Human_Player human = <Human_Player>possessed;

                    float padding = 8.0f * canvas.UI_Scale;

					Vec4 bbox = canvas.World_BBox_To_Screen(possessed.World_Bounding_Box);
					Vec2 name_pos = Vec2(bbox.X + (bbox.Width * 0.5f), bbox.Y);
                    if (human != null)
                    {
                        name_pos.Y += padding;
                    }
                    else
                    {
                        name_pos.Y -= padding;
                    }
                    
                    //name_pos.Y -= ((Math.Sin(Time.Get_Ticks() * 0.002f) + 1.0f) * 0.5f) * 6.0f * scale;

					float health_delta = possessed.Health / possessed.Max_Health;
					float armor_delta = possessed.Armor / possessed.Max_Armor;
					float reload_delta = 0.0f;

					Weapon weapon = possessed.Get_Active_Weapon();
					if (weapon != null && weapon.Is_Reloading)
					{
						reload_delta = weapon.Get_Reload_Delta();
					}

					// Draw inventory item if we have one.		
					Inventory_Pickup pickup = Inventory_Pickup.Get_Inventory_Item_For_Net_ID(possessed.Net_ID);
					if (pickup != null)
					{
						Vec2 size = (canvas.Get_Frame_Size(pickup.Get_Sprite()) * canvas.UI_Scale * 1.0f) * m_bob_scale;
						canvas.Color = Vec4(255.0f, 255.0f, 255.0f, 255.0f);
						canvas.Draw_Frame(pickup.Get_Sprite(), Vec4(name_pos.X - (size.X * 0.5f), name_pos.Y - (size.Y * 0.5f), size.X, size.Y));
						name_pos.Y -= (size.Y) ;//- (padding * 1.0f);
					}

					// Draw prompt?
					bool prompt_drawn = false;
					if (controller.Get_Interact_Prompt() != "" && user.Is_Local)
					{						
						if (Options.Get_Bool("interaction_messages_enabled"))
						{
							Draw_Interact(canvas, controller.Is_Interacting(), controller.Get_Interact_Progress(), controller.Get_Interact_Prompt(), name_pos, health_delta, armor_delta, reload_delta);
							prompt_drawn = true;
						}
					}
					else
					{
						if (human != null && human.Is_Being_Revived())
						{
							Draw_Interact(canvas, false, human.Get_Revive_Delta(), Locale.Get("#hud_being_revived").Format(new object[] { <object>human.Get_Reviver_User().Display_Username }), name_pos, health_delta, armor_delta, reload_delta);
							prompt_drawn = true;							
						}
						else if (map_pickup != null && user.Is_Local)
						{
                            m_map_prompt_time_since_shown += Time.Get_Delta_Seconds();

                            if (m_map_prompt_time_since_shown < MAP_PROMPT_SHOW_INTERVAL)
                            {
							    Draw_Interact(canvas, false, 0.0f, Locale.Get("#interact_map_prompt"), name_pos, health_delta, armor_delta, reload_delta);
							    prompt_drawn = true;
                            }

							if (Input.Is_Down(user, OutputBindings.Interact))
							{
								//bWasMapButtonDown = true;

								//if (!bWasTryingToActivateMapLastFrame)
								//{
									Minimap.Show(user);
									bShowingMap = true;
								//}
							}
						}
                        else
                        {
                            m_map_prompt_time_since_shown = 0.0f;
                        }
					}

					if (!prompt_drawn)
					{
                        string name = user.Username;                        
                        if (Cheat_Manager.Randomise_Names)
                        {
                            name = Random_Names[Random_Name_Index++];
                        }
						Draw_Username(canvas, name, name_pos, health_delta, armor_delta, reload_delta, user.Is_Premium, possessed);
					}
				}
			}
		}

		/*if (bWasMapButtonDown)
		{
			if (!bShowingMap)
			{
				bWasTryingToActivateMapLastFrame = true;
			}
		}
		else
		{
			bWasTryingToActivateMapLastFrame = false;
		}*/
		
		if (!bShowingMap)
		{
			Minimap.Hide();
		}

		// Little bobby effect.
		m_bob_time += Time.Get_Delta_Seconds();
		float bob_delta = Math.Clamp(m_bob_time / BOB_INTERVAL, 0.0f, 1.0f);
				
		// Bob up.
		if (m_bob_direction == 0)
		{
			m_bob_scale = Math.SmoothStep(Vec2(1.0f, 1.0f), Vec2(1.0f - BOB_SCALE_REDUCTION, 1.0f - BOB_SCALE_REDUCTION), bob_delta);
		}
		// Bob down.
		else
		{
			m_bob_scale = Math.SmoothStep(Vec2(1.0f - BOB_SCALE_REDUCTION, 1.0f - BOB_SCALE_REDUCTION), Vec2(1.0f, 1.0f), bob_delta);
		}

		// Reverse direction.
		if (bob_delta == 1.0f)
		{
			m_bob_direction = 1 - m_bob_direction;
			m_bob_time = 0.0f;
		}
	}
	
	private void Draw_Interaction_Pips(Canvas canvas, Camera camera)
	{
		Actor[] actors = Scene.Find_Actors_In_Radius(typeof(Interactable), 80.0f, Vec3(camera.Get_Viewport().Center(), 0.0f));

		float delta = ((1.0f - m_bob_scale.Y) / BOB_SCALE_REDUCTION);
		
		NetUser[] users = Network.Get_Local_Users();			

		foreach (Actor inter in actors)
		{
			bool can_interact = false;

			foreach (NetUser user in users)
			{
				if (user.Controller != null)
				{
					Actor possessed = user.Controller.Possessed;
					if (possessed != null)
					{
						if ((<Interactable>inter).Can_Interact(<Pawn>possessed))
						{
							can_interact = true;
							break;
						}
					}
				}
			}

			if (can_interact)
			{
				Vec4 bbox = canvas.World_BBox_To_Screen(inter.World_Bounding_Box);
				Vec2 info_pos = Vec2(bbox.X + (bbox.Width * 0.5f), (bbox.Y + (bbox.Height * 0.1f)) -  (delta * BOB_HEIGHT));
			
				canvas.Color = Vec4(255.0f, 232.0f, 30.0f, 255.0f);
				canvas.Rotation = 180;
				canvas.Scale = m_bob_scale * canvas.UI_Scale;
				canvas.Draw_Frame("game_hud_player_compass_pip", info_pos);
				canvas.Scale = Vec2(1.0f, 1.0f);
				canvas.Rotation = 0;
			}
		}
	}
	
	private void Draw_Turret_Infos(Canvas canvas)
	{
		float scale = canvas.UI_Scale;

		Actor[] turrets = Scene.Find_Actors(typeof(Turret));

		foreach (Turret turret in turrets)
		{
			Vec4 bbox = canvas.World_BBox_To_Screen(turret.World_Bounding_Box);
			Vec2 info_pos = Vec2(bbox.X + (bbox.Width * 0.5f), bbox.Y + turret.Get_Height_Offset() + (bbox.Height * 0.1f));

            //info_pos.Y -= ((Math.Sin(Time.Get_Ticks() * 0.002f) + 1.0f) * 0.5f) * 6.0f * scale;

			Draw_Turret_Info(canvas, turret, info_pos);
		}
		
		Actor[] pylons = Scene.Find_Actors(typeof(Love_Pylon));

		foreach (Love_Pylon pylon in pylons)
		{
			Vec4 bbox = canvas.World_BBox_To_Screen(pylon.World_Bounding_Box);
			Vec2 info_pos = Vec2(bbox.X + (bbox.Width * 0.5f), bbox.Y + pylon.Get_Height_Offset() + (bbox.Height * 0.1f));
			
            //info_pos.Y -= ((Math.Sin(Time.Get_Ticks() * 0.002f) + 1.0f) * 0.5f) * 6.0f * scale;

			Draw_Turret_Info(canvas, pylon, info_pos);
		}
	}
	
	private void Draw_Turret_Info(Canvas canvas, Movable mov, Vec2 position)
	{
		float scale = canvas.UI_Scale;

		Vec2 frame_size = canvas.Get_Frame_Size("actor_turrets_ammo_bubble_0");
		Vec4 bbox = Vec4(position.X - (frame_size.X * 0.5f * scale), position.Y - (frame_size.Y * scale), frame_size.X * scale, frame_size.Y * scale);

		Vec4 ammo_bar = Vec4(bbox.X, bbox.Y, 0.0f, 0.0f) + (Vec4(12, 3, 15, 2) * scale);
		Vec4 hp_bar   = Vec4(bbox.X, bbox.Y, 0.0f, 0.0f) + (Vec4(12, 8, 15, 2) * scale);

		Turret turret = <Turret>mov;
		Love_Pylon pylon = <Love_Pylon>mov;

        float FADE_SPEED = 10.0f;

		float health = 0.0f;
		float ammo   = 0.0f;    
        float alpha = 1.0f;
    
		float delta_t = Time.Get_Delta_Seconds();

        if (Screen_Rect_Overlaps_Pawns(canvas, bbox))
        {
            alpha = 0.15f;
        }

		if (turret)
		{
			health = turret.Get_Health_Scalar();
			ammo = turret.Get_Ammo_Scalar();

            turret.Hud_Element_Alpha = Math.Lerp(turret.Hud_Element_Alpha, alpha, FADE_SPEED * delta_t);
            alpha = turret.Hud_Element_Alpha;
		}
		else if (pylon)
		{
			health = pylon.Get_Health_Scalar();
			ammo = pylon.Get_Ammo_Scalar();

            pylon.Hud_Element_Alpha = Math.Lerp(pylon.Hud_Element_Alpha, alpha, FADE_SPEED * delta_t);
            alpha = pylon.Hud_Element_Alpha;	
        }
		
		canvas.Color = Vec4(255.0f, 255.0f, 255.0f, 255.0f * alpha);
		canvas.Draw_Frame("actor_turrets_ammo_bubble_0", bbox);
		canvas.Color = Vec4(255.0f, 0.0f, 25.0f, 255.0f * alpha);
		canvas.Draw_Rect(Vec4(hp_bar.X, hp_bar.Y, hp_bar.Width * health, hp_bar.Height));
		canvas.Color = Vec4(66.0f, 107.0f, 255.0f, 255.0f * alpha);
		canvas.Draw_Rect(Vec4(ammo_bar.X, ammo_bar.Y, ammo_bar.Width * ammo, ammo_bar.Height));
	}

	private void Draw_Player_HUD(Canvas canvas, NetUser user, Vec4 bbox, bool left_aligned, bool top_aligned, bool bPVP)
	{
		Vec4 avatar_box;
		Vec4 main_box;
		Vec4 xp_icon_box;
		Vec4 xp_box;
		Vec4 health_icon_box;
		Vec4 health_box;
		Vec4 sprint_icon_box;
		Vec4 sprint_box;
		Vec4 weapon_box;
		Vec4 ammo_icon_box;
		Vec4 ammo_box;
        Vec4 spawn_message_box;
		
		float padding = -1.0f;
		float bar_height = bbox.Height * (1.0f / 4);

		Vec4 old_viewport = canvas.Viewport;

		int near_text_align;
		int far_text_align;

		// Calculate box positions.
		if (left_aligned == true)
		{
			if (top_aligned == true)
			{
				avatar_box = Vec4(bbox.X, bbox.Y - (padding * 4), bbox.Height, bbox.Height);
				main_box = Vec4(bbox.X + bbox.Height, bbox.Y, bbox.Width - bbox.Height, bbox.Height);

                float y_offset = main_box.Y;
                float x_offset = main_box.X + bar_height + 2.0f;
                float icon_x_offset = main_box.X;
            
                xp_icon_box = Vec4(icon_x_offset, y_offset, bar_height, bar_height);
				xp_box = Vec4(x_offset, y_offset, main_box.Width - bar_height - padding, bar_height);
                y_offset += (bar_height + padding);

				sprint_icon_box = Vec4(icon_x_offset, y_offset, bar_height, bar_height);
				sprint_box = Vec4(x_offset, y_offset, main_box.Width - bar_height - padding, bar_height);
                y_offset += (bar_height + padding);
            
				health_icon_box = Vec4(icon_x_offset, y_offset, bar_height, bar_height);
				health_box = Vec4(x_offset, y_offset, main_box.Width - bar_height - padding, bar_height);
                y_offset += (bar_height + padding);

				weapon_box = Vec4(x_offset, y_offset, main_box.Width - bar_height - padding, bar_height);
                y_offset += (bar_height + padding);

				ammo_icon_box = Vec4(icon_x_offset, y_offset, bar_height, bar_height);
				ammo_box = Vec4(x_offset, y_offset, main_box.Width - bar_height - padding, bar_height);
                y_offset += (bar_height + padding);
            
                spawn_message_box = Vec4(icon_x_offset, y_offset, main_box.Width, bar_height);
			}
			else
			{
				avatar_box = Vec4(bbox.X, bbox.Y, bbox.Height, bbox.Height);
				main_box = Vec4(bbox.X + bbox.Height, bbox.Y, bbox.Width - bbox.Height, bbox.Height);
            
                float y_offset = main_box.Y - (bar_height + padding);
                float x_offset = main_box.X + bar_height + 2.0f;
                float icon_x_offset = main_box.X;
            
                ammo_box = Vec4(x_offset, y_offset, main_box.Width - bar_height - padding, bar_height);
				ammo_icon_box = Vec4(icon_x_offset, y_offset, bar_height, bar_height);
                y_offset += (bar_height + padding);

				weapon_box = Vec4(x_offset, y_offset, main_box.Width - bar_height - padding, bar_height);
                y_offset += (bar_height + padding);

                spawn_message_box = Vec4(icon_x_offset, y_offset, main_box.Width, bar_height);

				health_icon_box = Vec4(icon_x_offset, y_offset, bar_height, bar_height);
				health_box = Vec4(x_offset, y_offset, main_box.Width - bar_height - padding, bar_height);
                y_offset += (bar_height + padding);

				sprint_icon_box = Vec4(icon_x_offset, y_offset, bar_height, bar_height);
				sprint_box = Vec4(x_offset, y_offset, main_box.Width - bar_height - padding, bar_height);
                y_offset += (bar_height + padding);
				
				xp_icon_box = Vec4(icon_x_offset, y_offset, bar_height, bar_height);
				xp_box = Vec4(x_offset, y_offset, main_box.Width - bar_height - padding, bar_height);
                y_offset += (bar_height + padding);
            }

			near_text_align = Text_Alignment.Left;
			far_text_align = Text_Alignment.Right;
		}
		else
		{
			if (top_aligned == true)
			{				
				avatar_box = Vec4((bbox.X + bbox.Width) - bbox.Height, bbox.Y - (padding * 4), bbox.Height, bbox.Height);
				main_box = Vec4(bbox.X, bbox.Y, bbox.Width - bbox.Height, bbox.Height);
			
                float y_offset = main_box.Y;
                float x_offset = main_box.X;
                float icon_x_offset = main_box.X + main_box.Width - bar_height;
            
                xp_icon_box = Vec4(icon_x_offset, y_offset, bar_height, bar_height);
				xp_box = Vec4(x_offset, y_offset, main_box.Width - bar_height - 2.0f, bar_height);
                y_offset += (bar_height + padding);
           
				sprint_icon_box = Vec4(icon_x_offset, y_offset, bar_height, bar_height);
				sprint_box = Vec4(x_offset, y_offset, main_box.Width - bar_height - 2.0f, bar_height);
                y_offset += (bar_height + padding);

				health_icon_box = Vec4(icon_x_offset, y_offset, bar_height, bar_height);
				health_box = Vec4(x_offset, y_offset, main_box.Width - bar_height - 2.0f, bar_height);
                y_offset += (bar_height + padding);

				weapon_box = Vec4(x_offset, y_offset, main_box.Width - bar_height - 2.0f, bar_height);
                y_offset += (bar_height + padding);

				ammo_box = Vec4(x_offset, y_offset, main_box.Width - bar_height - 2.0f, bar_height);
				ammo_icon_box = Vec4(icon_x_offset, y_offset, bar_height, bar_height);
                y_offset += (bar_height + padding);
            
                spawn_message_box = Vec4(x_offset, y_offset, main_box.Width, bar_height);
            }
			else
			{
				avatar_box = Vec4((bbox.X + bbox.Width) - bbox.Height, bbox.Y, bbox.Height, bbox.Height);
				main_box = Vec4(bbox.X, bbox.Y, bbox.Width - bbox.Height, bbox.Height);
			
                float y_offset = main_box.Y - (bar_height + padding);
                float x_offset = main_box.X;
                float icon_x_offset = main_box.X + main_box.Width - bar_height;

				ammo_box = Vec4(x_offset, y_offset, main_box.Width - bar_height - 2.0f, bar_height);
				ammo_icon_box = Vec4(icon_x_offset, y_offset, bar_height, bar_height);
                y_offset += (bar_height + padding);
            
				weapon_box = Vec4(x_offset, y_offset, main_box.Width - bar_height - 2.0f, bar_height);
                y_offset += (bar_height + padding);
				
                spawn_message_box = Vec4(x_offset, y_offset, main_box.Width, bar_height);

				health_icon_box = Vec4(icon_x_offset, y_offset, bar_height, bar_height);
				health_box = Vec4(x_offset, y_offset, main_box.Width - bar_height - 2.0f, bar_height);
                y_offset += (bar_height + padding);

				sprint_icon_box = Vec4(icon_x_offset, y_offset, bar_height, bar_height);
				sprint_box = Vec4(x_offset, y_offset, main_box.Width - bar_height - 2.0f, bar_height);
                y_offset += (bar_height + padding);

				xp_icon_box = Vec4(icon_x_offset, y_offset, bar_height, bar_height);
				xp_box = Vec4(x_offset, y_offset, main_box.Width - bar_height - 2.0f, bar_height);
                y_offset += (bar_height + padding);
			}

			near_text_align = Text_Alignment.Right;
			far_text_align = Text_Alignment.Left;
		}
		
		Vec2 frame_size = canvas.Get_Frame_Size("game_hud_health_bar_border_0");
		float scale = canvas.UI_Scale;
		Vec4 health_bar_inside_box = Vec4(health_box.X + (4.0f * scale), health_box.Y + (3.0f * scale), health_box.Width - (8.0f * scale), health_box.Height - (6.0f * scale));
		Vec4 sprint_bar_inside_box = Vec4(sprint_box.X + (4.0f * scale), sprint_box.Y + (3.0f * scale), sprint_box.Width - (8.0f * scale), sprint_box.Height - (6.0f * scale));
		Vec4 xp_bar_inside_box = Vec4(xp_box.X + (4.0f * scale), xp_box.Y + (3.0f * scale), xp_box.Width - (8.0f * scale), xp_box.Height - (6.0f * scale));

		// Get avatar information.
		bool is_dead		    = true;
        bool is_incapacitated   = false;
		Vec4 color_tint		= Vec4(0.1f, 0.1f, 0.1f, 1.0f);
		int  direction		= Direction.S;
		Item head_item 		= user.Profile.Get_Item_Slot(Item_Slot.Head);
		Item accessory_item = user.Profile.Get_Item_Slot(Item_Slot.Accessory);

		float health_scalar	= 0.0f;
		float armor_scalar	= 0.0f;
		float sprint_scalar	= 0.0f;
		float xp_scalar	    = 0.0f;
		int	  coins			= 0;

        int xp_current = 50;
        int xp_max     = 100;
    
        int stamina_current = 0;
        int stamina_max     = 100;

        int health_current = 0;
        int health_max     = 100;

		Controller controller = user.Controller;
		Pawn pawn = null;

        Human_Player human = null;
        bool bIsHuman = (controller == null);

		if (controller != null)
		{
			if (controller.Owner != null)
			{
				coins = <int>Math.Ceiling(controller.Owner.Display_Coins);
			}

			pawn = controller.Possessed;

		    human = <Human_Player>pawn;
            bIsHuman = (pawn == null || human != null);

			if (pawn != null)
			{
				// Lerp the armour/health so it changes a bit smoother.
				pawn.Display_Health = Math.Lerp(pawn.Display_Health, pawn.Health, 0.05f);
				pawn.Display_Armor = Math.Lerp(pawn.Display_Armor, pawn.Armor, 0.05f);
				pawn.Display_Stamina = Math.Lerp(pawn.Display_Stamina, pawn.Stamina, 0.3f);
				user.Display_XP = Math.Lerp(user.Display_XP, user.Local_Profile.Level_XP, 0.3f);

				direction = pawn.Current_Direction;
				health_scalar = pawn.Display_Health / pawn.Max_Health;
				armor_scalar = pawn.Display_Armor / pawn.Max_Armor;
				sprint_scalar = pawn.Display_Stamina / pawn.Max_Stamina;
            
                health_current = pawn.Health + pawn.Armor;
                health_max     = pawn.Max_Health;
            
                stamina_current  = pawn.Stamina;
                stamina_max      = pawn.Max_Stamina;
            
                xp_current = user.Display_XP;
                xp_max     = user.Local_Profile.Next_Level_XP;

                if (xp_max == 0)
                {
                    xp_scalar = 1.0f;
                }
                else
                {
                    xp_scalar = <float>xp_current / <float>xp_max;
                }
    
				is_dead = (pawn.Health <= 0.0f);

                if (human != null)
                {
                    is_incapacitated = human.Is_Incapacitated();
                }

                if (is_dead == false)
				{
					color_tint = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
				}
				else
				{					
					direction = Direction.S;
				}
			}
		}
        
        // If user is dead and we are rendering in split-screen, don't shown any of this.
        if (is_dead && Scene.Get_Camera_Count() > 1)
        {
            return;
        }

		// Avatar
		// TODO: this needs to go into the controller class, as controlling zombies etc will need to 
		//		 render different things.
		Vec4 draw_avatar_box = avatar_box.Inflate(2.0f, 2.0f);
		canvas.Color = Vec4(255.0f, 255.0f, 255.0f, 255.0f) * color_tint;
        if (bIsHuman)
        {
		    if (head_item != null)
		    {
			    canvas.Draw_Animation(head_item.Archetype.Icon_Animation + "_" + Direction_Helper.Mnemonics[direction], draw_avatar_box);
			    if (head_item.Archetype.Is_Tintable)
			    {
				    canvas.Color = head_item.Primary_Color * color_tint;
				    canvas.Draw_Animation(head_item.Archetype.Icon_Animation + "_tint_" + Direction_Helper.Mnemonics[direction], draw_avatar_box);			
				    canvas.Color = Vec4(255.0f, 255.0f, 255.0f, 255.0f) * color_tint;
			    }
		    }
		    if (accessory_item != null && accessory_item.Archetype.Is_Icon_Overlay)
		    {
			    canvas.Draw_Animation(accessory_item.Archetype.Icon_Animation + "_" + Direction_Helper.Mnemonics[direction], draw_avatar_box);
			    if (accessory_item.Archetype.Is_Tintable)
			    {
				    canvas.Color = head_item.Primary_Color * color_tint;
				    canvas.Draw_Animation(accessory_item.Archetype.Icon_Animation + "_tint_" + Direction_Helper.Mnemonics[direction], draw_avatar_box);		
				    canvas.Color = Vec4(255.0f, 255.0f, 255.0f, 255.0f) * color_tint;	
			    }
		    }
        }   
        else if (pawn != null)
        {
		    canvas.Draw_Animation(pawn.Get_Hud_Head_Animation() + "_" + Direction_Helper.Mnemonics[direction], draw_avatar_box);
        }

		// Draw health.
		canvas.Color = Vec4(255.0f, 255.0f, 255.0f, 255.0f) * color_tint;
		canvas.Draw_Frame_Box("game_hud_health_bar_border_#", health_box, canvas.UI_Scale);

		canvas.Color = Vec4(255.0f, 0.0f, 0.0f, 255.0f) * color_tint;
		canvas.Draw_Rect(health_bar_inside_box * Vec4(1.0f, 1.0f, health_scalar, 1.0f));

		canvas.Color = Vec4(128.0f, 128.0f, 128.0f, 255.0f) * color_tint;
		canvas.Draw_Rect(health_bar_inside_box * Vec4(1.0f, 1.0f, armor_scalar, 1.0f));

		canvas.Color = Vec4(255.0f, 255.0f, 255.0f, 255.0f) * color_tint;
		if (armor_scalar > 0.001f)
		{
			canvas.Draw_Frame("game_hud_hud_icons_1", health_icon_box);
		}
		else
		{
			canvas.Draw_Frame("game_hud_hud_icons_0", health_icon_box);
		}

        if (Options.Get_Bool("show_numeric_stats"))
        {
            canvas.Color = Vec4(255.0f, 255.0f, 255.0f, 255.0f) * color_tint;
            canvas.Font_Shadowed = true;
            canvas.Draw_Text(health_current + " / " + health_max, health_box.Inflate(0.8f, 0.8f) + Vec4(0.0f, -(0.33f * scale), 0.0f, 0.0f), 16.0f, Text_Alignment.Center, Text_Alignment.ScaleToFit);
        }

		// Draw stamina.
		canvas.Color = Vec4(255.0f, 255.0f, 255.0f, 255.0f) * color_tint;
		canvas.Draw_Frame_Box("game_hud_health_bar_border_#", sprint_box, canvas.UI_Scale);

		canvas.Color = Vec4(34.0f, 181.0f, 78.0f, 255.0f) * color_tint;
		canvas.Draw_Rect(sprint_bar_inside_box * Vec4(1.0f, 1.0f, sprint_scalar, 1.0f));

		canvas.Color = Vec4(255.0f, 255.0f, 255.0f, 255.0f) * color_tint;
		canvas.Draw_Frame("game_hud_hud_icons_2", sprint_icon_box);

        if (Options.Get_Bool("show_numeric_stats"))
        {
            canvas.Color = Vec4(255.0f, 255.0f, 255.0f, 255.0f) * color_tint;
            canvas.Font_Shadowed = true;
            canvas.Draw_Text(stamina_current + " / " + stamina_max, sprint_box.Inflate(0.8f, 0.8f) + Vec4(0.0f, -(0.33f * scale), 0.0f, 0.0f), 16.0f, Text_Alignment.Center, Text_Alignment.ScaleToFit);
        }

        // Draw XP
		canvas.Color = Vec4(255.0f, 255.0f, 255.0f, 255.0f) * color_tint;
		canvas.Draw_Frame_Box("game_hud_health_bar_border_#", xp_box, canvas.UI_Scale);

		canvas.Color = Vec4(255.0f, 194.0f, 14.0f, 255.0f) * color_tint;
		canvas.Draw_Rect(xp_bar_inside_box * Vec4(1.0f, 1.0f, xp_scalar, 1.0f));

		canvas.Color = Vec4(255.0f, 255.0f, 255.0f, 255.0f) * color_tint;
		canvas.Draw_Frame("game_hud_hud_icons_3", xp_icon_box);

        if (Options.Get_Bool("show_numeric_stats"))
        {
            canvas.Color = Vec4(255.0f, 255.0f, 255.0f, 255.0f) * color_tint;
            canvas.Font_Shadowed = true;
            if (xp_max == 0.0f)
            {
                canvas.Draw_Text(Locale.Get("#menu_inventory_max_level"), xp_box.Inflate(0.8f, 0.8f) + Vec4(0.0f, -(0.33f * scale), 0.0f, 0.0f), 16.0f, Text_Alignment.Center, Text_Alignment.ScaleToFit);
            }
            else
            {
                string val =
                    Locale.Get("#menu_inventory_xp_no_postfix").Format(new object[] {
                        string.FormatNumber(<float>xp_current),
                        string.FormatNumber(<float>xp_max)
                    });

                canvas.Draw_Text(val, xp_box.Inflate(0.8f, 0.8f) + Vec4(0.0f, -(0.33f * scale), 0.0f, 0.0f), 16.0f, Text_Alignment.Center, Text_Alignment.ScaleToFit);
            }
        }

        // Draw ammo.	
        Weapon active_weapon = pawn == null ? null : pawn.Get_Active_Weapon();
		Weapon_Ammo active_ammo = active_weapon == null ? null : active_weapon.Ammo_Modifier;
		if (active_weapon != null && active_weapon.Stamina_Based == false)
		{
			if (active_weapon.Is_Reloading)
			{
				canvas.Draw_Text(". . .", ammo_box.Shift(0.0f, 0.1f), 16.0f, near_text_align, Text_Alignment.Center);
			}
			else
			{
				if (active_weapon.Is_Rechargable)
				{
					int percent = <int>((<float>active_weapon.Clip_Ammo / <float>active_weapon.Clip_Size) * 100.0f);
					canvas.Draw_Text(percent + " %", ammo_box.Shift(0.0f, 0.1f), 16.0f, near_text_align, Text_Alignment.Center);
				}
				else
				{
					string ammo_text = active_weapon.Clip_Ammo + " / " + active_weapon.Reserve_Ammo;
					
					bool is_unlimited = (active_weapon.Has_Unlimited_Ammo == true || user.Stat_Multipliers[Profile_Stat.Ammo_Usage] == 0.0f) && (active_weapon.Ammo_Modifier == null);
					if (is_unlimited)
					{
						ammo_text = active_weapon.Clip_Ammo + " / [img=game_hud_infinite_symbol,0.75,0.75,0,2]";
					}

					Vec2 text_size = canvas.Calculate_Markup_Text_Size(ammo_text, 16.0f);
					canvas.Draw_Markup_Text(ammo_text, ammo_box.Shift(0.0f, 0.1f), 16.0f, near_text_align, Text_Alignment.Center);
				}
			}
		}

		// Draw coins.
        if (bIsHuman)
        {
		    canvas.Draw_Markup_Text("[img=screen_main_gold_icon,1.0,1.0] " + string.FormatNumber(<float>coins), ammo_box, 16.0f, far_text_align, Text_Alignment.Center);
        }       
        		 
		// Draw ammo.
		if (active_weapon == null || active_weapon.Stamina_Based == false)
		{
			if (active_weapon != null && active_weapon.HUD_Ammo_Icon != "")
			{
				if (active_ammo != null)
				{
					canvas.Draw_Frame(active_ammo.HUD_Ammo_Icon + "_0", ammo_icon_box);	
				}
				else
				{
					canvas.Draw_Frame(active_weapon.HUD_Ammo_Icon + "_0", ammo_icon_box);
				}
			}
			else
			{
				if (active_weapon != null)
				{
					canvas.Draw_Frame("game_hud_weaponicon_unknown_0", ammo_icon_box);
				}
				else
				{
					canvas.Draw_Frame("game_hud_weaponicon_none_0", ammo_icon_box);
				}
			}
		}

		// Draw weapon icons.
		float fragment_size = weapon_box.Width / Weapon_Slot.COUNT;
		float offset = 0.0f;
		float extra_offset = (fragment_size - weapon_box.Height) * 0.5f;
		for (int i = 0; i < Weapon_Slot.COUNT; i++)
		{			
			Vec4 area;
			if (top_aligned)
			{
				area = Vec4(weapon_box.X + extra_offset + offset, weapon_box.Y, weapon_box.Height, weapon_box.Height);
			}
			else
			{
				area = Vec4(weapon_box.X + extra_offset + offset, weapon_box.Y - (weapon_box.Height * 0.2f), weapon_box.Height, weapon_box.Height);
			}

			Weapon weapon = pawn == null ? null : pawn.Get_Weapon_Slot(i);
			string frame_name = "";
			if (weapon != null && weapon.HUD_Icon != "")
			{
				frame_name = weapon.HUD_Icon;
			}
			else
			{
				if (weapon != null)
				{
					frame_name = "game_hud_weaponicon_unknown";
				}
				else
				{
					frame_name = "game_hud_weaponicon_none";
				}
			}
			
			// Draw selected shadow.
			if (pawn != null && pawn.Get_Active_Weapon_Slot() == i)
			{
				canvas.Color = Vec4(255.0f, 255.0f, 53.0f, 255.0f) * color_tint;
				canvas.Draw_Frame(frame_name + "_1", area);
				canvas.Color = Vec4(255.0f, 255.0f, 255.0f, 255.0f) * color_tint;
			}

			// Draw icon.
			canvas.Draw_Frame(frame_name + "_0", area);

			offset += fragment_size;
		}

		// Draw skill icons.
		if (bIsHuman && !bPVP && (human == null || !human.Skills_Disabled))
		{
			float skill_padding = 1.0f * canvas.UI_Scale;
			float skill_box_size = top_aligned ?  Math.Abs((sprint_box.Y + sprint_box.Height) - xp_box.Y) : Math.Abs((xp_box.Y + xp_box.Height) - sprint_box.Y);//Math.Clamp(20.0f * scale, 0.0f, 40.0f);
			Vec4 skill_box = Vec4(0.0f, 0.0f, skill_box_size, skill_box_size);
			if (left_aligned)
			{
				if (top_aligned)
				{
					skill_box = Vec4(health_box.X + health_box.Width + skill_padding, xp_box.Y, skill_box.Width, skill_box.Height);
				}
				else
				{
					skill_box = Vec4(health_box.X + health_box.Width + skill_padding, (xp_box.Y + xp_box.Height) - skill_box.Height, skill_box.Width, skill_box.Height);
				}
			}
			else
			{
				if (top_aligned)
				{
					skill_box = Vec4(health_box.X - skill_box.Width - skill_padding, xp_box.Y, skill_box.Width, skill_box.Height);			
				}
				else
				{
					skill_box = Vec4(health_box.X - skill_box.Width - skill_padding, (xp_box.Y + xp_box.Height) - skill_box.Height, skill_box.Width, skill_box.Height);
			
				}
			}
			for (int i = 0; i < 3; i++)
			{
				Skill skill = user.Profile.Get_Skill_Slot(i);
				if (skill != null)
				{
					Skill_Archetype archetype = Skill_Archetype.Find_By_ID(skill.Archetype_ID);
                    
					bool has_energy = Cheat_Manager.Infinite_Energy;			
                    float recharge_delta = 0.0f;

                    if (!has_energy)                
                    {
                        float energy_avail = user.Local_Profile.Get_Skill_Energy(archetype);

                        if (energy_avail >= archetype.Energy_Required)
                        {
                            has_energy = true;   
                        }
   
                        recharge_delta = 1.0f - Math.Clamp(energy_avail / archetype.Energy_Required, 0.0f, 1.0f);
                    }
    
					bool is_active = Skill_Effect_Manager.Is_Active(archetype, user);
                    
					Vec4 viewport_box = skill_box + Vec4(1.5f * canvas.UI_Scale, 1.5f * canvas.UI_Scale, -(3.0f * canvas.UI_Scale), -(3.0f * canvas.UI_Scale));
				
					if (!has_energy)
					{
						canvas.Color = Vec4(255.0f, 255.0f, 255.0f, 255.0f) * color_tint;
					}
					else
					{	
						canvas.Color = Vec4(255.0f, 255.0f, 255.0f, 255.0f) * color_tint;
					}

					//canvas.Viewport = viewport_box;
					canvas.Draw_Frame(archetype.Icon_Frame, viewport_box);
					//canvas.Viewport = old_viewport;

					if (is_active)
					{
						//canvas.Color = Vec4(255.0f, 255.0f, 255.0f, 255.0f) * color_tint;
						//canvas.Draw_Frame("game_hud_slot_fore", skill_box);	
                        
						float time_left = Skill_Effect_Manager.Get_Time_Left(archetype, user);
						float active_delta = 1.0f - (time_left / archetype.Duration);
                        
						canvas.Color = Vec4(255.0f, 255.0f, 255.0f, 255.0f) * color_tint;
						canvas.Draw_Frame("game_hud_slot_fore", skill_box);	
					
						float height = (viewport_box.Height * active_delta);
						canvas.Color = Vec4(0.0f, 0.0f, 0.0f, 172.0f) * color_tint;
						//canvas.Viewport = viewport_box;
						canvas.Draw_Rect(Vec4(viewport_box.X, viewport_box.Y + viewport_box.Height - height, viewport_box.Width, height));
						//canvas.Viewport = old_viewport;

						canvas.Color = Vec4(255.0f, 255.0f, 255.0f, 255.0f) * color_tint;
						canvas.Draw_Frame("game_hud_slot_fore_active", skill_box);	

	/*
						float time_left = Skill_Effect_Manager.Get_Time_Left(archetype, user);
						float active_delta = 1.0f - (time_left / archetype.Duration);

						float height = (skill_box.Height * active_delta);			
						canvas.Color = Vec4(0.0f, 0.0f, 0.0f, 172.0f);
						canvas.Draw_Rect(Vec4(skill_box.X, skill_box.Y + skill_box.Height - height, skill_box.Width, height));

						float alpha = 1.0f - ((1.0f - m_bob_scale.X) * 2);
						canvas.Color = Vec4(255.0f, 255.0f, 255.0f, 255.0f * alpha);	
						canvas.Draw_Frame("game_hud_slot_fore_active", skill_box);	
	*/
					}
					/*else if (!has_energy)
					{
						canvas.Color = Vec4(255.0f, 255.0f, 255.0f, 255.0f) * color_tint;
						canvas.Draw_Frame("game_hud_slot_fore_no_energy", skill_box);	

		//				canvas.Color = Vec4(255.0f, 255.0f, 255.0f, 255.0f);
		//				canvas.Draw_Frame("game_hud_slot_fore", skill_box);	
		//				canvas.Draw_Rect(skill_box);
					}*/
					else
					{
						if (has_energy || archetype.Energy_Required == 0)
						{
	//						float alpha = 1.0f - ((1.0f - m_bob_scale.X) * 2);
	//						canvas.Color = Vec4(255.0f, 255.0f, 255.0f, 255.0f * alpha);						
	//						canvas.Draw_Frame("skill_ready_overlay", skill_box);	
					
	//						canvas.Draw_Frame("game_hud_slot_fore", skill_box);		
	//						canvas.Draw_Frame("game_hud_slot_fore_active", skill_box);		
							
							canvas.Color = Vec4(255.0f, 255.0f, 255.0f, 255.0f) * color_tint;
							canvas.Draw_Frame("game_hud_slot_fore", skill_box);	
					
							float alpha = 1.0f - ((1.0f - m_bob_scale.X) * 2);
							canvas.Color = Vec4(255.0f, 255.0f, 255.0f, 255.0f * alpha) * color_tint;	
							canvas.Draw_Frame("game_hud_slot_fore_ready", skill_box);	
						}
						else
						{						
							float height = (viewport_box.Height * recharge_delta);
							canvas.Color = Vec4(0.0f, 0.0f, 0.0f, 172.0f) * color_tint;
							//canvas.Viewport = viewport_box;
							canvas.Draw_Rect(Vec4(viewport_box.X, viewport_box.Y + viewport_box.Height - height, viewport_box.Width, height));
							//canvas.Viewport = old_viewport;

							canvas.Color = Vec4(255.0f, 255.0f, 255.0f, 255.0f) * color_tint;
							canvas.Draw_Frame("game_hud_slot_fore_no_energy", skill_box);	
						}
					} 
				}
				else
				{				
					canvas.Color = Vec4(255.0f, 255.0f, 255.0f, 255.0f) * color_tint;	
					canvas.Draw_Frame("game_hud_slot_empty", skill_box);	
				}

				if (left_aligned)
				{
					skill_box.X += skill_box.Width + skill_padding;
				}
				else
				{
					skill_box.X -= skill_box.Width + skill_padding;
				}
			}
		}

		// Draw dead icon.
		if (is_dead == true)
		{
			canvas.Color = Vec4(255.0f, 255.0f, 255.0f, 255.0f);
			canvas.Draw_Frame("game_hud_dead_text", Vec4(main_box.X, main_box.Y + (main_box.Height * 0.25f), main_box.Width, main_box.Height - (main_box.Height * 0.25f)));
		}

        // If user is dead we can write the "Press X To Control Zombie" message.
        if (is_dead && !is_incapacitated)
        {
            if (user != null && Network.Get_Local_Users().Length() <= 1)// && user.Is_Premium)
            {
		        Base_Game_Mode mode = <Base_Game_Mode>Scene.Active_Game_Mode;
                if (mode.Can_Spectators_Control_Zombies)
                {
                    if (user.Waiting_For_Zombie_Control)
                    {
			            canvas.Draw_Markup_Text(Locale.Get("#hud_press_to_control_zombie_waiting"), spawn_message_box, 16.0f, near_text_align, Text_Alignment.Center);
                    }
                    else
                    {
			            canvas.Draw_Markup_Text(Locale.Get("#hud_press_to_control_zombie"), spawn_message_box, 16.0f, near_text_align, Text_Alignment.Center);
                    }

                    Check_Control_Zombie(user);
                }
            }
        }
	}
	
	private void Draw_Message(Canvas canvas, Vec4 bounds)
	{		
		if (Is_Message_Visible())
		{
			float padding = bounds.Height * 0.15f;

			Vec4 inside_bounds	= Vec4(bounds.X + padding, bounds.Y + padding, bounds.Width - (padding * 2), bounds.Height - (padding * 2));
			Vec4 icon_bounds	= Vec4(inside_bounds.X, inside_bounds.Y, inside_bounds.Height, inside_bounds.Height);
			Vec4 title_bounds	= Vec4(inside_bounds.X + icon_bounds.Width, inside_bounds.Y, inside_bounds.Width - icon_bounds.Width, inside_bounds.Height * 0.5f);
			Vec4 message_bounds	= Vec4(inside_bounds.X + icon_bounds.Width, inside_bounds.Y + (inside_bounds.Height * 0.5f), inside_bounds.Width - icon_bounds.Width, inside_bounds.Height * 0.5f);

			canvas.Color = Vec4(0.0f, 0.0f, 0.0f, 255.0f);
			canvas.Draw_Rect(bounds);		
			canvas.Color = Vec4(127.0f, 127.0f, 127.0f, 255.0f);
			canvas.Draw_Rect(Vec4(bounds.X, bounds.Y, bounds.Width, bounds.Height * 0.05f));		
			canvas.Color = Vec4(255.0f, 255.0f, 255.0f, 255.0f);
			canvas.Draw_Frame(m_message_icon, icon_bounds);		
			canvas.Draw_Markup_Text(m_message_title, title_bounds, 16.0f, Text_Alignment.Left, Text_Alignment.ScaleToFit);
			canvas.Draw_Markup_Text(m_message, message_bounds, 16.0f, Text_Alignment.Left, Text_Alignment.ScaleToFit);
		}
	}

	public bool Is_Message_Visible()
	{
		return m_message_active || m_message_height > 0.001f;
	}

	public void Show_Message(string message, string title, string icon, float time_multiplier = 1.0)
	{
		//if (Is_Message_Visible())
		//{
		//	return;
		//}

		m_message_active = true;
		m_message_timer = Message_Bar_Time * time_multiplier;
		m_message = message;
		m_message_title = title;
		m_message_icon = icon;
		
		Audio.Play2D("sfx_hud_tutorial");

		Log.Write("Showing hud message: '"+message+"' with icon '"+icon+"'.");
	}

	private void Update_Message()
	{
		float delta_t = Time.Get_Delta_Seconds();

		if (m_message_active)
		{
			m_message_timer -= delta_t;
			if (m_message_timer <= 0.0f)
			{
				m_message_active = false;
			}
			else
			{				
				m_message_height = Math.Lerp(m_message_height, Message_Bar_Height, Math.Clamp(Message_Bar_Lerp * delta_t, 0.0, 1.0f));
			}
		}
		else
		{
			m_message_height = Math.Lerp(m_message_height, 0.0f, Math.Clamp(Message_Bar_Lerp * delta_t, 0.0, 1.0f));
		}
	}

	private void Draw_Message(Canvas canvas)
	{
		float msg_height = canvas.Bounds.Height * m_message_height;
		Vec4 msg_bounds = Vec4(canvas.Bounds.X, canvas.Bounds.Y + (canvas.Bounds.Height - msg_height), canvas.Bounds.Width, msg_height);
		
		// Draw message bar.
		Draw_Message(canvas, msg_bounds);

		Update_Message();
	}

	private void Draw_Player_HUDS(Canvas canvas, bool bPVP)
	{
		NetUser[] users = Network.Get_Local_Users();			
		int user_count = users.Length();

		float msg_height = canvas.Bounds.Height * m_message_height;
		Vec4 msg_bounds = Vec4(canvas.Bounds.X, canvas.Bounds.Y + (canvas.Bounds.Height - msg_height), canvas.Bounds.Width, msg_height);
		
		Vec4 bounds = canvas.Bounds - Vec4(0.0f, 0.0f, 0.0f, msg_height);
			
		// If more than one camera we center vertically in the middle of the screen.
		float scale = 1.0f;
		if (Scene.Get_Camera_Count() > 1)
		{
			bounds = canvas.Bounds * Vec4(1.0f, 1.0f, 1.0f, 0.5f);
			scale = 2.0f;
		}

		float width = bounds.Width * (Player_HUD_Width);
		float height = bounds.Height * (Player_HUD_Height * scale);
		float padding = bounds.Width * (Player_HUD_Padding);
		
		float offset_height = height;
		int camera_count = Scene.Get_Camera_Count();
		if (camera_count > 1)
		{
			offset_height *= 0.5f;
		}
		
		int user_index = 0;

		foreach (NetUser user in users)
		{
			Vec4 bbox;
			bool left_aligned = false;
			bool top_aligned = false;

			// Users without a profile set we cannot render a hud for :(. Got to wait
			// till they get off their asses and send us one.
			if (user.Profile == null)
			{
				continue;
			}

			// Find split-screen viewport.
			Vec4 viewport = bounds;
			if (camera_count > 1)
			{
				viewport = Scene.Get_Camera_Screen_Viewport_Index(user_index);

				if (camera_count > 2 && (user_index == 1 || user_index == 3))
				{
					bbox = Vec4(viewport.X + viewport.Width - width - padding, viewport.Y + viewport.Height - height - padding, width, height);
					left_aligned = false;
				}
				else
				{
					bbox = Vec4(viewport.X + padding, viewport.Y + viewport.Height - height - padding, width, height);
					left_aligned = true;
				}

				// Top 2 viewports are top-aligned.
				if (camera_count > 2 && (user_index == 0 || user_index == 1))
				{
					bbox.Y = viewport.Y + padding;
					top_aligned = true;
				}
			}
			else
			{
				if (user_index == 0)
				{					
					bbox = Vec4(padding, bounds.Height - offset_height - padding, width, height);
					left_aligned = true;
				}
				else if (user_index == 1)
				{
					bbox = Vec4(bounds.Width - width - padding, bounds.Height - offset_height - padding, width, height);
					left_aligned = false;
				}
				else if (user_index == 2)
				{
					bbox = Vec4(padding, padding, width, height);
					left_aligned = true;
					top_aligned = true;
				}
				else if (user_index == 3)
				{
					bbox = Vec4(bounds.Width - width - padding, padding, width, height);
					left_aligned = false;
					top_aligned = true;
				}
			}

			Draw_Player_HUD(canvas, user, bbox, left_aligned, top_aligned, bPVP);
			user_index++;
		}
	}
}
