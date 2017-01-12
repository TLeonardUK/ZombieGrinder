// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_ACTOR_
#define _ENGINE_ACTOR_

#include "Engine/Engine/FrameTime.h"

#include "Generic/Types/Rect2D.h"
#include "Generic/Types/Vector3.h"

#include "Engine/Demo/DemoManager.h"

class UIScene_Editor;
class BinaryStream;
class OnlineUser;

class Actor
{
	MEMORY_ALLOCATOR(Actor, "Scene");

protected:
	friend class UIScene_Editor;

	static float g_layer_depth_multiplier;

	Vector3 m_position;
	Vector3 m_rotation;
	Vector3 m_scale;
	int		m_layer;
	float	m_depth_bias;
	Rect2D	m_bounding_box;

	Vector3 m_demo_interp_position;
	Vector3 m_demo_interp_rotation;
	Vector3 m_demo_interp_scale;

	bool	m_activated_by_player;

	float	m_depth_y_offset;
	bool	m_depth_y_offset_set;

	int		m_explicit_demo_id;
	int		m_created_demo_frame;

	DemoPlayer m_demo_player;
	int m_initial_demo_id;

public:

	INLINE static float Calculate_Draw_Depth(Rect2D world_bbox, Rect2D camera_bbox, float layer, float bias)
	{
		// Our draw depth is how far from the top of the screen the bottom of our 
		// bounding box bottom is. We also factor in layer depth and bias.
		//float  depth = (world_bbox.Y + world_bbox.Height);// - camera_bbox.Y;
		//return depth + (layer * g_layer_depth_multiplier) + (g_layer_depth_multiplier * 0.5f) + bias;
		float  depth = (world_bbox.Y + world_bbox.Height) - camera_bbox.Y;
		return depth + (layer * g_layer_depth_multiplier) + (g_layer_depth_multiplier * 0.5f) + bias;
	}
	INLINE static float Calculate_Draw_Depth(float y_offset, Rect2D camera_bbox, float layer, float bias)
	{
		// Our draw depth is how far from the top of the screen the bottom of our 
		// bounding box bottom is. We also factor in layer depth and bias.
		//float  depth = y_offset;// - camera_bbox.Y;
		//return depth + (layer * g_layer_depth_multiplier) + (g_layer_depth_multiplier * 0.5f) + bias;
		float  depth = y_offset - camera_bbox.Y;
		return depth + (layer * g_layer_depth_multiplier) + (g_layer_depth_multiplier * 0.5f) + bias;
	}

	virtual void Serialize_Demo(BinaryStream* stream, DemoVersion::Type version, bool bSaving, float frameDelta);

	virtual bool Is_Relevant_To_Demo();
	void Set_Created_On_Demo_Frame(int bValue);
	int Get_Created_On_Demo_Frame();
	void Set_Demo_Player(DemoPlayer value);
	DemoPlayer Get_Demo_Player();
	virtual OnlineUser* Get_Demo_Owner();
	virtual bool Is_Demo_Proxy();

	// Constructors.
	Actor();
	virtual ~Actor();

	// Static modifiers.
	static void  Set_Global_Layer_Depth_Multiplier(float depth);
	static float Get_Global_Layer_Depth_Multiplier();

	// Get modifiers.
	Rect2D	Get_World_Bounding_Box();	
	Vector3	Get_World_Center();

	virtual int		Get_Demo_ID();
	virtual void	Set_Demo_ID(int id);

	virtual bool	Get_Activated_By_Player();
	virtual void	Set_Activated_By_Player(bool position);
	virtual Vector3 Get_Position();
	virtual void	Set_Position(Vector3 position);
	virtual Vector3 Get_Rotation();
	virtual void	Set_Rotation(Vector3 rotation);
	virtual Vector3 Get_Scale();
	virtual void	Set_Scale(Vector3 scale);
	virtual Vector3 Get_Direction();
	virtual int		Get_Layer();
	virtual void	Set_Layer(int layer);
	virtual int		Get_Depth_Bias();
	virtual void	Set_Depth_Bias(float bias);
	virtual int		Get_Depth_Y_Offset();
	virtual void	Set_Depth_Y_Offset(float bias);
	virtual Rect2D	Get_Bounding_Box();
	virtual void	Set_Bounding_Box(Rect2D box);
	virtual float	Get_Draw_Depth();

	bool Is_Depth_Y_Offset_Set()
	{
		return m_depth_y_offset_set;
	}


};

#endif

