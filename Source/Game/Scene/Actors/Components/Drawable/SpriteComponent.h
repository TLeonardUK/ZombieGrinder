// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_SCENE_ACTORS_COMPONENTS_DRAWBLE_SPRITECOMPONENT_
#define _ENGINE_SCENE_ACTORS_COMPONENTS_DRAWBLE_SPRITECOMPONENT_

#include "Engine/Renderer/Drawable.h"

#include "Game/Scene/Actors/Components/Drawable/DrawableComponent.h"
#include "Engine/Scene/Animation.h"

struct AtlasFrame;
struct AtlasAnimation;

#ifndef MASTER_BUILD
//#define OPT_SHOW_REPLICATED_DEBUG_STATE
#endif

#ifdef OPT_SHOW_REPLICATED_DEBUG_STATE

struct SpriteDebugState
{
	Vector3 Position;
	Vector3 Velocity;
};

#endif

class SpriteComponent : public DrawableComponent
{
	MEMORY_ALLOCATOR(SpriteComponent, "Scene");

private:
	Vector4			m_object_mask;

	Color			m_color;
	Color			m_color_scalar;
	AtlasFrame*		m_frame;
	Animation		m_animation;
	float			m_animation_speed;

	bool			m_sort_by_transparency;

	Rect2D			m_area;
	Vector2			m_offset;

	Animation		m_oneshot_anim;
	float			m_oneshot_anim_speed;
	bool			m_oneshot_active;

	bool		m_flip_horizontal;
	bool		m_flip_vertical;

	float		m_depth_bias;
	float		m_z_fight_bias;

	bool		m_visible;
	bool		m_editor_visible;

	bool		m_scale;
	Vector2		m_draw_scale;

	int			m_last_update_frame;

	Vector3		m_last_static_position;
	Rect2D		m_last_static_bounds;
	Vector2		m_last_static_draw_scale;
	int			m_static_frames;

	Rect2D		m_demo_interp_area;
	Vector2		m_demo_interp_offset;
	Vector2		m_demo_interp_draw_scale;

	bool		m_allow_static;

	enum 
	{
		static_frame_threshold = 3,
	};

#ifdef OPT_SHOW_REPLICATED_DEBUG_STATE

	enum
	{
		max_debug_states = 500,
	};

	std::vector<SpriteDebugState> m_debug_states;
	Vector3 m_last_debug_position;

	float m_min_debug_magnitute;
	float m_max_debug_magnitute;

#endif

public:
	SpriteComponent();
	virtual ~SpriteComponent();

	virtual void Pre_Draw();

	virtual int Get_ID();
	virtual bool Is_Used_In_Demo();

	std::string	Get_Script_Class_Name()
	{
		return "SpriteComponent";
	}

	virtual void Serialize_Demo(BinaryStream* stream, DemoVersion::Type version, bool bSaving, float frameDelta);

	void			Set_Object_Mask		(Vector4 color);
	Vector4			Get_Object_Mask		();
	void			Set_Color			(Color color);
	Color			Get_Color			();
	void			Set_Color_Scalar	(Color color);
	Color			Get_Color_Scalar	();
	void			Set_Area			(Rect2D color);
	Rect2D			Get_Area			();
	void			Set_Offset			(Vector2 offset);
	Vector2			Get_Offset			();
	AtlasFrame*		Get_Frame			();
	void			Set_Frame			(AtlasFrame* frame);
	AtlasAnimation*	Get_Animation		();
	void			Set_Animation		(AtlasAnimation* frame);
	float			Get_Animation_Speed	();
	void			Set_Animation_Speed	(float speed);
	void			Set_Flip			(bool h, bool v);
	void			Get_Flip			(bool& h, bool& v);
	void			Set_Visible			(bool value);
	bool			Get_Visible			();
	void			Set_Editor_Visible	(bool value);
	bool			Get_Editor_Visible	();
	void			Set_Depth_Bias		(float value);
	float			Get_Depth_Bias		();
	virtual float	Get_Draw_Depth		();
	void			Set_Scale			(bool value);
	bool			Get_Scale			();
	void			Set_Draw_Scale		(Vector2 value);
	Vector2			Get_Draw_Scale		();
	void			Set_Allow_Static	(bool value);
	bool			Get_Allow_Static	();

	void			Set_Sort_By_Transparency	(bool value);
	bool			Get_Sort_By_Transparency	();


	bool			Is_Oneshot_Finished	();

	void			Play_Oneshot_Animation(AtlasAnimation* name, float speed);

	void			Draw		(const FrameTime& time, RenderPipeline* pipeline, const DrawInstance& instance);

	virtual void	Add_Draw_Instances(const FrameTime& time, std::vector<DrawInstance>& instances);
};

#endif

