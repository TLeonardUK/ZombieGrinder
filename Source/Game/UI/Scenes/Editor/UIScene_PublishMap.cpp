// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/Editor/UIScene_Editor.h"
#include "Game/UI/Scenes/Editor/UIScene_PublishMap.h"
#include "Game/UI/Scenes/UIScene_ConfirmDialog.h"
#include "Game/UI/Scenes/UIScene_Dialog.h"
#include "Engine/UI/Transitions/UISlideInTransition.h"
#include "Engine/UI/Transitions/UIFadeOutTransition.h"
#include "Engine/UI/Transitions/UIFadeInTransition.h"
#include "Engine/UI/Layouts/UILayoutFactory.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Display/GfxDisplay.h"
#include "Engine/Input/Input.h"
#include "Engine/UI/UIManager.h"
#include "Engine/UI/Elements/UIPanel.h"
#include "Engine/UI/Elements/UISlider.h"
#include "Engine/UI/Elements/UIListView.h"
#include "Engine/UI/Elements/UIProgressBar.h"
#include "Engine/UI/Elements/UILabel.h"
#include "Engine/UI/Elements/UIButton.h"
#include "Engine/UI/Elements/UIResourceBrowser.h"
#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Renderer/Textures/PixelmapFactory.h"
#include "Engine/Resources/ResourceFactory.h"

#include "XScript/VirtualMachine/CVMBinary.h"

#include "Engine/Audio/Sounds/Sound.h"
#include "Engine/Audio/Sounds/SoundHandle.h"

#include "Engine/Localise/Locale.h"

#include "Game/UI/Scenes/UIScene_Game.h"

#include "Engine/Engine/GameEngine.h"
#include "Engine/Engine/EngineOptions.h"
#include "Engine/Scripts/EngineVirtualMachine.h"

#include "Engine/Online/OnlinePlatform.h"
#include "Engine/Online/OnlineMods.h"

#include "XScript/VirtualMachine/CVMBinary.h"

#include "Engine/Scene/Scene.h"

#include "Game/Runner/Game.h"
#include "Game/Runner/GameOptions.h"

#include "Game/Scene/GameScene.h"
#include "Game/Scene/Actors/ScriptedActor.h"

UIScene_PublishMap::UIScene_PublishMap()
	: m_uploading(false)
	, m_requesting_mod_info(false)
	, m_creating_mod(false)
	, m_creation_status(false)
{
	Set_Layout("publish_map");
}

std::string UIScene_PublishMap::Validate_Map()
{
	Map* map = Game::Get()->Get_Map();
	MapFileHeaderBlock& block = map->Get_Map_Header();

	GameScene* scene = Game::Get()->Get_Game_Scene();
	EngineVirtualMachine* vm = GameEngine::Get()->Get_VM();

	CVMLinkedSymbol* game_mode_symbol = vm->Find_Class("Base_Game_Mode");
	CVMLinkedSymbol* preview_center_marker_symbol = vm->Find_Class("Preview_Image_Center");
	CVMLinkedSymbol* path_pylon_symbol = vm->Find_Class("Path_Pylon");
	CVMLinkedSymbol* player_start_symbol = vm->Find_Class("Player_Spawn");
	CVMLinkedSymbol* is_main_symbol = vm->Find_Variable(game_mode_symbol, "Is_Main");

	// Check for game mode.
	std::string game_mode_name = "";
	std::vector<ScriptedActor*> actors = scene->Find_Derived_Actors(game_mode_symbol);

	bool enabled_exists = false;

	for (std::vector<ScriptedActor*>::iterator iter = actors.begin(); iter != actors.end(); iter++)
	{
		ScriptedActor* actor = *iter;
		if (actor->Get_Enabled())
		{
			enabled_exists = true;
		}

		CVMContextLock lock(vm->Set_Context(actor->Get_Script_Context()));

		CVMValue is_main;
		vm->Get_Field(is_main_symbol, actor->Get_Script_Object(), is_main);
		if (is_main.int_value == 1)
		{
			game_mode_name = actor->Get_Script_Symbol()->symbol->Get_Meta_Data<std::string>("Name", "< unknown >");
		}

	}

	if (enabled_exists == false)
	{
		return S("#editor_error_game_mode_enabled");
	}
	if (game_mode_name == "")
	{
		return S("#editor_error_game_mode_main");
	}

	// Check for preview marker.
	actors = scene->Find_Derived_Actors(preview_center_marker_symbol);
	if (actors.size() == 0)
	{
		return S("#editor_error_preview_center"); 
	}

	// Check for player starts.
	actors = scene->Find_Derived_Actors(player_start_symbol);
	if (actors.size() == 0)
	{
		return S("#editor_error_player_start"); 
	}

	// Check for path markers.
	actors = scene->Find_Derived_Actors(path_pylon_symbol);
	if (actors.size() == 0)
	{
		return S("#editor_error_path_marker"); 
	}

	// Check preview has been generated.
	if (map->Get_Preview_Pixmap() == NULL)
	{
		return S("#editor_error_preview_save"); 
	}

	// Check we have a save path.
	UIScene_Editor* editor = GameEngine::Get()->Get_UIManager()->Get_Scene_By_Type<UIScene_Editor*>();
	std::string save_path = editor->Get_Save_Path();
	if (save_path == "")
	{
		return  S("#editor_error_preview_save"); 
	}

	Find_Element<UILabel*>("name_label")->Set_Value(SF("#editor_publish_name_text", block.Long_Name.c_str()));
	Find_Element<UILabel*>("description_label")->Set_Value(SF("#editor_publish_description_text", block.Description.c_str(), game_mode_name.c_str(), block.Max_Players));
	Find_Element<UIPanel*>("preview_panel")->Set_Background_Image(map->Get_Preview_Image());
	Find_Element<UILabel*>("upload_label")->Set_Visible(false);

	return "";
}

bool UIScene_PublishMap::Can_Accept_Invite()
{
	return false;
}

const char* UIScene_PublishMap::Get_Name()
{
	return "UIScene_PublishMap";
}

bool UIScene_PublishMap::Should_Render_Lower_Scenes()
{
	return true;
}

bool UIScene_PublishMap::Should_Display_Cursor()
{
	return true;
}

bool UIScene_PublishMap::Should_Display_Focus_Cursor()
{
	return false;
}

bool UIScene_PublishMap::Is_Focusable()
{
	return true;
}

bool UIScene_PublishMap::Should_Fade_Cursor()
{
	return false;
}

void UIScene_PublishMap::Enter(UIManager* manager)
{
	std::string error = Validate_Map();
	if (error != "")
	{	
		manager->Go(UIAction::Replace(new UIScene_Dialog(error, "", false), NULL));
	}
}	

void UIScene_PublishMap::Exit(UIManager* manager)
{
}	

void UIScene_PublishMap::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	Map* map = Game::Get()->Get_Map();
	MapFileHeaderBlock& header = map->Get_Map_Header();

	if (m_requesting_mod_info)
	{
		OnlineSubscribedMod* mod = NULL;
		if (OnlineMods::Get()->Request_Mod_Details(header.Workshop_ID, mod))
		{
			// If not the owner, clear out the workshop id.
			if (!mod->IsAuthor)
			{
				header.Workshop_ID = 0;
			}

			m_requesting_mod_info = false;
			Upload_Mod();
		}
	}
	else if (m_creating_mod)
	{
		// Created yet?
		if (m_creation_status == true)
		{
			if (header.Workshop_ID <= 0)
			{
				manager->Go(UIAction::Replace(new UIScene_Dialog(S("#editor_error_internal_error"), "", false), NULL));
				m_creating_mod = false;
			}
			else
			{
				Publish_Mod();
				m_creating_mod = false;
			}
		}
	}
	else if (m_uploading)
	{
		bool wasSuccessful = false;
		bool legalAgreementRequired = false;
		u64 modId;
		u64 uploadedBytes = 0;
		u64 totalBytes = 0;

		if (OnlineMods::Get()->Get_Mod_Update_Progress(wasSuccessful, legalAgreementRequired, modId, uploadedBytes, totalBytes))
		{
			// Save map with workshop id in it.
			header.Workshop_ID = modId;

			UIScene_Editor* editor = GameEngine::Get()->Get_UIManager()->Get_Scene_By_Type<UIScene_Editor*>();
			editor->Save(m_original_map_path);

			Game::Get()->Flag_Uploaded_To_Workshop();

			// Finish!
			OnlinePlatform::Get()->Show_Web_Browser(StringHelper::Format("http://steamcommunity.com/sharedfiles/filedetails/?id=%llu", modId).c_str());

			manager->Go(UIAction::Pop(new UIFadeOutTransition()));
			m_uploading = false;
		}
		else
		{
			// Update upload progress.
			float progress = totalBytes == 0 ? 0.0f : ((float)uploadedBytes / (float)totalBytes);
			Find_Element<UILabel*>("upload_label")->Set_Value(SF("#editor_publish_progress_text", progress * 100));
		}
	}

	UIScene::Tick(time, manager, scene_index);
}

void UIScene_PublishMap::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Draw(time, manager, scene_index);
}

void UIScene_PublishMap::Upload_Mod()
{
	Map* map = Game::Get()->Get_Map();
	MapFileHeaderBlock& header = map->Get_Map_Header();
	GameScene* scene = Game::Get()->Get_Game_Scene();
	EngineVirtualMachine* vm = GameEngine::Get()->Get_VM();

	UIScene_Editor* editor = GameEngine::Get()->Get_UIManager()->Get_Scene_By_Type<UIScene_Editor*>();
	std::string save_path = editor->Get_Save_Path();

	if (header.Workshop_ID != 0)
	{
		Publish_Mod();
	}
	else
	{
		OnlineMods::Get()->Create_Mod(&header.Workshop_ID, &m_creation_status);
		m_creating_mod = true;
	}
}

void UIScene_PublishMap::Publish_Mod()
{
	Map* map = Game::Get()->Get_Map();
	MapFileHeaderBlock& header = map->Get_Map_Header();
	GameScene* scene = Game::Get()->Get_Game_Scene();
	EngineVirtualMachine* vm = GameEngine::Get()->Get_VM();

	UIScene_Editor* editor = GameEngine::Get()->Get_UIManager()->Get_Scene_By_Type<UIScene_Editor*>();

	std::string save_path = editor->Get_Save_Path();
	std::string dir_path = save_path + ".workshop";
	std::string map_path = dir_path + "/map.zgmap";
	std::string preview_path = dir_path + "/preview.png";
	Platform::Get()->Create_Directory(dir_path.c_str(), false);

	CVMLinkedSymbol* game_mode_symbol = vm->Find_Class("Base_Game_Mode");
	CVMLinkedSymbol* is_main_symbol = vm->Find_Variable(game_mode_symbol, "Is_Main");

	// Check for game mode.
	std::string game_mode_name = "";
	std::vector<ScriptedActor*> actors = scene->Find_Derived_Actors(game_mode_symbol);

	for (std::vector<ScriptedActor*>::iterator iter = actors.begin(); iter != actors.end(); iter++)
	{
		ScriptedActor* actor = *iter;
		CVMContextLock lock(vm->Set_Context(actor->Get_Script_Context()));

		CVMValue is_main;
		vm->Get_Field(is_main_symbol, actor->Get_Script_Object(), is_main);
		if (is_main.int_value == 1)
		{
			game_mode_name = actor->Get_Script_Symbol()->symbol->Get_Meta_Data<std::string>("Name", "< unknown >");
		}
	}

	// Publish!
	DBG_LOG("Saving map before publish: %s", map_path.c_str());
	m_original_map_path = save_path;
	editor->Save(map_path);

	DBG_LOG("Saving map preview to: %s", preview_path.c_str());
	Pixelmap* pixmap = map->Get_Preview_Pixmap();
	PixelmapFactory::Save(preview_path.c_str(), pixmap);

	DBG_LOG("Publishing mod to id %llu ...", header.Workshop_ID);
	std::vector<std::string> tags;
	tags.push_back("Map");

	OnlineMods::Get()->Publish_Mod(
		header.Workshop_ID, 
		SF("#editor_publish_mod_name_text", header.Long_Name.c_str()),
		SF("#editor_publish_mod_description_text", header.Description.c_str(), game_mode_name.c_str(), header.Max_Players),
		dir_path,
		preview_path,
		"Mod Updated",
		tags
	);

	m_uploading = true;
}

void UIScene_PublishMap::Begin_Upload()
{
	Find_Element<UILabel*>("name_label")->Set_Visible(false);
	Find_Element<UILabel*>("description_label")->Set_Visible(false);
	Find_Element<UIPanel*>("preview_panel")->Set_Visible(false);
	Find_Element<UILabel*>("label")->Set_Visible(false);
	Find_Element<UIButton*>("agreement_button")->Set_Visible(false);
	Find_Element<UIButton*>("cancel_button")->Set_Visible(false);
	Find_Element<UIButton*>("upload_button")->Set_Visible(false);
	Find_Element<UILabel*>("upload_label")->Set_Visible(true);

	Map* map = Game::Get()->Get_Map();
	MapFileHeaderBlock& header = map->Get_Map_Header();
	if (header.Workshop_ID  != 0)
	{
		DBG_LOG("Requesting original mod information ...");

		m_requesting_mod_info = true;
	}
	else
	{
		Upload_Mod();
	}
}

void UIScene_PublishMap::Recieve_Event(UIManager* manager, UIEvent e)
{
	switch (e.Type)
	{
	case UIEventType::Button_Click:
		{	
			if (e.Source->Get_Name() == "agreement_button")
			{
				OnlinePlatform::Get()->Show_Web_Browser("http://steamcommunity.com/sharedfiles/workshoplegalagreement");
			}
			else if (e.Source->Get_Name() == "cancel_button")
			{
				manager->Go(UIAction::Pop(new UIFadeOutTransition()));
			}
			else if (e.Source->Get_Name() == "upload_button")
			{
				Begin_Upload();
			}
		}
		break;
	}
}
