// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/Editor/UIScene_Editor.h"
#include "Game/UI/Scenes/Editor/UIScene_ColorSelector.h"
#include "Game/UI/Scenes/Editor/UIScene_FileBrowser.h"
#include "Game/UI/Scenes/Editor/UIScene_ResourceBrowser.h"
#include "Game/UI/Scenes/Editor/UIScene_PublishMap.h"
#include "Game/UI/Scenes/UIScene_ConfirmDialog.h"
#include "Game/UI/Scenes/Editor/PfxEditor/UIScene_PfxEditor.h"
#include "Game/UI/Scenes/UIScene_Game.h"
#include "Game/UI/Scenes/Editor/EditorTileSelection.h"
#include "Game/UI/Scenes/UIScene_Chat.h"
#include "Engine/UI/Transitions/UIFadeInTransition.h"
#include "Engine/UI/Transitions/UIFadeInToNewTransition.h"
#include "Engine/UI/Layouts/UILayoutFactory.h"

#include "Game/Network/Packets/GamePackets.h"

#include "Engine/Display/GfxDisplay.h"

#include "Engine/Input/Input.h"

#include "Engine/UI/Elements/UIPanel.h"
#include "Engine/UI/Elements/UIComboBox.h"
#include "Engine/UI/Elements/UIMenuBar.h"
#include "Engine/UI/Elements/UITreeView.h"
#include "Engine/UI/Elements/UIPropertyGrid.h"
#include "Engine/UI/Elements/UISplitContainerItem.h"

#include "Engine/UI/UIElement.h"
#include "Engine/UI/UIManager.h"
#include "Engine/UI/Elements/UIToolbarItem.h"
#include "Engine/UI/Elements/UIMenuItem.h"

#include "Engine/Network/NetManager.h"
#include "Engine/Network/NetUser.h"
#include "Engine/Network/NetFileTransfer.h"

#include "Engine/Resources/ResourceFactory.h"

#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Renderer/Textures/Pixelmap.h"
#include "Engine/Renderer/Textures/PixelmapFactory.h"

#include "Engine/Engine/GameEngine.h"
#include "Game/Network/GameNetManager.h"
#include "Game/Network/GameNetClient.h"

#include "Game/Runner/Game.h"
#include "Game/Game/EditorManager.h"

#include "Generic/Math/Math.h"
#include "Generic/Helper/StringHelper.h"

#include "Engine/Localise/Locale.h"

#include "Engine/Scene/Scene.h"
#include "Engine/Scene/Camera.h"
#include "Game/Scene/Actors/Components/Drawable/DrawableComponent.h"
#include "Game/Scene/Cameras/EditorCamera.h"
#include "Game/Scene/Cameras/GameCamera.h"
#include "Game/Scene/Editor/EditorBackground.h"
#include "Game/Scene/Editor/EditorForeground.h"

#include "Engine/Scene/Pathing/PathManager.h"

#include "Engine/Particles/ParticleManager.h"

#include "Game/Game/VoteManager.h"
#include "Game/Game/Votes/EditModeVote.h"

#include "Game/Scene/GameScene.h"
#include "Game/Scene/Actors/ScriptedActor.h"

#include "Engine/Audio/AudioRenderer.h"

#include "Engine/Scene/Map/Map.h"
#include "Engine/Scene/Map/MapFile.h"
#include "Game/Scene/Map/MapSerializer.h"
#include "Game/Scene/Map/Blocks/MapFileLayerBlock.h"
#include "Game/Scene/Map/Blocks/MapFileObjectBlock.h"
#include "Engine/Scene/Map/Blocks/MapFileHeaderBlock.h"

#include "XScript/VirtualMachine/CVMBinary.h"

#include "Game/Version.h"

std::string GEditorLastSaveOpenFolder = "";
std::string GEditorLastSaveOpenFile = "";

UIScene_Editor::UIScene_Editor()
	: m_mode(EditorMode::Tilemap)	
	, m_tool(EditorTool::Pencil)
	, m_edit_layer(2)
	, m_flip_horizontal(false)
	, m_flip_vertical(false)
	, m_color_index(0)
	, m_collision_mode(MapTileCollisionType::None)
	, m_last_action_count(0)
	, m_supress_actions_this_frame(false)
	, m_manager(NULL)
	, m_saving(false)
	, m_dragging_objects(false)
	, m_resizing_objects(false)
	, m_rotating_objects(false)
	, m_properties_object(NULL)
	, m_new_confirm(false)
	, m_show_lighting(false)
	, m_circle_radius(8)
{
	m_manager = GameEngine::Get()->Get_UIManager();
	m_font			= m_manager->Get_Font();
	m_font_renderer	= FontRenderer(m_font, false, false);

	memset(m_layer_visibility, true, sizeof(m_layer_visibility));

	m_color_palette[0] = Color::White;
	m_color_palette[1] = Color::Black;
	m_color_palette[2] = Color::Green;
	m_color_palette[3] = Color::Blue;
	m_color_palette[4] = Color::Red;
	m_color_palette[5] = Color::Yellow;

	Set_Layout("editor");

	// Grab frames we need.
	m_background_frame = ResourceFactory::Get()->Get_Atlas_Frame("editor_background");

	// Copy game camera state to editor and begin using it.
	RenderPipeline*		pipeline	= RenderPipeline::Get();
	Game*				game		= Game::Get();
	Scene*				scene		= GameEngine::Get()->Get_Scene();
	Map*				map			= game->Get_Map();
	Atlas*				tileset		= map->Get_Tileset()->Get();
	Vector2				tile_size	= map->Get_Tile_Size();
	
	map->Set_Active_Layer_Tint(true, 2);

	AtlasFrame* tileset_frame		= tileset->Get_Frame_By_Index(0);	
	Vector2 tileset_expanse			= Vector2(tileset_frame->GridOrigin.Width, tileset_frame->GridOrigin.Height);

	Camera* game_camera				= Game::Get()->Get_Camera(CameraID::Game1); // SPLITSCREEN_TODO
	EditorCamera* editor_camera		= static_cast<EditorCamera*>(Game::Get()->Get_Camera(CameraID::Editor_Main));
	EditorCamera* tileset_camera	= static_cast<EditorCamera*>(Game::Get()->Get_Camera(CameraID::Editor_Tileset));
	EditorCamera* sub_camera		= static_cast<EditorCamera*>(Game::Get()->Get_Camera(CameraID::Editor_Sub));
	editor_camera->Set_Position(game_camera->Get_Position());
	editor_camera->Set_Rotation(game_camera->Get_Rotation());
	editor_camera->Set_Enabled(true);

	tileset_camera->Set_Position(Vector3(0, 0, 0));
	tileset_camera->Set_Rotation(Vector3(0, 0, 0));
	tileset_camera->Set_Enabled(true);

	sub_camera->Set_Enabled(false);
	
	pipeline->Set_Active_Camera(editor_camera);

	// Create background	 
	m_background = new EditorBackground(this);
	scene->Add_Drawable(m_background);

	// Create foreground.
	m_foreground = new EditorForeground(this);
	scene->Add_Drawable(m_foreground);

	// Setup selections.
	m_map_selection = EditorTileSelection(editor_camera, Vector2(map->Get_Width(), map->Get_Height()));
	m_tileset_selection = EditorTileSelection(tileset_camera, Vector2(tileset_expanse.X, tileset_expanse.Y));

	map->Set_Regenerate_Defer_Time(2.0f);

	// Reset actors.
	std::vector<Actor*> actors = scene->Get_Actors();
	for (std::vector<Actor*>::iterator iter = actors.begin(); iter != actors.end(); iter++)
	{
		ScriptedActor* actor = dynamic_cast<ScriptedActor*>(*iter);
		if (actor != NULL)
		{			
			actor->Set_Selected(false);
		}
	}

	// Fill tree view.	
	UITreeView* tree = Find_Element<UITreeView*>("object_list");
	CVirtualMachine* vm = GameEngine::Get()->Get_VM();
	CVMLinkedSymbol* actor_symbol = vm->Find_Class("Actor");
	DBG_ASSERT(actor_symbol != NULL);

	std::vector<CVMLinkedSymbol*> placable;
	Gather_Placeable_Objects(vm, actor_symbol, placable);

	//Build_Object_Tree(tree, vm, actor_symbol, NULL);
	Build_Object_Tree(tree, vm, "", NULL, placable);

	Game::Get()->Set_Update_Actors(false);

	m_original_bgm_volume = AudioRenderer::Get()->Get_BGM_Volume();
	AudioRenderer::Get()->Set_BGM_Volume(0.0f);

	RenderPipeline::Get()->Set_Ambient_Lighting(Vector4(0.5f, 0.5f, 0.5f, 1.0f));

	GEditorLastSaveOpenFile = "";

	if (GEditorLastSaveOpenFolder == "")
	{
		GEditorLastSaveOpenFolder = "../../Data/Base/Maps/";
		if (!Platform::Get()->Is_Directory(GEditorLastSaveOpenFolder.c_str()))
		{
			GEditorLastSaveOpenFolder = "../../Data/Workshop/Maps/";
			if (!Platform::Get()->Is_Directory(GEditorLastSaveOpenFolder.c_str()))
			{
				Platform::Get()->Create_Directory(GEditorLastSaveOpenFolder.c_str(), true);
			}
		}
	}
}

UIScene_Editor::~UIScene_Editor()
{
	RenderPipeline*		pipeline	= RenderPipeline::Get();
	Game*				game		= Game::Get();
	Scene*				scene		= GameEngine::Get()->Get_Scene();
	Map*				map			= game->Get_Map();

	Camera* game_camera				= Game::Get()->Get_Camera(CameraID::Game1); // SPLITSCREEN_TODO
	Camera* editor_camera			= Game::Get()->Get_Camera(CameraID::Editor_Main);

	EditorCamera* cam = static_cast<EditorCamera*>(Game::Get()->Get_Camera(CameraID::Editor_Main));
	cam->Set_Enabled(false);
	
	cam = static_cast<EditorCamera*>(Game::Get()->Get_Camera(CameraID::Editor_Tileset));
	cam->Set_Enabled(false);
	
	cam = static_cast<EditorCamera*>(Game::Get()->Get_Camera(CameraID::Editor_Sub));
	cam->Set_Enabled(false);
	
	pipeline->Set_Active_Camera(game_camera);

	map->Set_Regenerate_Defer_Time(0.0f);

	// Show all map layers.
	for (int i = 0; i < map->Get_Depth(); i++)
	{
		map->Get_Layer(i)->Set_Visible(true);
		map->Get_Layer(i)->Set_Collision_Visible(false);
	}

	RenderPipeline::Get()->Set_Draw_Entity_Debug(false);

	map->Set_Active_Layer_Tint(false, 2);

	// Reset map collision.
	CollisionManager::Get()->Reset();
	PathManager::Get()->Reset();
	map->Create_Collision();
	CollisionManager::Get()->Set_Show_Collision(false);
	PathManager::Get()->Set_Show_Paths(false);

	// Destroy editor objects.
	scene->Remove_Drawable(m_foreground);
	SAFE_DELETE(m_foreground);

	scene->Remove_Drawable(m_background);
	SAFE_DELETE(m_background);

	Game::Get()->Set_Update_Actors(true);
	AudioRenderer::Get()->Set_BGM_Volume(m_original_bgm_volume);
}

std::string UIScene_Editor::Get_Save_Path()
{
	return GEditorLastSaveOpenFile;
}

const char* UIScene_Editor::Get_Name()
{
	return "UIScene_Editor";
}

bool UIScene_Editor::Should_Render_Lower_Scenes()
{
	return false;
}

bool UIScene_Editor::Should_Display_Cursor()
{
	return true;
}
	
bool UIScene_Editor::Should_Display_Focus_Cursor()
{
	return false;
}

bool UIScene_Editor::Is_Focusable()
{
	return true;
}

bool UIScene_Editor::Should_Fade_Cursor()
{
	return false;
}

void UIScene_Editor::Enter(UIManager* manager)
{
	EditorCamera* editor_camera		= static_cast<EditorCamera*>(Game::Get()->Get_Camera(CameraID::Editor_Main));
	EditorCamera* tileset_camera	= static_cast<EditorCamera*>(Game::Get()->Get_Camera(CameraID::Editor_Tileset));
	EditorCamera* sub_camera		= static_cast<EditorCamera*>(Game::Get()->Get_Camera(CameraID::Editor_Sub));

	editor_camera->Set_Enabled(true);
	tileset_camera->Set_Enabled(true);
	sub_camera->Set_Enabled(false);

	// Refresh state.
	Refresh_State();
}	

void UIScene_Editor::Exit(UIManager* manager)
{
	Map* map = Game::Get()->Get_Map();

	for (int i = 0; i < map->Get_Depth(); i++)
	{
		map->Get_Layer(i)->Set_Visible(true);
		map->Get_Layer(i)->Set_Collision_Visible(false);
	}

	RenderPipeline::Get()->Set_Draw_Entity_Debug(false);
	CollisionManager::Get()->Set_Show_Collision(false);
	PathManager::Get()->Set_Show_Paths(false);
}

void UIScene_Editor::Refresh(UIManager* manager)
{
	// Refresh base-scene stuff first (instantiate layout etc).
	UIScene::Refresh(manager);

	// Add collision items.
	// NOTE: Keep in sync with MapTileCollisionType
	UIComboBox* box = Find_Element<UIComboBox*>("collision_box");
	box->Clear_Items();
	box->Add_Item("None");
	box->Add_Item("Solid");
	box->Add_Item("Players Only");
	box->Add_Item("Jump Down (Deprecated)");
	box->Add_Item("Jump Up (Deprecated)");
	box->Add_Item("Jump Left (Deprecated)");
	box->Add_Item("Jump Right (Deprecated)");
	box->Add_Item("Enemies Only");
	box->Add_Item("Path Blocking");
}

EditorTileSelection& UIScene_Editor::Get_Map_Selection()
{
	return m_map_selection;
}

EditorTileSelection& UIScene_Editor::Get_Tileset_Selection()
{
	return m_tileset_selection;
}

void UIScene_Editor::Gather_Placeable_Objects(CVirtualMachine* vm, CVMLinkedSymbol* parent_symbol, std::vector<CVMLinkedSymbol*>& results)
{
	bool placeable = parent_symbol->symbol->Get_Meta_Data<bool>("placeable", false);
	if (placeable == true)
	{
		results.push_back(parent_symbol);
	}

	std::vector<CVMLinkedSymbol*> derived = vm->Find_Derived_Classes(parent_symbol);

	for (std::vector<CVMLinkedSymbol*>::iterator iter = derived.begin(); iter != derived.end(); iter++)
	{
		CVMLinkedSymbol* child_sym = *iter;
		Gather_Placeable_Objects(vm, child_sym, results);
	}
}

/*
void UIScene_Editor::Build_Object_Tree(UITreeView* tree, CVirtualMachine* vm, CVMLinkedSymbol* parent_symbol, UITreeViewItem* parent_node)
{
	bool placeable = parent_symbol->symbol->Get_Meta_Data<bool>("placeable", false);

	UITreeViewItem* item = tree->Add_Item(parent_symbol->symbol->name, parent_node, placeable, parent_symbol);

	std::vector<CVMLinkedSymbol*> derived = vm->Find_Derived_Classes(parent_symbol);

	for (std::vector<CVMLinkedSymbol*>::iterator iter = derived.begin(); iter != derived.end(); iter++)
	{
		CVMLinkedSymbol* child_sym = *iter;
		Build_Object_Tree(tree, vm, child_sym, item);
	}
}
*/

void UIScene_Editor::Build_Object_Tree(UITreeView* tree, CVirtualMachine* vm, std::string path, UITreeViewItem* parent_node, std::vector<CVMLinkedSymbol*>& results)
{
	std::vector<std::string> folders;

	// Add folders.
	for (std::vector<CVMLinkedSymbol*>::iterator iter = results.begin(); iter != results.end(); iter++)
	{
		CVMLinkedSymbol* sym = *iter;

		// Grab the path to the class and strim off anything before the Data directory - TODO: Don't hardcode this.
		std::string file = vm->Get_String_Table_Entry(sym->symbol->debug.FileNameIndex);
		std::string::size_type data_offset = file.find("/Data/");
		file = StringHelper::Replace(file.substr(data_offset + 6).c_str(), "\\", "/");
		if (file[0] == '/')
		{
			file = file.substr(1);
		}

		if (file.size() > path.size())
		{
			std::string remaining = file.substr(path.size() + (path.size() > 0 ? 1 : 0));
			if (file.substr(0, path.size()) == path)
			{
				int offset = remaining.find("/");
				if (offset != std::string::npos)
				{
					remaining = remaining.substr(0, offset);
					if (std::find(folders.begin(), folders.end(), remaining) == folders.end())
					{
						UITreeViewItem* sub_node = tree->Add_Item(remaining, parent_node, false, NULL);
						Build_Object_Tree(tree, vm, (path == "" ? remaining : path + "/" + remaining), sub_node, results);

						folders.push_back(remaining);
					}
				}
			}
		}
	}

	// Add objects.
	for (std::vector<CVMLinkedSymbol*>::iterator iter = results.begin(); iter != results.end(); iter++)
	{
		CVMLinkedSymbol* sym = *iter;

		// Grab the path to the class and strim off anything before the Data directory - TODO: Don't hardcode this.
	 	std::string file = vm->Get_String_Table_Entry(sym->symbol->debug.FileNameIndex);
		std::string::size_type data_offset = file.find("/Data/");
		file = StringHelper::Replace(file.substr(data_offset + 6).c_str(), "\\", "/");
		if (file[0] == '/')
		{
			file = file.substr(1);
		}

		if (file.size() > path.size())
		{
			std::string remaining = file.substr(path.size() + (path.size() > 0 ? 1 : 0));
			if (file.substr(0, path.size()) == path)
			{
				int offset = remaining.find("/");
				if (offset == std::string::npos)
				{
					std::string name = sym->symbol->Get_Meta_Data<std::string>("name", sym->symbol->name);
					tree->Add_Item(name, parent_node, true, sym);
				}
			}
		}
	}
}

void UIScene_Editor::Set_Script_Field_Value(UIPropertyGridItem* node, void* value)
{
	CVirtualMachine* vm = GameEngine::Get()->Get_VM();

	CVMObject* base_obj = reinterpret_cast<CVMObject*>(node->Meta_Data);
	CVMContext* ctx = reinterpret_cast<CVMContext*>(node->Meta_Data_2);
	std::string path = node->Meta_Data_3;

	CVMContextLock lock = vm->Set_Context(ctx);
	
	//DBG_LOG("Setting %s::%s", base_obj->Get_Symbol()->symbol->name, path.c_str());

	CVMValue val;

	switch (node->Type)
	{
	case UIPropertyGridDataType::String:	val.string_value = (*reinterpret_cast<std::string*>(value)).c_str();	break;

	case UIPropertyGridDataType::Int:		
	case UIPropertyGridDataType::Combo:		val.int_value = *reinterpret_cast<int*>(value);							break;

	case UIPropertyGridDataType::Float:		val.float_value = *reinterpret_cast<float*>(value);						break;
	case UIPropertyGridDataType::Bool:		val.int_value = *reinterpret_cast<bool*>(value) ? 1 : 0;				break;
	}

	vm->Set_Value(base_obj, path, val);
}

void UIScene_Editor::Get_Script_Field_Value(UIPropertyGridItem* node, void* value)
{
	CVirtualMachine* vm = GameEngine::Get()->Get_VM();

	CVMObject* base_obj = reinterpret_cast<CVMObject*>(node->Meta_Data);
	CVMContext* ctx = reinterpret_cast<CVMContext*>(node->Meta_Data_2);
	std::string path = node->Meta_Data_3;

	CVMContextLock lock = vm->Set_Context(ctx);

	//DBG_LOG("Getting %s::%s", base_obj->Get_Symbol()->symbol->name, path.c_str());

	CVMValue val;
	vm->Get_Value(base_obj, path, val);
	
	switch (node->Type)
	{
	case UIPropertyGridDataType::String:	*reinterpret_cast<std::string*>(value) = val.string_value.C_Str();	break;

	case UIPropertyGridDataType::Int:		
	case UIPropertyGridDataType::Combo:		*reinterpret_cast<int*>(value) = val.int_value;						break;

	case UIPropertyGridDataType::Float:		*reinterpret_cast<float*>(value) = val.float_value;					break;
	case UIPropertyGridDataType::Bool:		*reinterpret_cast<bool*>(value) = val.int_value ? 1 : 0;			break;
	}
}

void UIScene_Editor::Add_Script_Field(UIPropertyGrid* grid, UIPropertyGridItem* node, CVMLinkedSymbol* sym, CVMObjectHandle handle, std::string path, CVMObjectHandle base_handle)
{
	CVirtualMachine* vm = GameEngine::Get()->Get_VM();

	// Get correct data type.
	CVMDataType* dt = NULL;
	if (sym->symbol->type == SymbolType::ClassProperty)
	{
		dt = sym->symbol->property_data->data_type;
	}
	else if (sym->symbol->type == SymbolType::ClassField)
	{
		dt = sym->symbol->field_data->data_type;
	}
	
	// Update path.
	if (path != "")
		path = path + "." + sym->symbol->name;
	else 
		path = sym->symbol->name;

	// Work out display name.
	std::string name = StringHelper::Replace(sym->symbol->name, "_", " ");
	CVMObject* obj = handle.Get();

	std::string edit_type = sym->symbol->Get_Meta_Data<std::string>("EditType", "");
	UIPropertyGridItem* sub_node = NULL;

	if (sym->symbol->Get_Meta_Data<bool>("Deprecated", false))
	{
		return;
	}

	if (edit_type != "")
	{
		sub_node = grid->Add_Item(name, node, UIPropertyGridDataType::External, NULL, std::vector<std::string>(), sym);
		sub_node->Meta_Data_3 = path;
	}
	else
	{
		// Create elements by type.
		switch (dt->type)
		{
		case CVMBaseDataType::String:	sub_node = grid->Add_Custom_Item(name, node, UIPropertyGridDataType::String, &UIScene_Editor::Get_Script_Field_Value, &UIScene_Editor::Set_Script_Field_Value, std::vector<std::string>(), base_handle.Get(), sym->index, vm->Get_Active_Context(), path);	break;
		case CVMBaseDataType::Int:		sub_node = grid->Add_Custom_Item(name, node, UIPropertyGridDataType::Int, &UIScene_Editor::Get_Script_Field_Value, &UIScene_Editor::Set_Script_Field_Value, std::vector<std::string>(), base_handle.Get(), sym->index, vm->Get_Active_Context(), path);		break;
		case CVMBaseDataType::Float:	sub_node = grid->Add_Custom_Item(name, node, UIPropertyGridDataType::Float, &UIScene_Editor::Get_Script_Field_Value, &UIScene_Editor::Set_Script_Field_Value, std::vector<std::string>(), base_handle.Get(), sym->index, vm->Get_Active_Context(), path);	break;
		case CVMBaseDataType::Bool:		sub_node = grid->Add_Custom_Item(name, node, UIPropertyGridDataType::Bool, &UIScene_Editor::Get_Script_Field_Value, &UIScene_Editor::Set_Script_Field_Value, std::vector<std::string>(), base_handle.Get(), sym->index, vm->Get_Active_Context(), path);	break;
		case CVMBaseDataType::Object:	sub_node = grid->Add_Item(name, node);	break;
		case CVMBaseDataType::Array:	sub_node = grid->Add_Item(name, node);	break;
		}
	}

	// Add sub-fields if object.
	if (dt->type == CVMBaseDataType::Object)
	{
		CVMValue class_handle;
		if (sym->symbol->type == SymbolType::ClassField)
		{
			vm->Get_Field(sym, handle, class_handle);
		}
		else if (sym->symbol->type == SymbolType::ClassProperty)
		{
			CVMLinkedSymbol* get_method = vm->Get_Symbol_Table_Entry(sym->symbol->property_data->get_property_symbol);
			vm->Invoke(get_method, CVMValue(handle), false, false);
			vm->Get_Return_Value(class_handle);
		}

		Add_Script_Fields(grid, sub_node, class_handle.object_value, path, base_handle);
	}

	// Add elements if array.
	else if (dt->type == CVMBaseDataType::Array)
	{
		for (int i = 0; i < handle.Get()->Slot_Count(); i++)
		{
			std::string element_name = StringHelper::Format("[%i]", i);
			UIPropertyGridItem* element_node = NULL;

			CVMDataType* element_dt = dt->element_type;

			// Create elements by type.
			switch (element_dt->type)
			{
			case CVMBaseDataType::String:	element_node = grid->Add_Custom_Item(element_name, sub_node, UIPropertyGridDataType::String, &UIScene_Editor::Get_Script_Field_Value, &UIScene_Editor::Set_Script_Field_Value, std::vector<std::string>(), base_handle.Get(), sym->index, vm->Get_Active_Context(), path);	break;
			case CVMBaseDataType::Int:		element_node = grid->Add_Custom_Item(element_name, sub_node, UIPropertyGridDataType::Int, &UIScene_Editor::Get_Script_Field_Value, &UIScene_Editor::Set_Script_Field_Value, std::vector<std::string>(), base_handle.Get(), sym->index, vm->Get_Active_Context(), path);		break;
			case CVMBaseDataType::Float:	element_node = grid->Add_Custom_Item(element_name, sub_node, UIPropertyGridDataType::Float, &UIScene_Editor::Get_Script_Field_Value, &UIScene_Editor::Set_Script_Field_Value, std::vector<std::string>(), base_handle.Get(), sym->index, vm->Get_Active_Context(), path);	break;
			case CVMBaseDataType::Bool:		element_node = grid->Add_Custom_Item(element_name, sub_node, UIPropertyGridDataType::Bool, &UIScene_Editor::Get_Script_Field_Value, &UIScene_Editor::Set_Script_Field_Value, std::vector<std::string>(), base_handle.Get(), sym->index, vm->Get_Active_Context(), path);	break;
			case CVMBaseDataType::Object:	element_node = grid->Add_Item(element_name, sub_node);	break;
			case CVMBaseDataType::Array:	element_node = grid->Add_Item(element_name, sub_node);	break;
			}

			if (element_dt->type == CVMBaseDataType::Object || element_dt->type == CVMBaseDataType::Array)
			{
				Add_Script_Field(grid, element_node, handle.Get()->Get_Slot(i).object_value.Get()->Get_Symbol(), handle.Get()->Get_Slot(i).object_value, path, base_handle);
			}
		}
	}
}

struct Sorted_Script_Field
{
	bool bHasChildren;
	CVMLinkedSymbol* symbol;
};

bool Script_Field_Sort_Predicate(const Sorted_Script_Field& a, const Sorted_Script_Field& b)
{
	if (a.bHasChildren && !b.bHasChildren)
	{
		return true;
	}
	else if (!a.bHasChildren && b.bHasChildren)
	{
		return false;
	}
	else
	{
		return strcmp(a.symbol->symbol->name, b.symbol->symbol->name) < 0;
	}
}

bool Script_Field_Has_Children(CVMLinkedSymbol* sym)
{
	CVirtualMachine* vm = GameEngine::Get()->Get_VM();

	CVMDataType* dt = NULL;
	if (sym->symbol->type == SymbolType::ClassProperty)
	{
		dt = sym->symbol->property_data->data_type;
	}
	else if (sym->symbol->type == SymbolType::ClassField)
	{
		dt = sym->symbol->field_data->data_type;
	}

	if (dt)
	{
		CVMLinkedSymbol* dt_sym = vm->Get_Symbol_Table_Entry(dt->class_index);
		if (dt_sym)
		{		
			for (int i = 0; i < dt_sym->symbol->children_count; i++)
			{
				CVMLinkedSymbol* sub_sym = vm->Get_Symbol_Table_Entry(dt_sym->symbol->children_indexes[i]);
				if (sub_sym->symbol->type == SymbolType::ClassProperty)
				{
					if (sub_sym->symbol->property_data->is_serialized != 0)
					{
						return true;
					}
				}
				else if (sub_sym->symbol->type == SymbolType::ClassField)
				{
					if (sub_sym->symbol->field_data->is_serialized != 0)
					{
						return true;
					}
				}
			}
		}
	}

	return false;
}

void UIScene_Editor::Add_Script_Fields(UIPropertyGrid* grid, UIPropertyGridItem* node, CVMObjectHandle handle, std::string path, CVMObjectHandle base_handle)
{
	CVirtualMachine* vm = GameEngine::Get()->Get_VM();

	CVMObject* obj = handle.Get();
	CVMBinarySymbol* sym = obj->Get_Symbol()->symbol;

	std::vector<Sorted_Script_Field> sorted_symbols;

	for (int i = 0; i < sym->children_count; i++)
	{
		CVMLinkedSymbol* sub_sym = vm->Get_Symbol_Table_Entry(sym->children_indexes[i]);
		if (sub_sym->symbol->type == SymbolType::ClassProperty)
		{
			if (sub_sym->symbol->property_data->is_serialized != 0)
			{
				Sorted_Script_Field sorted_sym;
				sorted_sym.bHasChildren = Script_Field_Has_Children(sub_sym);
				sorted_sym.symbol = sub_sym;
				sorted_symbols.push_back(sorted_sym);
			}
		}
		else if (sub_sym->symbol->type == SymbolType::ClassField)
		{
			if (sub_sym->symbol->field_data->is_serialized != 0)
			{
				Sorted_Script_Field sorted_sym;
				sorted_sym.bHasChildren = Script_Field_Has_Children(sub_sym);
				sorted_sym.symbol = sub_sym;
				sorted_symbols.push_back(sorted_sym);
			}
		}
	}
	
	std::sort(sorted_symbols.begin(), sorted_symbols.end(), Script_Field_Sort_Predicate);

	for (unsigned int i = 0; i < sorted_symbols.size(); i++)
	{
		Add_Script_Field(grid, node, sorted_symbols[i].symbol, handle, path, base_handle);
	}
}

void UIScene_Editor::Show_Properties(ScriptedActor* object)
{
	UIPropertyGrid* grid = Find_Element<UIPropertyGrid*>("properties_grid");
	grid->Clear_Items();

	if (object != NULL)
	{
		// Add script values.	
		{
			CVirtualMachine* vm = GameEngine::Get()->Get_VM();
			CVMContextLock lock = vm->Set_Context(object->Get_Script_Context());
			Add_Script_Fields(grid, NULL, object->Get_Script_Object(), "", object->Get_Script_Object());
		}

		Store_Old_Object_Properties();
	}

	m_properties_object = object;
}

void UIScene_Editor::Show_Map_Properties()
{
	Map* map = Game::Get()->Get_Map();
	MapFileHeaderBlock* block = &map->Get_Map_Header();

	UIPropertyGrid* grid = Find_Element<UIPropertyGrid*>("properties_grid");

	m_properties_object = NULL;
	m_mode = EditorMode::Objects;
	Refresh_State();

	// Add script values.	
	grid->Clear_Items();
	grid->Add_Item("Short Name", NULL, UIPropertyGridDataType::String, &block->Short_Name);
	grid->Add_Item("Long Name", NULL, UIPropertyGridDataType::String, &block->Long_Name);
	grid->Add_Item("Author", NULL, UIPropertyGridDataType::String, &block->Author);
	grid->Add_Item("Description", NULL, UIPropertyGridDataType::String, &block->Description);
	grid->Add_Item("Loading Slogan", NULL, UIPropertyGridDataType::String, &block->Loading_Slogan);
	grid->Add_Item("Min Players", NULL, UIPropertyGridDataType::Int, &block->Min_Players);
	grid->Add_Item("Max Players", NULL, UIPropertyGridDataType::Int, &block->Max_Players);
	grid->Add_Item("Music Name", NULL, UIPropertyGridDataType::External, NULL, std::vector<std::string>(), &block->Music_Path);
	grid->Add_Item("Preview Path", NULL, UIPropertyGridDataType::String, &block->Preview_Path);
	grid->Add_Item("Tileset Name", NULL, UIPropertyGridDataType::String, &block->Tileset_Name);
	grid->Add_Item("Intro Cutscene", NULL, UIPropertyGridDataType::String, &block->Intro_Cutscene);
	grid->Add_Item("Outro Cutscene", NULL, UIPropertyGridDataType::String, &block->Outro_Cutscene);
	grid->Add_Item("Width", NULL, UIPropertyGridDataType::Int, &block->Width);
	grid->Add_Item("Height", NULL, UIPropertyGridDataType::Int, &block->Height);
}

void UIScene_Editor::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	bool is_topmost = (manager->Get_Top_Scene_Index() == scene_index);

	GfxDisplay*		display			= GfxDisplay::Get();
	Camera*			last_camera		= RenderPipeline::Get()->Get_Active_Camera();
	RenderPipeline* pipeline		= RenderPipeline::Get();
	Renderer*		renderer		= Renderer::Get();
	Game*			game			= Game::Get();
	Input*			input			= Input::Get();
	MouseState*		mouse_state		= input->Get_Mouse_State();

	UIScene_Game*	game_scene		= reinterpret_cast<UIScene_Game*>(manager->Get_Scene_By_Name("UIScene_Game"));
	
	UIMenuBar*		menu_bar		= Find_Element<UIMenuBar*>("menu");

	// Switch to game?
	if (GameNetManager::Get()->Game_Client()->Get_Server_State().In_Editor_Mode == false)
	{
		manager->Go(UIAction::Pop(NULL, "UIScene_Game"));
	}
		
	if (is_topmost)
	{
		// Open the chat menu.
		if (Input::Get()->Was_Pressed_By_Any(OutputBindings::Chat) && dynamic_cast<UITextBox*>(manager->Get_Focused_Element()) == NULL)
		{
			manager->Play_UI_Sound(UISoundType::Small_Click);
			manager->Go(UIAction::Push(new UIScene_Chat(game_scene, false), new UIFadeInTransition()));
			return;
		}

#ifndef MASTER_BUILD
		if (Input::Get()->Was_Pressed(InputBindings::Keyboard_F2))
		{
			manager->Play_UI_Sound(UISoundType::Small_Click);
			VoteManager::Get()->Call_Vote(new EditModeVote());
			return;
		}
#endif


		if (Input::Get()->Is_Down(InputBindings::Keyboard_LeftCtrl) ||
			Input::Get()->Is_Down(InputBindings::Keyboard_RightCtrl))
		{
			// Cut
			if (Input::Get()->Is_Down(InputBindings::Keyboard_X) && !Input::Get()->Was_Down(InputBindings::Keyboard_X))
			{
				if (Can_Cut())
					Cut();
			}
			// Copy
			else if (Input::Get()->Is_Down(InputBindings::Keyboard_C) && !Input::Get()->Was_Down(InputBindings::Keyboard_C))
			{
				if (Can_Copy())
					Copy();
			}
			// Paste
			else if (Input::Get()->Is_Down(InputBindings::Keyboard_V) && !Input::Get()->Was_Down(InputBindings::Keyboard_V))
			{
				if (Can_Paste())
					Paste();					
			}

			// Undo
			if (Input::Get()->Is_Down(InputBindings::Keyboard_Z) && !Input::Get()->Was_Down(InputBindings::Keyboard_Z))
			{
				if (EditorManager::Get()->Get_Undo_Stack_Size() > 0)
					EditorManager::Get()->Replicate_Client_Undo();

				Objects_Deleted();
			}
			// Redo
			else if (Input::Get()->Is_Down(InputBindings::Keyboard_Y) && !Input::Get()->Was_Down(InputBindings::Keyboard_Y))
			{
				if (EditorManager::Get()->Get_Redo_Stack_Size() > 0)
					EditorManager::Get()->Replicate_Client_Redo();
				
				Objects_Deleted();
			}
		}
		// Delete
		else if (Input::Get()->Was_Pressed(InputBindings::Keyboard_Delete) && Can_Cut())
		{
			Action_Delete();
		}
	}

	// Update based on type.
	if (!m_supress_actions_this_frame)
	{
		if (!menu_bar->Is_Context_Open())
		{
			switch (m_mode)
			{
			case EditorMode::Tilemap:	Update_Mode_Tilemap();		break;
			case EditorMode::Collision:	Update_Mode_Collision();	break;
			case EditorMode::Objects:	Update_Mode_Objects();		break;
			}
		}
		else
		{
			if (m_tool != EditorTool::Select &&
				m_mode != EditorMode::Objects)
			{
				m_map_selection.Clear();
			}
		}
	}
	else
	{
		if (!Input::Get()->Is_Down(InputBindings::Mouse_Left) &&
			!Input::Get()->Was_Pressed(InputBindings::Mouse_Left))
		{
			m_supress_actions_this_frame = false;
			DBG_LOG("Unsupressing actions!");
		}

		m_map_selection.Clear();
	}

	// Update state if an event has happened.
	int action_count = EditorManager::Get()->Get_Editor_Action_Index();
	if (action_count != m_last_action_count)
	{
		Refresh_State();

		Update_Object_Selection();

		m_last_action_count = action_count;
	}

	UIScene::Tick(time, manager, scene_index);
}

void UIScene_Editor::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	// Get some junk we need.
	GfxDisplay*		display		= GfxDisplay::Get();
	Camera*			last_camera = RenderPipeline::Get()->Get_Active_Camera();
	RenderPipeline* pipeline	= RenderPipeline::Get();
	Renderer*		renderer	= Renderer::Get();
	Game*			game		= Game::Get();
	Map*			map			= game->Get_Map();
	Atlas*			tileset		= map->Get_Tileset()->Get();
	Vector2			tile_size	= map->Get_Tile_Size();

	UIScene*		game_scene		= manager->Get_Scene_By_Name("UIScene_Game");
	EditorCamera*	editor_camera	= static_cast<EditorCamera*>(game->Get_Camera(CameraID::Editor_Main));
	EditorCamera*	tileset_camera	= static_cast<EditorCamera*>(game->Get_Camera(CameraID::Editor_Tileset));
	EditorCamera*	ui_camera		= static_cast<EditorCamera*>(game->Get_Camera(CameraID::UI));
	
	UIPanel*		camera_panel		= Find_Element<UIPanel*>("camera_panel");
	UIPanel*		tileset_panel		= Find_Element<UIPanel*>("tileset_panel");
	Rect2D			camera_panel_box	= camera_panel->Get_Screen_Box();
	Rect2D			tileset_panel_box	= tileset_panel->Get_Screen_Box();

	// Update input-active areas of cameras.	
	tileset_camera->Set_Input_Viewport(tileset_panel_box);
	tileset_camera->Set_Viewport(tileset_panel_box);
	editor_camera->Set_Input_Viewport(camera_panel_box);

	// Draw all post scene elements.
	UIScene::Draw(time, manager, scene_index);

	// Set tileset viewport.
	pipeline->Flush_Batches();
	pipeline->Set_Render_Batch_Scissor_Rectangle(tileset_panel_box);
	pipeline->Set_Render_Batch_Scissor_Test(true);
	pipeline->Set_Active_Camera(tileset_camera);
	renderer->Set_Depth_Test(false);
	renderer->Set_Viewport(tileset_panel_box);

	// Draw tileset.
	if (m_mode == EditorMode::Objects)
	{
		// Draw object selection.
	}
	else
	{
		Draw_Tileset(time, manager, scene_index);
	}

	// Clear viewport.
	pipeline->Flush_Batches();
	renderer->Set_Depth_Test(false);
	renderer->Set_Viewport(ui_camera->Get_Viewport());
	pipeline->Set_Render_Batch_Scissor_Test(false);	
	pipeline->Set_Active_Camera(ui_camera);

	// Draw some debug info.
	NetConnection* connection = NetManager::Get()->Client()->Get_Connection();

	NetFileTransfer* send_transfer = connection->Get_File_Transfer(INTERNAL_EDITOR_MAP_GUID, false);
	NetFileTransfer* recv_transfer = connection->Get_File_Transfer(INTERNAL_EDITOR_MAP_GUID, true);

	std::string txt = "";
	if (send_transfer != NULL && !send_transfer->Is_Finished())
		txt = "Sending map to server ...";
	else if (recv_transfer != NULL && !recv_transfer->Is_Finished())
		txt = "Receiving map from server ...";

	if (txt != "")
	{
		Rect2D txt_box = Rect2D(camera_panel_box.X, camera_panel_box.Y + camera_panel_box.Height - 16.0f, camera_panel_box.Width, 16.0f);

		PrimitiveRenderer pr;
		pr.Draw_Solid_Quad(txt_box, Color::Black);
		m_font_renderer.Draw_String(txt.c_str(), Rect2D(txt_box.X + 5.0f, txt_box.Y, txt_box.Width, txt_box.Height), 8.0f, Color::White, TextAlignment::Left, TextAlignment::Center);
	}

	// Draw game overlays.
	static_cast<UIScene_Game*>(manager->Get_Scene(scene_index - 1))->Draw(time, manager, scene_index - 1);
}

void UIScene_Editor::Draw_Tileset(const FrameTime& time, UIManager* manager, int scene_index)
{
	Game*			game				= Game::Get();
	Map*			map					= game->Get_Map();
	Atlas*			tileset				= map->Get_Tileset()->Get();
	Vector2			tile_size			= map->Get_Tile_Size();
	RenderPipeline* pipeline			= RenderPipeline::Get();
	Renderer*		renderer			= Renderer::Get();
	EditorCamera*	tileset_camera		= static_cast<EditorCamera*>(game->Get_Camera(CameraID::Editor_Tileset));
	UIPanel*		tileset_panel		= Find_Element<UIPanel*>("tileset_panel");
	Rect2D			tileset_panel_box	= tileset_panel->Get_Screen_Box();

	AtlasRenderer tileset_renderer;
	PrimitiveRenderer primitive_renderer;
	
	AtlasFrame* tileset_frame	= tileset->Get_Frame_By_Index(0);	
	Vector2 tileset_expanse		= Vector2(tileset_frame->GridOrigin.Width, tileset_frame->GridOrigin.Height);
	float tileset_scale			= tileset_camera->Get_Zoom();
	Rect2D  tileset_rect		= Rect2D(0.0f, 0.0f, tileset_expanse.X * tile_size.X, tileset_expanse.Y * tile_size.Y);
	float span					= 999999999.0f;

	// Draw checkered background.
	tileset_renderer.Tile_Frame(
		m_background_frame, 
		tileset_rect, 
		0.0f, 
		Color::White,
		false, 
		false, 
		RendererOption::E_Src_Alpha_One_Minus_Src_Alpha, 
		Vector2(1.0f, 1.0f), 
		0.0f
	);

	// Draw actual tiles.
	std::vector<Animation> animations = map->Get_All_Tile_Animations();
	for (int i = 0; i < tileset->Get_Frame_Count(); i++)
	{		
		AtlasFrame* tile = tileset->Get_Frame_By_Index(i);	
		Rect2D area = Rect2D(tile->GridOrigin.X * tile_size.X, tile->GridOrigin.Y * tile_size.Y, tile_size.X, tile_size.Y);

		// TODO: Slow and stupid. Fix.
		for (std::vector<Animation>::iterator iter = animations.begin(); iter != animations.end(); iter++)
		{
			Animation& anim = *iter;
			if (anim.Get_Start_Frame() == i)
			{
				int frame = anim.Get_Frame();
				tile = tileset->Get_Frame_By_Index(frame);
				break;
			}
		}
		
		tileset_renderer.Draw_Frame(tile, area, 0.0f, m_color_palette[m_color_index], m_flip_horizontal, m_flip_vertical);
	}	

	// Outline of map.
	primitive_renderer.Draw_Wireframe_Quad(tileset_rect, 0.0f, Color::Gray, 2.0f);

	// Horizontal expanse lines.
	primitive_renderer.Draw_Line(Vector3(0.0f, -span, 0.0f), Vector3(0.0f, span, 0.0f), 2.0f, Color::Gray);
	primitive_renderer.Draw_Line(Vector3(tileset_rect.Width, -span, 0.0f), Vector3(tileset_rect.Width, span, 0.0f), 2.0f, Color::Gray);

	// Vertical expanse lines.
	primitive_renderer.Draw_Line(Vector3(-span, 0.0f, 0.0f), Vector3(span, 0.0f, 0.0f), 2.0f, Color::Gray);
	primitive_renderer.Draw_Line(Vector3(-span, tileset_rect.Height, 0.0f), Vector3(span, tileset_rect.Height, 0.0f), 2.0f, Color::Gray);
	
	// Draw cursor box.
	m_tileset_selection.Draw(time, pipeline);
}

void UIScene_Editor::Draw_Selection(const FrameTime& time, RenderPipeline* pipeline)
{	
	// Draw map selection.
	if (m_tool == EditorTool::Circle)
	{
		m_map_selection.Draw_Radius(time, pipeline, (float)m_circle_radius);
	}
	else
	{
		m_map_selection.Draw(time, pipeline);
	}

	// Draw union of selected objects.
	if (m_mode == EditorMode::Objects &&
		m_selected_objects.size() > 0)
	{
		float half_grab_size = (float)(grab_block_size / 2);
		float full_grab_size = (float)grab_block_size;

		Rect2D tl_grab = Rect2D(m_selected_objects_union_rect.X - half_grab_size, m_selected_objects_union_rect.Y - half_grab_size, full_grab_size, full_grab_size);
		Rect2D tr_grab = Rect2D(m_selected_objects_union_rect.X + m_selected_objects_union_rect.Width - half_grab_size, m_selected_objects_union_rect.Y - half_grab_size, full_grab_size, full_grab_size);
		Rect2D bl_grab = Rect2D(m_selected_objects_union_rect.X - half_grab_size, m_selected_objects_union_rect.Y + m_selected_objects_union_rect.Height - half_grab_size, full_grab_size, full_grab_size);
		Rect2D br_grab = Rect2D(m_selected_objects_union_rect.X + m_selected_objects_union_rect.Width - half_grab_size, m_selected_objects_union_rect.Y + m_selected_objects_union_rect.Height - half_grab_size, full_grab_size, full_grab_size);
		Rect2D rot_grab = Rect2D(m_selected_objects_union_rect.X + (m_selected_objects_union_rect.Width * 0.5f) - half_grab_size, m_selected_objects_union_rect.Y + m_selected_objects_union_rect.Height - half_grab_size, full_grab_size, full_grab_size);

		PrimitiveRenderer pr;
		pr.Draw_Solid_Quad(m_selected_objects_union_rect, Color(51, 255, 153, 128));
		pr.Draw_Wireframe_Quad(m_selected_objects_union_rect, 0.0f, Color(51, 255, 153, 255), 1.0f);
		pr.Draw_Solid_Quad(tl_grab, Color(0, 99, 198, 255));
		pr.Draw_Solid_Quad(tr_grab, Color(0, 99, 198, 255));
		pr.Draw_Solid_Quad(bl_grab, Color(0, 99, 198, 255));
		pr.Draw_Solid_Quad(br_grab, Color(0, 99, 198, 255));
		pr.Draw_Solid_Oval(rot_grab, Color(198, 99, 0, 255));
	}
}

void UIScene_Editor::Recieve_Event(UIManager* manager, UIEvent e)
{
	switch (e.Type)
	{
	case UIEventType::ComboBox_SelectedIndexChanged:
		{
			if (e.Source->Get_Name() == "collision_box")
			{
				UIComboBox* box = dynamic_cast<UIComboBox*>(e.Source);
				m_collision_mode = (MapTileCollisionType::Type)box->Get_Selected_Item_Index();
				DBG_LOG("Changed collision to mode %i", m_collision_mode);
			}

			break;
		}

	case UIEventType::ToolbarItem_Click:
		if (e.Source->Get_Name() == "select_toolbar_item")
			Action_ChangeTool(EditorTool::Select);
		if (e.Source->Get_Name() == "paint_toolbar_item")
			Action_ChangeTool(EditorTool::Pencil);
		if (e.Source->Get_Name() == "fill_toolbar_item")
			Action_ChangeTool(EditorTool::Fill);
		if (e.Source->Get_Name() == "erase_toolbar_item")
			Action_ChangeTool(EditorTool::Erase);
		if (e.Source->Get_Name() == "circle_toolbar_item")
			Action_ChangeTool(EditorTool::Circle);
							
		if (e.Source->Get_Name() == "tilemap_toolbar_item")
			Action_ChangeMode(EditorMode::Tilemap);
		if (e.Source->Get_Name() == "collision_toolbar_item")
			Action_ChangeMode(EditorMode::Collision);
		if (e.Source->Get_Name() == "object_toolbar_item")
			Action_ChangeMode(EditorMode::Objects);
						
		if (e.Source->Get_Name() == "increase_toolbar_item")
		{
			m_circle_radius = Min(128, m_circle_radius + 1);
			Refresh_State();
		}
		if (e.Source->Get_Name() == "decrease_toolbar_item")
		{
			m_circle_radius = Max(0, m_circle_radius - 1);
			Refresh_State();
		}

		if (e.Source->Get_Name() == "layer1_toolbar_item")
			Action_ChangeEditLayer(0);
		if (e.Source->Get_Name() == "layer2_toolbar_item")
			Action_ChangeEditLayer(1);
		if (e.Source->Get_Name() == "layer3_toolbar_item")
			Action_ChangeEditLayer(2);
		if (e.Source->Get_Name() == "layer4_toolbar_item")
			Action_ChangeEditLayer(3);
		if (e.Source->Get_Name() == "layer5_toolbar_item")
			Action_ChangeEditLayer(4);
						
		if (e.Source->Get_Name() == "flip_h_toolbar_item")
			Action_ToggleFlipHorizontal();
		if (e.Source->Get_Name() == "flip_v_toolbar_item")
			Action_ToggleFlipVertical();
			
		if (e.Source->Get_Name() == "palette_color1_item")
			Action_SelectColorPalette(0);
		if (e.Source->Get_Name() == "palette_color2_item")
			Action_SelectColorPalette(1);
		if (e.Source->Get_Name() == "palette_color3_item")
			Action_SelectColorPalette(2);
		if (e.Source->Get_Name() == "palette_color4_item")
			Action_SelectColorPalette(3);
		if (e.Source->Get_Name() == "palette_color5_item")
			Action_SelectColorPalette(4);
		if (e.Source->Get_Name() == "palette_color6_item")
			Action_SelectColorPalette(5);

		if (e.Source->Get_Name() == "map_properties_toolbar_item")
			Show_Map_Properties();

		if (e.Source->Get_Name() == "delete_toolbar_item")
			Action_Delete();


		m_supress_actions_this_frame = true;

		break;
		
	case UIEventType::ToolbarItem_RightClick:
		if (e.Source->Get_Name() == "palette_color1_item")
		{			
			Action_SelectColorPalette(0);
			manager->Go(UIAction::Push(new UIScene_ColorSelector(&m_color_palette[0]), new UIFadeInTransition()));
		}
		if (e.Source->Get_Name() == "palette_color2_item")
		{			
			Action_SelectColorPalette(1);
			manager->Go(UIAction::Push(new UIScene_ColorSelector(&m_color_palette[1]), new UIFadeInTransition()));
		}
		if (e.Source->Get_Name() == "palette_color3_item")
		{			
			Action_SelectColorPalette(2);
			manager->Go(UIAction::Push(new UIScene_ColorSelector(&m_color_palette[2]), new UIFadeInTransition()));
		}
		if (e.Source->Get_Name() == "palette_color4_item")
		{			
			Action_SelectColorPalette(3);
			manager->Go(UIAction::Push(new UIScene_ColorSelector(&m_color_palette[3]), new UIFadeInTransition()));
		}
		if (e.Source->Get_Name() == "palette_color5_item")
		{			
			Action_SelectColorPalette(4);
			manager->Go(UIAction::Push(new UIScene_ColorSelector(&m_color_palette[4]), new UIFadeInTransition()));
		}
		if (e.Source->Get_Name() == "palette_color6_item")
		{			
			Action_SelectColorPalette(5);
			manager->Go(UIAction::Push(new UIScene_ColorSelector(&m_color_palette[5]), new UIFadeInTransition()));
		}

		m_supress_actions_this_frame = true;

		break;

	case UIEventType::MenuItem_Click:
		if (e.Source->Get_Name() == "new_menu_item")
			Begin_New();
		if (e.Source->Get_Name() == "open_menu_item")
			Begin_Open();
		if (e.Source->Get_Name() == "save_menu_item")
			Begin_Save();
		if (e.Source->Get_Name() == "save_as_menu_item")
			Begin_SaveAs();
		if (e.Source->Get_Name() == "publish_menu_item")
		{
			manager->Go(UIAction::Push(new UIScene_PublishMap(), NULL));		
		}

		if (e.Source->Get_Name() == "return_menu_item")
		{
			VoteManager::Get()->Call_Vote(new EditModeVote());
		}
		if (e.Source->Get_Name() == "exit_menu_item")
		{
			NetManager::Get()->Leave_Network_Game();
			manager->Play_UI_Sound(UISoundType::Exit_SubMenu);

			if (manager->Get_Scene_By_Name("UIScene_GameSetupMenu") != NULL)
				manager->Go(UIAction::Pop(new UIFadeInToNewTransition(), "UIScene_GameSetupMenu"));		
			else
				manager->Go(UIAction::Pop(new UIFadeInToNewTransition(), "UIScene_PlayerLogin"));		
		}

		if (e.Source->Get_Name() == "undo_menu_item")
		{
			EditorManager::Get()->Replicate_Client_Undo();
		}
		if (e.Source->Get_Name() == "redo_menu_item")
		{
			EditorManager::Get()->Replicate_Client_Redo();
		}
		
		if (e.Source->Get_Name() == "show_collision_menu_item")
			Action_ToggleCollision();
		if (e.Source->Get_Name() == "show_lighting_menu_item")
			Action_ToggleLighting();
		if (e.Source->Get_Name() == "show_path_menu_item")
			Action_TogglePaths();
		if (e.Source->Get_Name() == "show_randgen_menu_item")
			Action_ToggleRandGen();
		if (e.Source->Get_Name() == "show_grid_menu_item")
			Action_ToggleGrid();
		
		if (e.Source->Get_Name() == "cut_menu_item")
			Cut();					
		if (e.Source->Get_Name() == "copy_menu_item")
			Copy();
		if (e.Source->Get_Name() == "paste_menu_item")
			Paste();

		if (e.Source->Get_Name() == "delete_menu_item")
			Action_Delete();

		if (e.Source->Get_Name() == "reset_camera_menu_item")
			Action_ResetCamera();		

		if (e.Source->Get_Name() == "layer1_menu_item")
			Action_ChangeEditLayer(0);
		if (e.Source->Get_Name() == "layer2_menu_item")
			Action_ChangeEditLayer(1);
		if (e.Source->Get_Name() == "layer3_menu_item")
			Action_ChangeEditLayer(2);
		if (e.Source->Get_Name() == "layer4_menu_item")
			Action_ChangeEditLayer(3);
		if (e.Source->Get_Name() == "layer5_menu_item")
			Action_ChangeEditLayer(4);

		if (e.Source->Get_Name() == "layer1_visible_menu_item")
			Action_ToggleLayerVisibility(0);
		if (e.Source->Get_Name() == "layer2_visible_menu_item")
			Action_ToggleLayerVisibility(1);
		if (e.Source->Get_Name() == "layer3_visible_menu_item")
			Action_ToggleLayerVisibility(2);
		if (e.Source->Get_Name() == "layer4_visible_menu_item")
			Action_ToggleLayerVisibility(3);
		if (e.Source->Get_Name() == "layer5_visible_menu_item")
			Action_ToggleLayerVisibility(4);
		
		if (e.Source->Get_Name() == "flip_h_menu_item")
			Action_ToggleFlipHorizontal();
		if (e.Source->Get_Name() == "flip_v_menu_item")
			Action_ToggleFlipVertical();
			
		if (e.Source->Get_Name() == "tilemap_menu_item")
			Action_ChangeMode(EditorMode::Tilemap);			
		if (e.Source->Get_Name() == "collision_menu_item")
			Action_ChangeMode(EditorMode::Collision);	
		if (e.Source->Get_Name() == "object_menu_item")
			Action_ChangeMode(EditorMode::Objects);

		if (e.Source->Get_Name() == "select_menu_item")
			Action_ChangeTool(EditorTool::Select);	
		if (e.Source->Get_Name() == "paint_menu_item")
			Action_ChangeTool(EditorTool::Pencil);
		if (e.Source->Get_Name() == "fill_menu_item")
			Action_ChangeTool(EditorTool::Fill);
		if (e.Source->Get_Name() == "erase_menu_item")
			Action_ChangeTool(EditorTool::Erase);
		if (e.Source->Get_Name() == "circle_menu_item")
			Action_ChangeTool(EditorTool::Circle);

		if (e.Source->Get_Name() == "map_properties_menu_item")
			Show_Map_Properties();
		
		if (e.Source->Get_Name() == "pfx_editor_menu_item")
		{
			manager->Go(UIAction::Push(new UIScene_PfxEditor(), NULL));		
		}

		m_supress_actions_this_frame = true;

		break;

	case UIEventType::PropertyGrid_BeginEdit:
		{
			if (m_selected_objects.size() > 0)
				Store_Old_Object_Properties();
			else
				Store_Old_Map_Properties();
		}
		break;

	case UIEventType::PropertyGrid_Changed:
		{
			if (m_selected_objects.size() > 0)
				Replicate_Object_Properties();
			else
				Replicate_Map_Properties();
			Update_Object_Selection();
		}
		break;
		
	case UIEventType::PropertyGrid_ExternalClicked:
		{
			UIPropertyGridItem* item = reinterpret_cast<UIPropertyGridItem*>(e.Sub_Source);

			// Object changes.
			if (m_properties_object != NULL)
			{
				CVMLinkedSymbol* sym = reinterpret_cast<CVMLinkedSymbol*>(item->Meta_Data);

				std::string edit_type = sym->symbol->Get_Meta_Data<std::string>("EditType", "");
				UIPropertyGridItem* sub_node = NULL;

				if (edit_type == "Image")
				{
					m_manager->Go(UIAction::Push(new UIScene_ResourceBrowser(UIScene_ResourceBrowserMode::Image), new UIFadeInTransition()));	
				}
				else if (edit_type == "Audio")
				{
					m_manager->Go(UIAction::Push(new UIScene_ResourceBrowser(UIScene_ResourceBrowserMode::Audio), new UIFadeInTransition()));	
				}
				else if (edit_type == "Animation")
				{
					m_manager->Go(UIAction::Push(new UIScene_ResourceBrowser(UIScene_ResourceBrowserMode::Animation), new UIFadeInTransition()));	
				}
				else
				{
					DBG_ASSERT_STR(false, "Unknown edit type '%s'.", edit_type.c_str());
				}

				Store_Old_Object_Properties();

				m_resource_object = m_properties_object;
				m_resource_meta = sym;
				m_resource_path = item->Meta_Data_3;
			}
			// Map changes.
			else
			{
				Map* map = Game::Get()->Get_Map();
				MapFileHeaderBlock* block = &map->Get_Map_Header();

				if (item->Meta_Data == &block->Music_Path)
				{
					m_manager->Go(UIAction::Push(new UIScene_ResourceBrowser(UIScene_ResourceBrowserMode::Audio, "music_"), new UIFadeInTransition()));	
				}

				Store_Old_Map_Properties();

				m_resource_object = NULL;
				m_resource_meta = item->Meta_Data;
			}
		}
		break;

	case UIEventType::Dialog_Close:
		{
			UIScene_FileBrowser* browser = dynamic_cast<UIScene_FileBrowser*>(e.Scene);
			if (browser != NULL)
			{
				std::string path = browser->Get_Selected_Path();
				if (path != "")
				{
					GEditorLastSaveOpenFolder = Platform::Get()->Extract_Directory(path);

					if (m_saving == true)
					{
						Save(path);
					}
					else
					{
						Open(path);
					}
				}
			}
			
			UIScene_ResourceBrowser* res_browser = dynamic_cast<UIScene_ResourceBrowser*>(e.Scene);
			if (res_browser != NULL)
			{
				std::string path = res_browser->Get_Selected_Resource();
				if (path != "")
				{
					DBG_LOG("Assigning resource '%s'.", path.c_str());
					if (m_resource_object != NULL)
					{
						CVirtualMachine* vm = GameEngine::Get()->Get_VM();
						CVMContextLock lock = vm->Set_Context(m_resource_object->Get_Script_Context());

						DBG_LOG("Assigning to field '%s'.", m_resource_path.c_str());

						CVMValue val;
						val.string_value = path.c_str();
						vm->Set_Value(m_resource_object->Get_Script_Object(), m_resource_path, val);

						Replicate_Object_Properties();
						Update_Object_Selection();
					}
					else
					{
						// Apply map property.
						*reinterpret_cast<std::string*>(m_resource_meta) = path;

						Replicate_Map_Properties();
						Update_Object_Selection();
					}
				}
			}

			UIScene_ConfirmDialog* confirm = dynamic_cast<UIScene_ConfirmDialog*>(e.Scene);
			if (confirm != NULL)
			{
				if (m_new_confirm == true)
				{
					if (confirm->Get_Selected_Index() == 1)
					{
						New();
					}
					m_new_confirm = false;
				}
			}

			break;
		}
	}
}

bool UIScene_Editor::Can_Cut()
{
	return (m_mode != EditorMode::Objects && m_tool == EditorTool::Select && m_map_selection.Is_Selecting()) || 
		   (m_mode == EditorMode::Objects && m_selected_objects.size() > 0);
}

bool UIScene_Editor::Can_Copy()
{
	return (m_mode != EditorMode::Objects && m_tool == EditorTool::Select && m_map_selection.Is_Selecting()) || 
		   (m_mode == EditorMode::Objects && m_selected_objects.size() > 0);
}

bool UIScene_Editor::Can_Paste()
{
	return (m_mode != EditorMode::Objects && m_tool == EditorTool::Select && m_map_selection.Is_Selecting() && m_copy_map_tiles.size() > 0) || 
		   (m_mode == EditorMode::Objects && m_copy_object_states.size() > 0);
}

void UIScene_Editor::Cut()
{
	EditorManager* manager = Game::Get()->Get_Editor_Manager();
	Map* map = Game::Get()->Get_Map();

	if (m_mode == EditorMode::Objects)
	{
		// Store deleted object state.
		m_copy_object_states.clear();
		m_copy_object_states_origin_bb = m_selected_objects_union_rect;

		for (std::vector<ScriptedActor*>::iterator iter = m_selected_objects.begin(); iter != m_selected_objects.end(); iter++)
		{
			ScriptedActor* actor = *iter;

			std::string original_tags = actor->Get_Tag();
			std::string original_links = actor->Get_Link();

			actor->Set_Tag("");
			actor->Set_Link("");

			MapFileObjectBlock object_block;
			actor->Serialize(&object_block);

			actor->Set_Tag(original_tags);
			actor->Set_Link(original_links);

			m_copy_object_states.push_back(object_block);
		}

		// Delete objects.
		Action_Delete();
	}
	else
	{
		// Store tile state.
		m_copy_map_tiles.clear();
		m_copy_map_tile_selection = m_map_selection.Get_Selection();
		
		for (int y = 0; y < (int)m_copy_map_tile_selection.Height; y++)
		{
			for (int x = 0; x < (int)m_copy_map_tile_selection.Width; x++)
			{
				MapTile* tile = map->Get_Tile((int)m_copy_map_tile_selection.X + x, (int)m_copy_map_tile_selection.Y + y, m_edit_layer);
				m_copy_map_tiles.push_back(*tile);
			}
		}

		m_copy_map_tile_selection.X = 0;
		m_copy_map_tile_selection.Y = 0;

		// Erase tiles.
		NetPacket_C2S_ModifyTilemap action;
		action.Action				= ModifyTilemapAction::Eraser;
		action.Mode					= m_mode == EditorMode::Collision ? ModifyTilemapMode::Collision : ModifyTilemapMode::Tilemap;
		action.Layer_Index			= m_edit_layer;
		action.Tilemap_Selection	= m_map_selection.Get_Selection();
		action.Tileset_Selection	= m_tileset_selection.Get_Selection();
		action.Flip_Horizontal		= m_flip_horizontal;
		action.Flip_Vertical		= m_flip_vertical;
		action.Tile_Color			= m_color_palette[m_color_index];

		manager->Replicate_Client_Action(&action);
	}

	m_map_selection.Clear();
	Refresh_State();
}

void UIScene_Editor::Copy()
{
	Map* map = Game::Get()->Get_Map();

	if (m_mode == EditorMode::Objects)
	{
		// Store object state.
		m_copy_object_states.clear();
		m_copy_object_states_origin_bb = m_selected_objects_union_rect;

		for (std::vector<ScriptedActor*>::iterator iter = m_selected_objects.begin(); iter != m_selected_objects.end(); iter++)
		{
			ScriptedActor* actor = *iter;

			std::string original_tags = actor->Get_Tag();
			std::string original_links = actor->Get_Link();

			actor->Set_Tag("");
			actor->Set_Link("");

			MapFileObjectBlock object_block;
			actor->Serialize(&object_block);

			actor->Set_Tag(original_tags);
			actor->Set_Link(original_links);

			m_copy_object_states.push_back(object_block);
		}
	}
	else
	{
		// Store tile state.
		m_copy_map_tiles.clear();
		m_copy_map_tile_selection = m_map_selection.Get_Selection();
		
		for (int y = 0; y < (int)m_copy_map_tile_selection.Height; y++)
		{
			for (int x = 0; x < (int)m_copy_map_tile_selection.Width; x++)
			{
				MapTile* tile = map->Get_Tile((int)m_copy_map_tile_selection.X + x, (int)m_copy_map_tile_selection.Y + y, m_edit_layer);
				m_copy_map_tiles.push_back(*tile);
			}
		}

		m_copy_map_tile_selection.X = 0;
		m_copy_map_tile_selection.Y = 0;
	}

	m_map_selection.Clear();
	Refresh_State();
}

void UIScene_Editor::Paste()
{
	EditorManager* manager = Game::Get()->Get_Editor_Manager();

	if (m_mode == EditorMode::Objects)
	{
		// Get mouse position and paste to there.
		Vector2 pos = Input::Get()->Get_Mouse_State()->Get_Position();
		Vector2 world_pos = Game::Get()->Get_Camera(CameraID::Editor_Main)->Unproject(pos);
		Vector2 origin = world_pos - Vector2(m_copy_object_states_origin_bb.X, m_copy_object_states_origin_bb.Y);

		// Paste object state.
		NetPacket_C2S_EditorCreateObjects action;
		action.Origin_X = origin.X;
		action.Origin_Y = origin.Y;
		action.Origin_Layer	= m_edit_layer;
		action.Creator_ID = NetManager::Get()->Get_Primary_Local_Net_User()->Get_Net_ID();

		for (std::vector<MapFileObjectBlock>::iterator iter = m_copy_object_states.begin(); iter != m_copy_object_states.end(); iter++)
		{
			MapFileObjectBlock& state = *iter;
			
			EditorCreateObjectState ed_state;
			ed_state.Class_Name = state.Class_Name;
			ed_state.Serialized_Data = state.Serialized_Data;
			ed_state.Serialized_Data_Version = state.Serialized_Data_Version;

			action.States.push_back(ed_state);
		}

		manager->Replicate_Client_Action(&action);
	}
	else
	{
		Rect2D paste_rect = m_copy_map_tile_selection;
		Vector2 tile_pos = m_map_selection.Get_Cursor_Tile_Position(false);
		paste_rect.X += tile_pos.X;
		paste_rect.Y += tile_pos.Y;

		// Paste copied tiles.
		NetPacket_C2S_ModifyTilemap action;
		action.Action				= ModifyTilemapAction::Paste;
		action.Mode					= m_mode == EditorMode::Collision ? ModifyTilemapMode::Collision : ModifyTilemapMode::Tilemap;
		action.Layer_Index			= m_edit_layer;
		action.Tilemap_Selection	= paste_rect;

		for (int y = 0; y < m_copy_map_tile_selection.Height; y++)
		{
			for (int x = 0; x < m_copy_map_tile_selection.Width; x++)
			{
				MapTile& copy_tile = m_copy_map_tiles.at((y * (int)m_copy_map_tile_selection.Width) + x);

				ModifyTilemapTileState tile;
				tile.Collision			= copy_tile.Collision;
				tile.Collision_Frame	= copy_tile.CollisionFrame;
				tile.Frame				= copy_tile.Frame;
				tile.Tile_Color			= copy_tile.TileColor;
				tile.Flip_Horizontal	= copy_tile.FlipHorizontal;
				tile.Flip_Vertical		= copy_tile.FlipVertical;

				action.Tiles.push_back(tile);
			}
		}

		manager->Replicate_Client_Action(&action);
	}

	m_map_selection.Clear();
	Refresh_State();
}

void UIScene_Editor::Action_Delete()
{
	EditorManager* manager = Game::Get()->Get_Editor_Manager();

	DBG_LOG("Performing delete.");

	if (m_mode == EditorMode::Objects)
	{
		NetPacket_C2S_EditorDeleteObjects action;

		for (std::vector<ScriptedActor*>::iterator iter = m_selected_objects.begin(); iter != m_selected_objects.end(); iter++)
		{
			ScriptedActor* actor = *iter;
			actor->Set_Selected(false);

			EditorDeleteObjectID state;
			state.ID = EditorManager::Get()->Get_Editor_ID(actor);

			action.IDs.push_back(state);
		}

		manager->Replicate_Client_Action(&action);

		m_selected_objects.clear();
		Show_Properties(NULL);
	}
	else
	{
		// Erase tiles.
		NetPacket_C2S_ModifyTilemap action;
		action.Action				= ModifyTilemapAction::Eraser;
		action.Mode					= m_mode == EditorMode::Collision ? ModifyTilemapMode::Collision : ModifyTilemapMode::Tilemap;
		action.Layer_Index			= m_edit_layer;
		action.Tilemap_Selection	= m_map_selection.Get_Selection();
		action.Tileset_Selection	= m_tileset_selection.Get_Selection();
		action.Flip_Horizontal		= m_flip_horizontal;
		action.Flip_Vertical		= m_flip_vertical;
		action.Tile_Color			= m_color_palette[m_color_index];

		manager->Replicate_Client_Action(&action);
	}
}

void UIScene_Editor::Select_Objects(Rect2D rect)
{
	Game* game = Game::Get();
	GameScene* scene = game->Get_Game_Scene();

	// If ctrl is held do a union.
	if (!Input::Get()->Is_Down(InputBindings::Keyboard_LeftCtrl) &&
		!Input::Get()->Is_Down(InputBindings::Keyboard_RightCtrl))
	{
		for (std::vector<ScriptedActor*>::iterator iter = m_selected_objects.begin(); iter != m_selected_objects.end(); iter++)
		{
			(*iter)->Set_Selected(false);
		}
		m_selected_objects.clear();		
	}

	std::vector<Actor*> actors = scene->Get_Actors();
	for (std::vector<Actor*>::iterator iter = actors.begin(); iter != actors.end(); iter++)
	{
		ScriptedActor* actor = dynamic_cast<ScriptedActor*>(*iter);
		if (actor != NULL)
		{	
			if (actor->Get_Layer() == m_edit_layer)
			{		
				Rect2D bb = actor->Get_World_Bounding_Box();
			
				if (bb.Intersects(rect))
				{
					if (actor->Get_Script_Object().Get()->Get_Symbol()->symbol->Get_Meta_Data<bool>("Placeable", false) == true)
					{
						actor->Set_Selected(true);
						m_selected_objects.push_back(actor);
					}
				}
			}
		}
	}

	// Show properties.
	if (m_selected_objects.size() == 1)
	{
		Show_Properties(m_selected_objects.at(0));
	}
	else
	{
		Show_Properties(NULL);
	}

	Update_Object_Selection();
	Refresh_State();
}

void UIScene_Editor::Select_Specific_Objects(std::vector<ScriptedActor*> actors)
{
	for (std::vector<ScriptedActor*>::iterator iter = m_selected_objects.begin(); iter != m_selected_objects.end(); iter++)
	{
		(*iter)->Set_Selected(false);
	}
	m_selected_objects.clear();		

	for (std::vector<ScriptedActor*>::iterator iter = actors.begin(); iter != actors.end(); iter++)
	{
		ScriptedActor* actor = dynamic_cast<ScriptedActor*>(*iter);

		actor->Set_Selected(true);
		m_selected_objects.push_back(actor);
	}

	// Show properties.
	if (m_selected_objects.size() == 1)
	{
		Show_Properties(m_selected_objects.at(0));
	}
	else
	{
		Show_Properties(NULL);
	}

	Update_Object_Selection();
	Refresh_State();
}

void UIScene_Editor::Objects_Deleted()
{
	for (std::vector<ScriptedActor*>::iterator iter = m_selected_objects.begin(); iter != m_selected_objects.end(); iter++)
	{
		(*iter)->Set_Selected(false);
	}
	m_selected_objects.clear();	

	Show_Properties(NULL);

	Update_Object_Selection();
	Refresh_State();
}

void UIScene_Editor::Map_Properties_Changed()
{	
	Game*				game		= Game::Get();
	Map*				map			= game->Get_Map();
	Atlas*				tileset		= map->Get_Tileset()->Get();
	
	AtlasFrame* tileset_frame		= tileset->Get_Frame_By_Index(0);	
	Vector2 tileset_expanse			= Vector2(tileset_frame->GridOrigin.Width, tileset_frame->GridOrigin.Height);

	EditorCamera* editor_camera		= static_cast<EditorCamera*>(Game::Get()->Get_Camera(CameraID::Editor_Main));
	EditorCamera* tileset_camera	= static_cast<EditorCamera*>(Game::Get()->Get_Camera(CameraID::Editor_Tileset));

	m_map_selection = EditorTileSelection(editor_camera, Vector2(map->Get_Width(), map->Get_Height()));
	m_tileset_selection = EditorTileSelection(tileset_camera, Vector2(tileset_expanse.X, tileset_expanse.Y));

	m_map_selection.Clear();
	m_tileset_selection.Clear();
}

void UIScene_Editor::Update_Object_Selection()
{	
	if (m_selected_objects.size() > 0)
	{
		m_selected_objects_union_rect = m_selected_objects.at(0)->Get_World_Bounding_Box();
		for (std::vector<ScriptedActor*>::iterator iter = m_selected_objects.begin(); iter != m_selected_objects.end(); iter++)
		{
			m_selected_objects_union_rect = m_selected_objects_union_rect.Union((*iter)->Get_World_Bounding_Box());
		}
	}

	Refresh_State();
}

void UIScene_Editor::Replicate_Object_Properties()
{
	EditorManager* manager = Game::Get()->Get_Editor_Manager();

	ScriptedActor* actor = m_selected_objects.at(0);
	
	MapFileObjectBlock object_block;
	actor->Serialize(&object_block);

	NetPacket_C2S_EditorChangeObjectState action;
	action.ID = manager->Get_Editor_ID(actor);
	action.Class_Name = object_block.Class_Name;

	action.Serialized_Data = object_block.Serialized_Data;
	action.Serialized_Data_Version = object_block.Serialized_Data_Version;

	action.Old_Serialized_Data = m_old_object_properties_state.Serialized_Data;
	action.Old_Serialized_Data_Version = m_old_object_properties_state.Serialized_Data_Version;

	manager->Replicate_Client_Action(&action);

	{
		CVirtualMachine* vm = GameEngine::Get()->Get_VM();
		CVMContextLock lock = vm->Set_Context(actor->Get_Script_Context());

		actor->Get_Event_Listener()->On_Editor_Property_Change();
	}

	Store_Old_Object_Properties();
}

void UIScene_Editor::Store_Old_Object_Properties()
{	
	m_selected_objects.at(0)->Serialize(&m_old_object_properties_state);
}

void UIScene_Editor::Replicate_Map_Properties()
{
	EditorManager* manager = Game::Get()->Get_Editor_Manager();
	Map* map = Game::Get()->Get_Map();

	// Enforce some sanity checks on the map so we don't end up going out of memory because
	// someone typed too many zero's.
	MapFileHeaderBlock& block = map->Get_Map_Header();
	block.Width = Min(block.Width, 2000);
	block.Height = Min(block.Height, 2000);

	BinaryStream bs;
	map->Get_Map_Header().Encode(MapVersion::Current_Version, &bs);
	DataBuffer buffer(bs.Data(), bs.Length());

	NetPacket_C2S_EditorChangeMapState action;
	action.Map_Data = buffer;
	action.Old_Map_Data = m_old_map_properties;

	manager->Replicate_Client_Action(&action);

	Store_Old_Map_Properties();
}

void UIScene_Editor::Store_Old_Map_Properties()
{	
	BinaryStream bs;
	Game::Get()->Get_Map()->Get_Map_Header().Encode(MapVersion::Current_Version, &bs);
	m_old_map_properties = DataBuffer(bs.Data(), bs.Length());
}

void UIScene_Editor::Place_Object(CVMLinkedSymbol* symbol, Vector2 pos)
{
	EditorManager* manager = Game::Get()->Get_Editor_Manager();
	DBG_LOG("Placing Object '%s' at %f,%f", symbol->symbol->name, pos.X, pos.Y);

	// Paste object state.
	NetPacket_C2S_EditorCreateObjects action;
	action.Origin_X = pos.X;
	action.Origin_Y = pos.Y;
	action.Origin_Layer = m_edit_layer;
	action.Creator_ID = NetManager::Get()->Get_Primary_Local_Net_User()->Get_Net_ID();

	EditorCreateObjectState ed_state;
	ed_state.Class_Name = symbol->symbol->name;
	ed_state.Serialized_Data.Reserve(0, false);
	ed_state.Serialized_Data_Version = 0;

	action.States.push_back(ed_state);

	manager->Replicate_Client_Action(&action);
}

void UIScene_Editor::Begin_New()
{
	// Confirm new.
	m_manager->Go(UIAction::Push(new UIScene_ConfirmDialog("Are you sure you wish to create a new map?\n\nYou will loose any unsaved progress.", S("#menu_no"), S("#menu_yes"), false), new UIFadeInTransition()));	
	m_new_confirm = true;
}

void UIScene_Editor::Begin_Open()
{
	// Confirm open.
	//m_manager->Go(UIAction::Push(new UIScene_ConfirmDialog("Are you sure you wish to open a map?\n\nYou will loose any unsaved progress.", S("#menu_no"), S("#menu_yes"), false), new UIFadeInTransition()));	

	m_saving = false;
	m_manager->Go(UIAction::Push(new UIScene_FileBrowser(false, GEditorLastSaveOpenFolder, "zgmap"), new UIFadeInTransition()));	
}

void UIScene_Editor::Begin_Save()
{
	if (GEditorLastSaveOpenFile != "")
	{
		Save(GEditorLastSaveOpenFile);
	}
	else
	{
		Begin_SaveAs();
	}
}

void UIScene_Editor::Begin_SaveAs()
{
	GEditorLastSaveOpenFile = "";
	m_saving = true;
	m_manager->Go(UIAction::Push(new UIScene_FileBrowser(true, GEditorLastSaveOpenFolder, "zgmap"), new UIFadeInTransition()));	
}

void UIScene_Editor::New()
{
	GEditorLastSaveOpenFile = "";

	EditorManager* manager = Game::Get()->Get_Editor_Manager();
	DBG_LOG("Resetting map.");

	NetPacket_C2S_EditorResetMap action;

	manager->Replicate_Client_Action(&action);
}

void UIScene_Editor::Save(std::string path)
{
	DBG_LOG("Saving '%s'.", path.c_str());
	
	Map* map = Game::Get()->Get_Map();
	EngineVirtualMachine* vm = GameEngine::Get()->Get_VM();
	GameScene* scene = Game::Get()->Get_Game_Scene();

	CVMLinkedSymbol* preview_center_marker_symbol = vm->Find_Class("Preview_Image_Center");
	std::vector<ScriptedActor*> actors = scene->Find_Derived_Actors(preview_center_marker_symbol);
	if (actors.size() > 0)
	{
		DBG_LOG("Updating map preview ...");

		static const float view_width = 320.0f;
		static const float view_height = 240.0f;

		ScriptedActor* actor = actors[0];

		// Update the maps preview.
		FrameTime* time = GameEngine::Get()->Get_Time();
		EditorCamera* camera = (EditorCamera*)Game::Get()->Get_Camera(CameraID::Editor_Main);
		Vector3 old_position = camera->Get_Position();
		Rect2D old_viewport = camera->Get_Viewport();
		RenderPipeline* pipeline = RenderPipeline::Get();
		Renderer* renderer = Renderer::Get();
		float gfx_width = (float)GfxDisplay::Get()->Get_Width();
		float gfx_height = (float)GfxDisplay::Get()->Get_Height();

		// Reset to "game" view mode.
		map->Set_Active_Layer_Tint(false, 0);
		for (int i = 0; i < map->Get_Depth(); i++)
		{
			map->Get_Layer(i)->Set_Visible(true);
			map->Get_Layer(i)->Set_Collision_Visible(false);
		}
		RenderPipeline::Get()->Set_Draw_Entity_Debug(false);
		CollisionManager::Get()->Set_Show_Collision(false);
		PathManager::Get()->Set_Show_Paths(false);
		camera->Set_Zoom(1.0f);
		scene->Set_Editor_Force_Hide(true);

		scene->Remove_Drawable(m_background);
		scene->Remove_Drawable(m_foreground);

		// Draw!
		Renderer::Get()->Clear_Buffer(true, true);
		pipeline->Flush_Batches();
		pipeline->Set_Render_Batch_Scissor_Test(false);
		pipeline->Set_Render_Batch_Global_Color(Color::White);
		pipeline->Set_Active_Camera(camera);
		camera->Set_Screen_Viewport(Rect2D(0.0f, 0.0f, gfx_width, gfx_height));
		camera->Set_Viewport(Rect2D(0.0f, 0.0f, gfx_width, gfx_height));
		renderer->Set_Depth_Test(false);
		camera->Set_Position(actor->Get_Position() - Vector3(view_width * 0.5f, view_height * 0.5f, 0.0f));

		pipeline->Draw_Game(*time);

		camera->Set_Position(old_position);
		camera->Set_Viewport(old_viewport);

		//Renderer::Get()->Flip(*time);

		// Reset to editor view-mode.
		scene->Add_Drawable(m_background);
		scene->Add_Drawable(m_foreground);
		scene->Set_Editor_Force_Hide(false);
		map->Set_Active_Layer_Tint(true, m_edit_layer);

		// Save the pixelmap!
		Pixelmap* pix = Renderer::Get()->Capture_Backbuffer();
		Pixelmap* final_pix = new Pixelmap((int)view_width, (int)view_height, PixelmapFormat::R8G8B8);
		final_pix->Paste(Vector2(0.0f, 0.0f), pix->Window(Rect2D(0.0f, 0.0f, (float)view_width, (float)view_height)), 0);
		map->Set_Preview_Pixmap(final_pix, true);
		SAFE_DELETE(pix);
	}

	// Save the map!
	std::string tmp_path = StringHelper::Format("%s.tmp", path.c_str());

	DBG_LOG("Serializing map state ...");
	MapSerializer* serializer = new MapSerializer(map);
	serializer->Serialize(tmp_path.c_str());
	SAFE_DELETE(serializer);

	Platform::Get()->Delete_File(path.c_str());
	if (Platform::Get()->Copy_File(tmp_path.c_str(), path.c_str()))
	{
		Platform::Get()->Delete_File(tmp_path.c_str());
	}

	GEditorLastSaveOpenFile = path;
}

void UIScene_Editor::Open(std::string path)
{
	DBG_LOG("Opening '%s'.", path.c_str());

	// Begin transfering map to server.
	Game::Get()->Get_Editor_Manager()->Transfer_And_Open_Map(path);

	GEditorLastSaveOpenFile = path;
}

void UIScene_Editor::Refresh_State()
{
	Game* game = Game::Get();
	Map* map = game->Get_Map();

	// Toolbar state.
	Find_Element<UIToolbarItem*>("select_toolbar_item")->Set_Selected		(m_tool == EditorTool::Select);
	Find_Element<UIToolbarItem*>("paint_toolbar_item")->Set_Selected		(m_tool == EditorTool::Pencil);
	Find_Element<UIToolbarItem*>("erase_toolbar_item")->Set_Selected		(m_tool == EditorTool::Erase);
	Find_Element<UIToolbarItem*>("fill_toolbar_item")->Set_Selected			(m_tool == EditorTool::Fill);
	Find_Element<UIToolbarItem*>("circle_toolbar_item")->Set_Selected		(m_tool == EditorTool::Circle);
	Find_Element<UIToolbarItem*>("select_toolbar_item")->Set_Enabled		(m_mode != EditorMode::Objects);
	Find_Element<UIToolbarItem*>("paint_toolbar_item")->Set_Enabled			(m_mode != EditorMode::Objects);
	Find_Element<UIToolbarItem*>("erase_toolbar_item")->Set_Enabled			(m_mode != EditorMode::Objects);
	Find_Element<UIToolbarItem*>("fill_toolbar_item")->Set_Enabled			(m_mode != EditorMode::Objects);

	Find_Element<UIToolbarItem*>("tilemap_toolbar_item")->Set_Selected		(m_mode == EditorMode::Tilemap);
	Find_Element<UIToolbarItem*>("collision_toolbar_item")->Set_Selected	(m_mode == EditorMode::Collision);
	Find_Element<UIToolbarItem*>("object_toolbar_item")->Set_Selected		(m_mode == EditorMode::Objects);

	Find_Element<UIToolbarItem*>("increase_toolbar_item")->Set_Enabled(m_tool == EditorTool::Circle);
	Find_Element<UIToolbarItem*>("value_toolbar_item")->Set_Value(StringHelper::To_String((int)m_circle_radius));
	Find_Element<UIToolbarItem*>("decrease_toolbar_item")->Set_Enabled(m_tool == EditorTool::Circle);
	
	Find_Element<UIToolbarItem*>("delete_toolbar_item")->Set_Enabled					(m_mode == EditorMode::Objects && m_selected_objects.size() > 0);

	Find_Element<UIToolbarItem*>("flip_h_toolbar_item")->Set_Selected		(m_flip_horizontal);
	Find_Element<UIToolbarItem*>("flip_v_toolbar_item")->Set_Selected		(m_flip_vertical);

	Find_Element<UIToolbarItem*>("layer1_toolbar_item")->Set_Selected		(m_edit_layer == 0);
	Find_Element<UIToolbarItem*>("layer2_toolbar_item")->Set_Selected		(m_edit_layer == 1);
	Find_Element<UIToolbarItem*>("layer3_toolbar_item")->Set_Selected		(m_edit_layer == 2);
	Find_Element<UIToolbarItem*>("layer4_toolbar_item")->Set_Selected		(m_edit_layer == 3);
	Find_Element<UIToolbarItem*>("layer5_toolbar_item")->Set_Selected		(m_edit_layer == 4);

	Find_Element<UIToolbarItem*>("palette_color1_item")->Set_Selected		(m_color_index == 0);
	Find_Element<UIToolbarItem*>("palette_color2_item")->Set_Selected		(m_color_index == 1);
	Find_Element<UIToolbarItem*>("palette_color3_item")->Set_Selected		(m_color_index == 2);
	Find_Element<UIToolbarItem*>("palette_color4_item")->Set_Selected		(m_color_index == 3);
	Find_Element<UIToolbarItem*>("palette_color5_item")->Set_Selected		(m_color_index == 4);
	Find_Element<UIToolbarItem*>("palette_color6_item")->Set_Selected		(m_color_index == 5);
	Find_Element<UIToolbarItem*>("palette_color1_item")->Set_Icon_Color		(m_color_palette[0]);
	Find_Element<UIToolbarItem*>("palette_color2_item")->Set_Icon_Color		(m_color_palette[1]);
	Find_Element<UIToolbarItem*>("palette_color3_item")->Set_Icon_Color		(m_color_palette[2]);
	Find_Element<UIToolbarItem*>("palette_color4_item")->Set_Icon_Color		(m_color_palette[3]);
	Find_Element<UIToolbarItem*>("palette_color5_item")->Set_Icon_Color		(m_color_palette[4]);
	Find_Element<UIToolbarItem*>("palette_color6_item")->Set_Icon_Color		(m_color_palette[5]);

	// Menu state.
	Find_Element<UIMenuItem*>("select_menu_item")->Set_Selected				(m_tool == EditorTool::Select);
	Find_Element<UIMenuItem*>("paint_menu_item")->Set_Selected				(m_tool == EditorTool::Pencil);
	Find_Element<UIMenuItem*>("erase_menu_item")->Set_Selected				(m_tool == EditorTool::Erase);
	Find_Element<UIMenuItem*>("fill_menu_item")->Set_Selected				(m_tool == EditorTool::Fill);
	Find_Element<UIMenuItem*>("circle_menu_item")->Set_Selected				(m_tool == EditorTool::Circle);
	Find_Element<UIMenuItem*>("select_menu_item")->Set_Enabled				(m_mode != EditorMode::Objects);
	Find_Element<UIMenuItem*>("paint_menu_item")->Set_Enabled				(m_mode != EditorMode::Objects);
	Find_Element<UIMenuItem*>("erase_menu_item")->Set_Enabled				(m_mode != EditorMode::Objects);
	Find_Element<UIMenuItem*>("fill_menu_item")->Set_Enabled				(m_mode != EditorMode::Objects);
	
	Find_Element<UIMenuItem*>("show_collision_menu_item")->Set_Selected		(CollisionManager::Get()->Get_Show_Collision());
	Find_Element<UIMenuItem*>("show_lighting_menu_item")->Set_Selected		(m_show_lighting);
	Find_Element<UIMenuItem*>("show_path_menu_item")->Set_Selected			(PathManager::Get()->Get_Show_Paths());
	Find_Element<UIMenuItem*>("show_randgen_menu_item")->Set_Selected		(m_foreground->Get_Draw_RandGen_Template());
	Find_Element<UIMenuItem*>("show_grid_menu_item")->Set_Selected(m_foreground->Get_Draw_Grid_Template());
	
	Find_Element<UIMenuItem*>("delete_menu_item")->Set_Enabled				(m_mode == EditorMode::Objects && m_selected_objects.size() > 0);

	Find_Element<UIMenuItem*>("tilemap_menu_item")->Set_Selected			(m_mode == EditorMode::Tilemap);
	Find_Element<UIMenuItem*>("collision_menu_item")->Set_Selected			(m_mode == EditorMode::Collision);
	Find_Element<UIMenuItem*>("object_menu_item")->Set_Selected				(m_mode == EditorMode::Objects);
	
	Find_Element<UIMenuItem*>("undo_menu_item")->Set_Enabled				(EditorManager::Get()->Get_Undo_Stack_Size() > 0);
	Find_Element<UIMenuItem*>("redo_menu_item")->Set_Enabled				(EditorManager::Get()->Get_Redo_Stack_Size() > 0);
	
	Find_Element<UIMenuItem*>("cut_menu_item")->Set_Enabled					(Can_Cut());
	Find_Element<UIMenuItem*>("copy_menu_item")->Set_Enabled				(Can_Copy());
	Find_Element<UIMenuItem*>("paste_menu_item")->Set_Enabled				(Can_Paste());

	Find_Element<UIMenuItem*>("flip_h_menu_item")->Set_Selected				(m_flip_horizontal);
	Find_Element<UIMenuItem*>("flip_v_menu_item")->Set_Selected				(m_flip_vertical);

	Find_Element<UIMenuItem*>("layer1_menu_item")->Set_Selected				(m_edit_layer == 0);
	Find_Element<UIMenuItem*>("layer2_menu_item")->Set_Selected				(m_edit_layer == 1);
	Find_Element<UIMenuItem*>("layer3_menu_item")->Set_Selected				(m_edit_layer == 2);
	Find_Element<UIMenuItem*>("layer4_menu_item")->Set_Selected				(m_edit_layer == 3);
	Find_Element<UIMenuItem*>("layer5_menu_item")->Set_Selected				(m_edit_layer == 4);
	
	Find_Element<UIMenuItem*>("layer1_visible_menu_item")->Set_Selected		(m_layer_visibility[0] == true);
	Find_Element<UIMenuItem*>("layer2_visible_menu_item")->Set_Selected		(m_layer_visibility[1] == true);
	Find_Element<UIMenuItem*>("layer3_visible_menu_item")->Set_Selected		(m_layer_visibility[2] == true);
	Find_Element<UIMenuItem*>("layer4_visible_menu_item")->Set_Selected		(m_layer_visibility[3] == true);
	Find_Element<UIMenuItem*>("layer5_visible_menu_item")->Set_Selected		(m_layer_visibility[4] == true);

	// Hide collision options if not in collision mode.
	UISplitContainerItem* split_item = Find_Element<UISplitContainerItem*>("collision_container_item");
	split_item->Set_Collapsed(m_mode != EditorMode::Collision);
	split_item->Get_Parent()->Refresh();
	
	split_item = Find_Element<UISplitContainerItem*>("properties_panel_container_item");
	split_item->Set_Collapsed(m_mode != EditorMode::Objects);
	split_item->Get_Parent()->Refresh();
	
	split_item = Find_Element<UISplitContainerItem*>("properties_flip_toolbar_container_item");
	split_item->Set_Collapsed(m_mode == EditorMode::Objects);
	split_item->Get_Parent()->Refresh();

	UIElement* panel = Find_Element<UIElement*>("tileset_panel");
	panel->Set_Visible(m_mode != EditorMode::Objects);
	panel->Set_Enabled(m_mode != EditorMode::Objects);
	
	panel = Find_Element<UIElement*>("object_list");
	panel->Set_Visible(m_mode == EditorMode::Objects);
	panel->Set_Enabled(m_mode == EditorMode::Objects);

	// Hide non-visible map layers.
	DBG_ASSERT(map->Get_Depth() == 5);
	for (int i = 0; i < map->Get_Depth(); i++)
	{
		map->Get_Layer(i)->Set_Visible(m_layer_visibility[i]);
		map->Get_Layer(i)->Set_Collision_Visible(m_mode == EditorMode::Collision);
	}

	map->Set_Active_Layer_Tint(true, m_edit_layer);

	RenderPipeline::Get()->Set_Draw_Entity_Debug(m_mode == EditorMode::Objects);
	RenderPipeline::Get()->Set_Debug_Layer(m_edit_layer);	
	
	if (m_show_lighting)
	{
		RenderPipeline::Get()->Set_Ambient_Lighting(Color::Black.To_Vector4());
	}
	else
	{
		RenderPipeline::Get()->Set_Ambient_Lighting(Vector4(0.5f, 0.5f, 0.5f, 1.0f));
	}
}

void UIScene_Editor::Action_ResetCamera()
{
	EditorCamera* cam = static_cast<EditorCamera*>(Game::Get()->Get_Camera(CameraID::Editor_Main));
	cam->Set_Position(Vector3(0, 0, 0));
	cam->Set_Zoom(1.0f);
	
	cam = static_cast<EditorCamera*>(Game::Get()->Get_Camera(CameraID::Editor_Tileset));
	cam->Set_Position(Vector3(0, 0, 0));
	cam->Set_Zoom(1.0f);
}

void UIScene_Editor::Action_ChangeMode(EditorMode::Type type)
{
	m_mode = type;
	m_map_selection.Clear();
	Refresh_State();
}

void UIScene_Editor::Action_ChangeTool(EditorTool::Type type)
{
	m_tool = type;
	m_map_selection.Clear();
	Refresh_State();
}

void UIScene_Editor::Action_ChangeEditLayer(int layer)
{
	m_edit_layer = layer;
	Refresh_State();
}

void UIScene_Editor::Action_ToggleLayerVisibility(int layer)
{
	m_layer_visibility[layer] = !m_layer_visibility[layer];
	Refresh_State();
}

void UIScene_Editor::Action_ToggleFlipVertical()
{
	m_flip_vertical = !m_flip_vertical;
	Refresh_State();
}

void UIScene_Editor::Action_ToggleFlipHorizontal()
{
	m_flip_horizontal = !m_flip_horizontal;
	Refresh_State();
}

void UIScene_Editor::Action_SelectColorPalette(int index)
{
	m_color_index = index;
	Refresh_State();
}

void UIScene_Editor::Action_ToggleCollision()
{
	CollisionManager::Get()->Set_Show_Collision(!CollisionManager::Get()->Get_Show_Collision());
	Refresh_State();
}

void UIScene_Editor::Action_ToggleLighting()
{
	m_show_lighting = !m_show_lighting;
	Refresh_State();
}

void UIScene_Editor::Action_ToggleRandGen()
{
	m_foreground->Set_Draw_RandGen_Template(!m_foreground->Get_Draw_RandGen_Template());
	Refresh_State();
}

void UIScene_Editor::Action_ToggleGrid()
{
	m_foreground->Set_Draw_Grid_Template(!m_foreground->Get_Draw_Grid_Template());
	Refresh_State();
}

void UIScene_Editor::Action_TogglePaths()
{
	PathManager::Get()->Set_Show_Paths(!PathManager::Get()->Get_Show_Paths());
	Refresh_State();
}

void UIScene_Editor::Update_Mode_Tilemap()
{
	EditorManager* manager = Game::Get()->Get_Editor_Manager();

	switch (m_tool)
	{
	case EditorTool::Select:
		if (m_map_selection.Update(true))
		{
			Refresh_State();
		}
		break;
	case EditorTool::Pencil:
		if (m_map_selection.Update(false))
		{
			NetPacket_C2S_ModifyTilemap action;
			action.Action				= ModifyTilemapAction::Pencil;
			action.Mode					= ModifyTilemapMode::Tilemap;
			action.Layer_Index			= m_edit_layer;
			action.Tilemap_Selection	= m_map_selection.Get_Selection();
			action.Tileset_Selection	= m_tileset_selection.Get_Selection();			
			action.Flip_Horizontal		= m_flip_horizontal;
			action.Flip_Vertical		= m_flip_vertical;
			action.Tile_Color			= m_color_palette[m_color_index];

			manager->Replicate_Client_Action(&action);
		}
		break;
	case EditorTool::Erase:
		if (m_map_selection.Update(false))
		{
			NetPacket_C2S_ModifyTilemap action;
			action.Action				= ModifyTilemapAction::Eraser;
			action.Mode					= ModifyTilemapMode::Tilemap;
			action.Layer_Index			= m_edit_layer;
			action.Tilemap_Selection	= m_map_selection.Get_Selection();
			action.Tileset_Selection	= m_tileset_selection.Get_Selection();
			action.Flip_Horizontal		= m_flip_horizontal;
			action.Flip_Vertical		= m_flip_vertical;
			action.Tile_Color			= m_color_palette[m_color_index];

			manager->Replicate_Client_Action(&action);
		}
		break;
	case EditorTool::Fill:
		if (m_map_selection.Update(false, false))
		{
			NetPacket_C2S_ModifyTilemap action;
			action.Action				= ModifyTilemapAction::Flood;
			action.Mode					= ModifyTilemapMode::Tilemap;
			action.Layer_Index			= m_edit_layer;
			action.Tilemap_Selection	= m_map_selection.Get_Selection();
			action.Tileset_Selection	= m_tileset_selection.Get_Selection();			
			action.Flip_Horizontal		= m_flip_horizontal;
			action.Flip_Vertical		= m_flip_vertical;
			action.Tile_Color			= m_color_palette[m_color_index];

			manager->Replicate_Client_Action(&action);
		}
		break;
	case EditorTool::Circle:
		if (m_map_selection.Update(false, false, false, true))
		{
			NetPacket_C2S_ModifyTilemap action;
			action.Action = ModifyTilemapAction::Circle;
			action.Mode = ModifyTilemapMode::Tilemap;
			action.Layer_Index = m_edit_layer;
			action.Tilemap_Selection = m_map_selection.Get_Selection();
			action.Tileset_Selection = m_tileset_selection.Get_Selection();
			action.Flip_Horizontal = m_flip_horizontal;
			action.Flip_Vertical = m_flip_vertical;
			action.Tile_Color = m_color_palette[m_color_index];
			action.Radius = m_circle_radius;

			manager->Replicate_Client_Action(&action);
		}
		break;
	}

	m_tileset_selection.Update(true);
}

void UIScene_Editor::Update_Mode_Collision()
{
	EditorManager* manager = Game::Get()->Get_Editor_Manager();

	switch (m_tool)
	{
	case EditorTool::Select:
		if (m_map_selection.Update(true))
		{
			Refresh_State();
		}
		break;
	case EditorTool::Pencil:
		if (m_map_selection.Update(false))
		{
			NetPacket_C2S_ModifyTilemap action;
			action.Action				= ModifyTilemapAction::Pencil;
			action.Mode					= ModifyTilemapMode::Collision;
			action.Collision			= m_collision_mode;
			action.Layer_Index			= m_edit_layer;
			action.Tilemap_Selection	= m_map_selection.Get_Selection();
			action.Tileset_Selection	= m_tileset_selection.Get_Selection();			
			action.Flip_Horizontal		= m_flip_horizontal;
			action.Flip_Vertical		= m_flip_vertical;
			action.Tile_Color			= m_color_palette[m_color_index];

			manager->Replicate_Client_Action(&action);
		}
		break;
	case EditorTool::Erase:
		if (m_map_selection.Update(false))
		{
			NetPacket_C2S_ModifyTilemap action;
			action.Action				= ModifyTilemapAction::Eraser;
			action.Mode					= ModifyTilemapMode::Collision;
			action.Collision			= m_collision_mode;
			action.Layer_Index			= m_edit_layer;
			action.Tilemap_Selection	= m_map_selection.Get_Selection();
			action.Tileset_Selection	= m_tileset_selection.Get_Selection();
			action.Flip_Horizontal		= m_flip_horizontal;
			action.Flip_Vertical		= m_flip_vertical;
			action.Tile_Color			= m_color_palette[m_color_index];

			manager->Replicate_Client_Action(&action);
		}
		break;
	case EditorTool::Fill:
		if (m_map_selection.Update(false, false))
		{
			NetPacket_C2S_ModifyTilemap action;
			action.Action				= ModifyTilemapAction::Flood;
			action.Mode					= ModifyTilemapMode::Collision;
			action.Collision			= m_collision_mode;
			action.Layer_Index			= m_edit_layer;
			action.Tilemap_Selection	= m_map_selection.Get_Selection();
			action.Tileset_Selection	= m_tileset_selection.Get_Selection();			
			action.Flip_Horizontal		= m_flip_horizontal;
			action.Flip_Vertical		= m_flip_vertical;
			action.Tile_Color			= m_color_palette[m_color_index];

			manager->Replicate_Client_Action(&action);
		}
		break;
	case EditorTool::Circle:
		if (m_map_selection.Update(false))
		{
			NetPacket_C2S_ModifyTilemap action;
			action.Action = ModifyTilemapAction::Circle;
			action.Mode = ModifyTilemapMode::Collision;
			action.Collision = m_collision_mode;
			action.Layer_Index = m_edit_layer;
			action.Tilemap_Selection = m_map_selection.Get_Selection();
			action.Tileset_Selection = m_tileset_selection.Get_Selection();
			action.Flip_Horizontal = m_flip_horizontal;
			action.Flip_Vertical = m_flip_vertical;
			action.Tile_Color = m_color_palette[m_color_index];		
			action.Radius = m_circle_radius;

			manager->Replicate_Client_Action(&action);
		}
		break;
	}

	m_tileset_selection.Update(true);
}

void UIScene_Editor::Replicate_Object_Position_Changes()
{			
	EditorManager* manager = Game::Get()->Get_Editor_Manager();

	NetPacket_C2S_EditorRepositionObjects action;

	for (std::vector<ScriptedActor*>::iterator iter = m_selected_objects.begin(); iter != m_selected_objects.end(); iter++)
	{
		ScriptedActor* actor = *iter;

		EditorRepositionObjectsState state;
		state.Index = EditorManager::Get()->Get_Editor_ID(actor);
		state.X = actor->Get_Position().X;
		state.Y = actor->Get_Position().Y;
		state.Angle = actor->Get_Rotation().Z;
		state.Bounding_Box = actor->Get_Bounding_Box();

		{
			CVirtualMachine* vm = GameEngine::Get()->Get_VM();
			CVMContextLock lock = vm->Set_Context(actor->Get_Script_Context());

			actor->Get_Event_Listener()->On_Editor_Property_Change();
		}

		action.States.push_back(state);
	}

	for (std::vector<EditorRepositionObjectsState>::iterator iter = m_old_selected_object_states.begin(); iter != m_old_selected_object_states.end(); iter++)
	{
		EditorRepositionObjectsState& state = *iter;
		action.Old_States.push_back(state);
	}

	manager->Replicate_Client_Action(&action);
}

void UIScene_Editor::Store_Old_Object_Positions()
{
	EditorManager* manager = Game::Get()->Get_Editor_Manager();

	m_old_selected_object_states.clear();

	for (std::vector<ScriptedActor*>::iterator iter = m_selected_objects.begin(); iter != m_selected_objects.end(); iter++)
	{
		ScriptedActor* actor = *iter;

		EditorRepositionObjectsState state;
		state.Index = EditorManager::Get()->Get_Editor_ID(actor);
		state.X = actor->Get_Position().X;
		state.Y = actor->Get_Position().Y;
		state.Angle = actor->Get_Rotation().Z;
		state.Bounding_Box = actor->Get_Bounding_Box();

		m_old_selected_object_states.push_back(state);
	}
}

void UIScene_Editor::Update_Mode_Objects()
{
	EditorManager* manager = Game::Get()->Get_Editor_Manager();	
	Input* input = Input::Get();
	MouseState* mouse = input->Get_Mouse_State();
	Vector2 mouse_pos = mouse->Get_Position();
	Vector2 world_mouse_pos = Game::Get()->Get_Camera(CameraID::Editor_Main)->Unproject(mouse_pos);

	bool snap = input->Is_Down(InputBindings::Keyboard_LeftShift) ||
				input->Is_Down(InputBindings::Keyboard_RightShift);
	bool link = input->Is_Down(InputBindings::Keyboard_LeftCtrl) ||
				input->Is_Down(InputBindings::Keyboard_RightCtrl);

	bool update_selection = true;

	// Resizing objects.
	if (m_resizing_objects == true)
	{
		if (mouse->Is_Button_Down(InputBindings::Mouse_Left))
		{
			Vector2 diff = world_mouse_pos - m_resize_start;			
			Vector2 increment = diff;

			if (snap)
			{
				m_snap_offset = m_snap_offset + diff;
				increment = Vector2((int)m_snap_offset.X / (int)snap_size, (int)m_snap_offset.Y / (int)snap_size);
				m_snap_offset = m_snap_offset - Vector2(increment.X * (float)snap_size, increment.Y * (float)snap_size);
				increment = Vector2((float)increment.X * (float)snap_size, (float)increment.Y * (float)snap_size);
			}
			
			Rect2D new_rect;
			new_rect.X = m_selected_objects_union_rect.X + (increment.X * m_resize_anchor.X);
			new_rect.Y = m_selected_objects_union_rect.Y + (increment.Y * m_resize_anchor.Y);
			new_rect.Width = m_selected_objects_union_rect.Width   + (increment.X * m_resize_anchor.Width);
			new_rect.Height = m_selected_objects_union_rect.Height + (increment.Y * m_resize_anchor.Height);
		
			for (std::vector<ScriptedActor*>::iterator iter = m_selected_objects.begin(); iter != m_selected_objects.end(); iter++)
			{
				ScriptedActor* actor = *iter;
			
				Vector3 position = actor->Get_Position();
				position.X += (increment.X * m_resize_anchor.X);
				position.Y += (increment.Y * m_resize_anchor.Y);

				Rect2D bb = actor->Get_Bounding_Box();
				bb.Width += (increment.X * m_resize_anchor.Width);
				bb.Height += (increment.Y * m_resize_anchor.Height);

				if (snap)
				{
					bb.Width = (float)((int)((bb.Width / (float)snap_size)) * snap_size);
					bb.Height = (float)((int)((bb.Height / (float)snap_size)) * snap_size);
					position.X = (float)((int)((position.X / (float)snap_size)) * snap_size);
					position.Y = (float)((int)((position.Y / (float)snap_size)) * snap_size);
				}

				actor->Set_Position(position);
				actor->Set_Bounding_Box(bb);
			}

			Update_Object_Selection();

			m_resize_start = world_mouse_pos;
		}
		else
		{
			Replicate_Object_Position_Changes();
			m_resizing_objects = false;
		}
		
		update_selection = false;
	}
	
	// Rotating objects.
	else if (m_rotating_objects == true)
	{
		if (mouse->Is_Button_Down(InputBindings::Mouse_Left))
		{
			Vector2 center_point = Vector2(m_selected_objects_union_rect.X + (m_selected_objects_union_rect.Width * 0.5f), m_selected_objects_union_rect.Y + (m_selected_objects_union_rect.Height * 0.5f));

			float angle = atan2(world_mouse_pos.Y - center_point.Y, world_mouse_pos.X - center_point.X) - HALFPI;
		
			for (std::vector<ScriptedActor*>::iterator iter = m_selected_objects.begin(); iter != m_selected_objects.end(); iter++)
			{
				ScriptedActor* actor = *iter;
				
				Vector3 rot = actor->Get_Rotation();
				rot.Z = RadToDeg(angle);

				if (snap == true)
				{
					rot.Z = (float)(((int)(rot.Z / (float)rot_grid_size)) * rot_grid_size);
				}

				actor->Set_Rotation(rot);
			}
		}
		else
		{
			Replicate_Object_Position_Changes();
			m_rotating_objects = false;
		}
		
		update_selection = false;
	}

	// Dragging existing selection.
	else if (m_dragging_objects == true)
	{
		if (mouse->Is_Button_Down(InputBindings::Mouse_Left))
		{
			Vector2 diff = world_mouse_pos - m_drag_start;			
			if (diff.X != 0.0f || diff.Y != 0.0f)
			{
				Vector2 increment = diff;

				if (snap)
				{
					m_snap_offset = m_snap_offset + diff;
					increment = Vector2((int)m_snap_offset.X / (int)snap_size, (int)m_snap_offset.Y / (int)snap_size);
					m_snap_offset = m_snap_offset - Vector2(increment.X * (float)snap_size, increment.Y * (float)snap_size);
					increment = Vector2((float)increment.X * (float)snap_size, (float)increment.Y * (float)snap_size);
				}
			
				for (std::vector<ScriptedActor*>::iterator iter = m_selected_objects.begin(); iter != m_selected_objects.end(); iter++)
				{
					ScriptedActor* actor = *iter;
					Vector3 new_pos = actor->Get_Position();
					new_pos.X += increment.X;
					new_pos.Y += increment.Y;

					if (snap == true)
					{
						new_pos.X = (float)((int)(new_pos.X / (float)snap_size) * snap_size);
						new_pos.Y = (float)((int)(new_pos.Y / (float)snap_size) * snap_size);
					}

					actor->Set_Position(new_pos);
				}

				Update_Object_Selection();
			}
			m_drag_start = world_mouse_pos;
		}
		else
		{
			Replicate_Object_Position_Changes();
			m_dragging_objects = false;
			m_snap_offset = Vector2(0, 0);
		}

		update_selection = false;
	}

	// Selecting objects.
	else
	{
		if (m_selected_objects.size() > 0 && mouse->Is_Button_Down(InputBindings::Mouse_Left) && !mouse->Was_Button_Down(InputBindings::Mouse_Left))
		{
			float half_grab_size = (float)(grab_block_size / 2);
			float full_grab_size = (float)grab_block_size;

			Rect2D tl_grab = Rect2D(m_selected_objects_union_rect.X - half_grab_size, m_selected_objects_union_rect.Y - half_grab_size, full_grab_size, full_grab_size);
			Rect2D tr_grab = Rect2D(m_selected_objects_union_rect.X + m_selected_objects_union_rect.Width - half_grab_size, m_selected_objects_union_rect.Y - half_grab_size, full_grab_size, full_grab_size);
			Rect2D bl_grab = Rect2D(m_selected_objects_union_rect.X - half_grab_size, m_selected_objects_union_rect.Y + m_selected_objects_union_rect.Height - half_grab_size, full_grab_size, full_grab_size);
			Rect2D br_grab = Rect2D(m_selected_objects_union_rect.X + m_selected_objects_union_rect.Width - half_grab_size, m_selected_objects_union_rect.Y + m_selected_objects_union_rect.Height - half_grab_size, full_grab_size, full_grab_size);
			Rect2D rot_grab = Rect2D(m_selected_objects_union_rect.X + (m_selected_objects_union_rect.Width * 0.5f) - half_grab_size, m_selected_objects_union_rect.Y + m_selected_objects_union_rect.Height - half_grab_size, full_grab_size, full_grab_size);
			
			if (tl_grab.Intersects(world_mouse_pos))
			{
				m_resizing_objects = true;
				m_resize_start = world_mouse_pos;
				m_resize_anchor = Rect2D(1.0f, 1.0f, -1.0f, -1.0f);
				m_snap_offset = Vector2(0, 0);
				update_selection = false;
			}
			else if (tr_grab.Intersects(world_mouse_pos))
			{
				m_resizing_objects = true;
				m_resize_start = world_mouse_pos;
				m_resize_anchor = Rect2D(0.0f, 1.0f, 1.0f, -1.0f);
				m_snap_offset = Vector2(0, 0);
				update_selection = false;
			}
			else if (bl_grab.Intersects(world_mouse_pos))
			{
				m_resizing_objects = true;
				m_resize_start = world_mouse_pos;
				m_resize_anchor = Rect2D(1.0f, 0.0f, -1.0f, 1.0f);
				m_snap_offset = Vector2(0, 0);
				update_selection = false;
			}
			else if (br_grab.Intersects(world_mouse_pos))
			{
				m_resizing_objects = true;
				m_resize_start = world_mouse_pos;
				m_resize_anchor = Rect2D(0.0f, 0.0f, 1.0f, 1.0f);
				m_snap_offset = Vector2(0, 0);
				update_selection = false;
			}
			else if (rot_grab.Intersects(world_mouse_pos))
			{
				m_rotating_objects = true;
				m_rotate_start = world_mouse_pos;
				update_selection = false;
			}
			else if (m_selected_objects_union_rect.Intersects(world_mouse_pos))
			{
				m_dragging_objects = true;
				m_drag_start = world_mouse_pos;
				update_selection = false;
			}

			Store_Old_Object_Positions();
		}
	}

	if (link)
	{
		if (mouse->Was_Button_Clicked(InputBindings::Mouse_Left))
		{
			Vector2 mouse_pos = m_map_selection.Get_Cursor_Tile_Position(true);			

			std::vector<Actor*> actors = Get_Objects_In_Rect(Rect2D(mouse_pos.X, mouse_pos.Y, 1.0f, 1.0f));
			if (actors.size() > 0)
			{
				if (m_link_object_source != NULL)
				{
					ScriptedActor* from = (ScriptedActor*)m_link_object_source;
					ScriptedActor* to = (ScriptedActor*)actors[0];

					std::vector<ScriptedActor*> original_actors = m_selected_objects;

					// Give the target a tag if it dosen't have one already.
					std::string auto_tag = StringHelper::Format("[%08x]", StringHelper::Hash(Platform::Get()->Generate_UUID().c_str()));
					if (to->Get_Tag() == "")
					{
						m_selected_objects.clear();
						m_selected_objects.push_back(to);
						to->Set_Tag(auto_tag);
						Replicate_Object_Properties();
					}

					// Either add or remove the tag from the link list.
					std::vector<std::string> original_links;
					std::vector<std::string> result_links;
					StringHelper::Split(from->Get_Link().c_str(), ',', original_links);

					bool bAdd = true;

					for (std::vector<std::string>::iterator iter = original_links.begin(); iter != original_links.end(); iter++)
					{
						std::string src = (*iter).c_str();
						std::string dst = to->Get_Tag().c_str();
						if (src == dst)
						{
							bAdd = false;
						}
						else
						{
							result_links.push_back(src);
						}
					}

					if (bAdd)
					{
						result_links.push_back(to->Get_Tag());
					}

					std::string final_links = StringHelper::Join(result_links.begin(), result_links.end(), ",");

					m_selected_objects.clear();
					m_selected_objects.push_back(from);
					from->Set_Link(final_links);
					Replicate_Object_Properties();

					m_selected_objects = original_actors;
					Replicate_Object_Properties();

					m_link_object_source = NULL;
				}
				else
				{
					m_link_object_source = actors[0];
				}
			}
		}
	}
	else
	{
		m_link_object_source = NULL;
	}
	
	// Update object selection.
	if (update_selection && !link)
	{
		if (m_map_selection.Update(false, true, true))
		{
			Rect2D selection = m_map_selection.Get_Selection();
			Select_Objects(selection);
			
			if (m_selected_objects.size() == 0 && selection.Width < 3 && selection.Height < 3)
			{
				UITreeView* tree = Find_Element<UITreeView*>("object_list");
				UITreeViewItem* item = tree->Get_Selected_Item();
				if (item != NULL && item->Is_Selectable == true)
				{
					CVMLinkedSymbol* symbol = reinterpret_cast<CVMLinkedSymbol*>(item->Meta_Data);
					Place_Object(symbol, Vector2(selection.X, selection.Y));
				}
			}
		}
	}
}

std::vector<Actor*> UIScene_Editor::Get_Objects_In_Rect(Rect2D rect)
{
	std::vector<Actor*> result;

	Scene* scene = GameEngine::Get()->Get_Scene();

	std::vector<Actor*> actors = scene->Get_Actors();
	for (std::vector<Actor*>::iterator iter = actors.begin(); iter != actors.end(); iter++)
	{
		ScriptedActor* actor = dynamic_cast<ScriptedActor*>(*iter);
		if (actor != NULL)
		{
			if (actor->Get_Layer() == m_edit_layer)
			{
				Rect2D bb = actor->Get_World_Bounding_Box();

				if (bb.Intersects(rect))
				{
					if (actor->Get_Script_Object().Get()->Get_Symbol()->symbol->Get_Meta_Data<bool>("Placeable", false) == true)
					{
						result.push_back(actor);
					}
				}
			}
		}
	}

	return result;
}