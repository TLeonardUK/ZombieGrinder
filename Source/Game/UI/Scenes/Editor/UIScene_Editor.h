// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_EDITOR_
#define _GAME_UI_SCENES_UISCENE_EDITOR_

#include "Engine/UI/UIScene.h"

#include "Game/UI/Scenes/Editor/EditorTileSelection.h"
#include "Game/Network/Packets/GamePackets.h"

#include "Engine/Renderer/Text/Font.h"
#include "Engine/Renderer/Text/FontRenderer.h"

#include "Game/Scene/Map/Blocks/MapFileObjectBlock.h"

#include "Engine/Scene/Map/Map.h"

#include "Generic/Types/IntVector3.h"
#include "Generic/Types/AABB.h"
#include "Generic/Types/Color.h"

class UIToolbarItem;
class UIPropertyGrid;
struct UIPropertyGridItem;
class EditorCamera;
class EditorBackground;
class EditorForeground;
struct AtlasFrame;
class ScriptedActor;
class UITreeView;
struct UITreeViewItem;

#define EDITOR_SELECTION_PRIMITIVE_PADDING 0.01f

struct EditorMode
{
	enum Type
	{
		Tilemap,
		Collision,
		Objects,

		COUNT
	};
};

struct EditorTool
{
	enum Type
	{
		Select,
		Pencil,
		Erase,
		Fill,
		Circle,

		COUNT
	};
};

class UIScene_Editor : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_Editor, "UI");

private:
	EditorBackground*			m_background;
	EditorForeground*			m_foreground;

	FontHandle*					m_font;
	FontRenderer				m_font_renderer;

	AtlasFrame*					m_background_frame;

	EditorMode::Type			m_mode;
	EditorTool::Type			m_tool;
	int							m_edit_layer;
	bool						m_layer_visibility[5];
	bool						m_flip_horizontal;
	bool						m_flip_vertical;
	int							m_color_index;
	Color						m_color_palette[6];

	EditorTileSelection			m_map_selection;
	EditorTileSelection			m_tileset_selection;

	MapTileCollisionType::Type	m_collision_mode;

	int							m_last_action_count;

	bool						m_supress_actions_this_frame;
	
	UIManager*					m_manager;

	bool						m_saving;

	int							m_circle_radius;

	Rect2D										m_copy_object_states_origin_bb;
	std::vector<MapFileObjectBlock>				m_copy_object_states;
	std::vector<ScriptedActor*>					m_selected_objects;
	std::vector<EditorRepositionObjectsState>	m_old_selected_object_states;
	Rect2D										m_selected_objects_union_rect;
	
	std::vector<MapTile>						m_copy_map_tiles;
	Rect2D										m_copy_map_tile_selection;

	bool						m_dragging_objects;
	Vector2						m_drag_start;
	Vector2						m_snap_offset;

	bool						m_resizing_objects;
	Vector2						m_resize_start;
	Rect2D						m_resize_anchor;

	bool						m_rotating_objects;
	Vector2						m_rotate_start;

	ScriptedActor*				m_properties_object;
		
	MapFileObjectBlock			m_old_object_properties_state;

	DataBuffer					m_old_map_properties;

	float						m_original_bgm_volume;

	ScriptedActor*				m_resource_object;
	void*						m_resource_meta;
	std::string					m_resource_path;

	bool						m_new_confirm;

	bool						m_show_lighting;

	Actor*						m_link_object_source;

protected:
	
	void Gather_Placeable_Objects(CVirtualMachine* vm, CVMLinkedSymbol* parent_symbol, std::vector<CVMLinkedSymbol*>& results);
	//void Build_Object_Tree(UITreeView* tree, CVirtualMachine* vm, CVMLinkedSymbol* parent_symbol, UITreeViewItem* parent_node);
	void Build_Object_Tree(UITreeView* tree, CVirtualMachine* vm, std::string path, UITreeViewItem* parent_node, std::vector<CVMLinkedSymbol*>& results);


	void Add_Script_Field(UIPropertyGrid* grid, UIPropertyGridItem* node, CVMLinkedSymbol* sym, CVMObjectHandle handle, std::string path, CVMObjectHandle base_handle);
	void Add_Script_Fields(UIPropertyGrid* grid, UIPropertyGridItem* node, CVMObjectHandle handle, std::string path, CVMObjectHandle base_handle);
	
	static void Set_Script_Field_Value(UIPropertyGridItem* node, void* value);
	static void Get_Script_Field_Value(UIPropertyGridItem* node, void* value);

	void Show_Properties(ScriptedActor* object);
	void Store_Old_Object_Properties();

	void Show_Map_Properties();

	bool Can_Cut();
	bool Can_Copy();
	bool Can_Paste();
	void Cut();
	void Copy();
	void Paste();
	
	void Begin_New();
	void Begin_Open();
	void Begin_Save();
	void Begin_SaveAs();

	void Select_Objects(Rect2D rect);
	void Update_Object_Selection();

	void Replicate_Object_Properties();

	void Replicate_Map_Properties();
	void Store_Old_Map_Properties();

	void Place_Object(CVMLinkedSymbol* symbol, Vector2 pos);

	std::vector<Actor*> Get_Objects_In_Rect(Rect2D rect);

	enum
	{
		grab_block_size = 9,
		snap_size		= 8,
		rot_grid_size	= 360 / 16,
	};

public:
	UIScene_Editor();
	~UIScene_Editor();

	const char* Get_Name();
	bool Should_Render_Lower_Scenes();
	bool Should_Display_Cursor();
	bool Should_Display_Focus_Cursor();
	bool Is_Focusable();
	bool Should_Fade_Cursor();

	void New();
	void Save(std::string path);
	void Open(std::string path);

	void Select_Specific_Objects(std::vector<ScriptedActor*> actors);
	void Objects_Deleted();
	void Map_Properties_Changed();

	std::string Get_Save_Path();

	void Enter(UIManager* manager);
	void Exit(UIManager* manager);
	
	void Tick(const FrameTime& time, UIManager* manager, int scene_index);
	void Draw(const FrameTime& time, UIManager* manager, int scene_index);
	
	void Draw_Selection(const FrameTime& time, RenderPipeline* pipeline);
	void Draw_Tileset(const FrameTime& time, UIManager* manager, int scene_index);
	void Draw_Object_Browser(const FrameTime& time, UIManager* manager, int scene_index);
	
	void Refresh(UIManager* manager);
	void Recieve_Event(UIManager* manager, UIEvent e);
	void Refresh_State();

	void Action_ResetCamera();
	void Action_ChangeMode(EditorMode::Type type);	
	void Action_ChangeTool(EditorTool::Type type);	
	void Action_ChangeEditLayer(int layer);
	void Action_ToggleLayerVisibility(int layer);
	void Action_ToggleFlipVertical();
	void Action_ToggleFlipHorizontal();
	void Action_SelectColorPalette(int index);
	void Action_ToggleCollision();
	void Action_ToggleLighting();
	void Action_TogglePaths();
	void Action_ToggleRandGen();
	void Action_ToggleGrid();
	void Action_Delete();

	void Store_Old_Object_Positions();
	void Replicate_Object_Position_Changes();

	void Update_Mode_Tilemap();
	void Update_Mode_Collision();
	void Update_Mode_Objects();

	EditorTileSelection& Get_Map_Selection();
	EditorTileSelection& Get_Tileset_Selection();

};

#endif

