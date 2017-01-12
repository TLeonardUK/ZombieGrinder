// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_PFX_EDITOR_
#define _GAME_UI_SCENES_UISCENE_PFX_EDITOR_

#include "Engine/UI/UIScene.h"

#include "Game/UI/Scenes/Editor/EditorTileSelection.h"
#include "Engine/Renderer/Text/Font.h"
#include "Engine/Renderer/Text/FontRenderer.h"

#include "Engine/Particles/ParticleManager.h"

#include "Engine/Scene/Map/Map.h"

#include "Generic/Types/IntVector3.h"
#include "Generic/Types/AABB.h"
#include "Generic/Types/Color.h"

class UIToolbarItem;
class EditorCamera;
class EditorBackground;
class EditorForeground;
struct AtlasFrame;
class ParticleFX;
class UIPropertyGrid;
struct UIPropertyGridItem;

class UIScene_PfxEditor : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_PfxEditor, "UI");

private:	
	AtlasFrame*				m_background_frame;

	UIManager*				m_manager;
	FontHandle*				m_font;
	FontRenderer			m_font_renderer;
	
	ParticleFX*				m_fx;
	ParticleEffectHandle	m_effect;

	bool					m_needs_refresh;

	ParticleFX_Curve*		m_edit_curve;

	std::string				m_save_path;
	bool					m_saving;

	Vector3					m_effect_pos;
	float					m_effect_dir;

	bool					m_collision_show;

	std::string*			m_edit_resource_name;

	ParticleCollectorHandle	m_collector_handle;

	bool					m_show_lighting;

protected:
	void Add_Effect();
	void Add_Attractor();

	void Delete_Effect(ParticleFX_EmitterConfig* emitter);
	void Delete_Attractor(ParticleFX_AttractorConfig* attractor);
	void Add_Event(UIPropertyGrid* grid, UIPropertyGridItem* item, ParticleFX_EmitterConfig* emt);
	void Remove_Event(UIPropertyGrid* grid, UIPropertyGridItem* item, ParticleFX_Event* evt);
	void Add_Event_Node(int index, UIPropertyGrid* grid, UIPropertyGridItem* parent, ParticleFX_Event* evt);

	void Refresh_Curve();
	void Update_Curves();
	void Precalculate_Curve();
	void Update_Properties();

	void Reset_Camera();

	void New(const char* path = NULL);
	void Begin_Open();
	void Begin_Save();
	void Begin_SaveAs();

	void Save(std::string path);
	void Open(std::string path);

public:
	UIScene_PfxEditor();
	~UIScene_PfxEditor();

	UIScene* Get_Background(UIManager* manager);
	const char* Get_Name();
	bool Should_Render_Lower_Scenes();
	bool Should_Display_Cursor();
	bool Should_Display_Focus_Cursor();
	bool Is_Focusable();
	bool Should_Fade_Cursor();

	void Refresh_Properties();

	void Enter(UIManager* manager);
	void Exit(UIManager* manager);
	
	void Tick(const FrameTime& time, UIManager* manager, int scene_index);
	void Draw(const FrameTime& time, UIManager* manager, int scene_index);
	
	void Refresh_State();
	void Refresh(UIManager* manager);
	void Recieve_Event(UIManager* manager, UIEvent e);

};

#endif

