// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Scene/Actor.h"
#include "Engine/Scene/Camera.h"
#include "Engine/Renderer/RenderPipeline.h"

#include "Engine/Engine/GameEngine.h"

#include "Engine/Scene/Scene.h"

#include "Engine/IO/BinaryStream.h"

float Actor::g_layer_depth_multiplier = 0.0f;

Actor::Actor()
	: m_position(0, 0, 0)
	, m_rotation(0, 0, 0)
	, m_scale(1, 1, 1)
	, m_layer(0)
	, m_depth_bias(0)
	, m_depth_y_offset(0.0f)
	, m_bounding_box(0, 0, 0, 0)
	, m_depth_y_offset_set(false)
	, m_activated_by_player(false)
	, m_explicit_demo_id(-1)
	, m_initial_demo_id(0)
{
	static int s_demo_id = 0;
	m_initial_demo_id = s_demo_id++;
}

Actor::~Actor()
{
	GameEngine::Get()->Get_Scene()->Remove_Actor(this);
}

int Actor::Get_Demo_ID()
{
	// blah, not a great idea, but it works.
	return m_explicit_demo_id >= 0 ? m_explicit_demo_id : m_initial_demo_id;
}

void Actor::Set_Demo_ID(int id)
{
	m_explicit_demo_id = id;
}

bool Actor::Is_Relevant_To_Demo()
{
	return false;
}

void Actor::Set_Created_On_Demo_Frame(int bValue)
{
	m_created_demo_frame = bValue;
}

int Actor::Get_Created_On_Demo_Frame()
{
	return m_created_demo_frame;
}

OnlineUser* Actor::Get_Demo_Owner()
{
	return NULL;
}

void Actor::Set_Demo_Player(DemoPlayer value)
{
	m_demo_player = value;
}

DemoPlayer Actor::Get_Demo_Player()
{
	return m_demo_player;
}

void Actor::Serialize_Demo(BinaryStream* stream, DemoVersion::Type version, bool bSaving, float frameDelta)
{
	if (bSaving)
	{
		stream->Serialize<float>(bSaving, m_position.X);
		stream->Serialize<float>(bSaving, m_position.Y);
		stream->Serialize<float>(bSaving, m_position.Z);
		stream->Serialize<float>(bSaving, m_rotation.X);
		stream->Serialize<float>(bSaving, m_rotation.Y);
		stream->Serialize<float>(bSaving, m_rotation.Z);
		stream->Serialize<float>(bSaving, m_scale.X);
		stream->Serialize<float>(bSaving, m_scale.Y);
		stream->Serialize<float>(bSaving, m_scale.Z);
	}
	else
	{
		if (frameDelta == 0.0f)
		{
			m_demo_interp_position = m_position;
			m_demo_interp_rotation = m_rotation;
			m_demo_interp_scale = m_scale;
		}

		Vector3 to_position, to_rotation, to_scale;

		stream->Serialize<float>(bSaving, to_position.X);
		stream->Serialize<float>(bSaving, to_position.Y);
		stream->Serialize<float>(bSaving, to_position.Z);
		stream->Serialize<float>(bSaving, to_rotation.X);
		stream->Serialize<float>(bSaving, to_rotation.Y);
		stream->Serialize<float>(bSaving, to_rotation.Z);
		stream->Serialize<float>(bSaving, to_scale.X);
		stream->Serialize<float>(bSaving, to_scale.Y);
		stream->Serialize<float>(bSaving, to_scale.Z);

		m_position.X = Math::Lerp(m_demo_interp_position.X, to_position.X, frameDelta);
		m_position.Y = Math::Lerp(m_demo_interp_position.Y, to_position.Y, frameDelta);
		m_position.Z = Math::Lerp(m_demo_interp_position.Z, to_position.Z, frameDelta);
		m_rotation.X = Math::Lerp(m_demo_interp_rotation.X, to_rotation.X, frameDelta);
		m_rotation.Y = Math::Lerp(m_demo_interp_rotation.Y, to_rotation.Y, frameDelta);
		m_rotation.Z = Math::Lerp(m_demo_interp_rotation.Z, to_rotation.Z, frameDelta);
		m_scale.X = Math::Lerp(m_demo_interp_scale.X, to_scale.X, frameDelta);
		m_scale.Y = Math::Lerp(m_demo_interp_scale.Y, to_scale.Y, frameDelta);
		m_scale.Z = Math::Lerp(m_demo_interp_scale.Z, to_scale.Z, frameDelta);
	}

	stream->Serialize<int>  (bSaving, m_layer);
	stream->Serialize<float>(bSaving, m_depth_bias);
	stream->Serialize<float>(bSaving, m_bounding_box.X);
	stream->Serialize<float>(bSaving, m_bounding_box.Y);
	stream->Serialize<float>(bSaving, m_bounding_box.Width);
	stream->Serialize<float>(bSaving, m_bounding_box.Height);
	stream->Serialize<float>(bSaving, m_depth_y_offset);
	stream->Serialize<bool> (bSaving, m_depth_y_offset_set);
}

Vector3 Actor::Get_Position()
{
	return m_position;
}

void Actor::Set_Position(Vector3 position)
{
	m_position = position;
}

bool Actor::Get_Activated_By_Player()
{
	return m_activated_by_player;
}

void Actor::Set_Activated_By_Player(bool position)
{
	m_activated_by_player = position;
}

Vector3 Actor::Get_Scale()
{
	return m_scale;
}

void Actor::Set_Scale(Vector3 scale)
{
	m_scale = scale;
}

Vector3 Actor::Get_Rotation()
{
	return m_rotation;
}

void Actor::Set_Rotation(Vector3 rotation)
{
	m_rotation = rotation;
}

Vector3 Actor::Get_Direction()
{	
	float horizontal = m_rotation.Y;
	float vertical   = m_rotation.Z;
	
	Vector3 direction
	(
		cos(vertical) * sin(horizontal),
		sin(vertical),
		cos(vertical) * cos(horizontal)
	);

	return direction;
}

int Actor::Get_Layer()
{
	return m_layer;
}

void Actor::Set_Layer(int layer)
{
	m_layer = layer;
}

int Actor::Get_Depth_Bias()
{
	return (int)m_depth_bias;
}

void Actor::Set_Depth_Bias(float bias)
{
	m_depth_bias = bias;
}

int Actor::Get_Depth_Y_Offset()
{
	return (int)m_depth_y_offset;
}

void Actor::Set_Depth_Y_Offset(float bias)
{
	m_depth_y_offset = bias;
	m_depth_y_offset_set = true;
}

Rect2D Actor::Get_World_Bounding_Box()
{
	return Rect2D
	(
		m_position.X + m_bounding_box.X,
		m_position.Y + m_bounding_box.Y,
		m_bounding_box.Width,
		m_bounding_box.Height
	);
}

Vector3 Actor::Get_World_Center()
{
	return Vector3
	(
		m_position.X + m_bounding_box.X + (m_bounding_box.Width * 0.5f),
		m_position.Y + m_bounding_box.Y + (m_bounding_box.Height * 0.5f),
		m_position.Z
	);
}

bool Actor::Is_Demo_Proxy()
{
	return false;
}

Rect2D Actor::Get_Bounding_Box()
{
	return m_bounding_box;
}

void Actor::Set_Bounding_Box(Rect2D box)
{
	m_bounding_box = box;
}

void Actor::Set_Global_Layer_Depth_Multiplier(float depth)
{
	g_layer_depth_multiplier = depth;
}

float Actor::Get_Global_Layer_Depth_Multiplier()
{
	return g_layer_depth_multiplier;
}

float Actor::Get_Draw_Depth()
{
	Rect2D position = GameEngine::Get()->Get_Scene()->Get_Full_Camera_Bounding_Box();

	if (m_depth_y_offset_set == true)
	{
		return Calculate_Draw_Depth(m_position.Y + m_depth_y_offset, position, (float)m_layer, m_depth_bias);
	}
	else
	{
		return Calculate_Draw_Depth(Rect2D(m_position.X + m_bounding_box.X, m_position.Y + m_bounding_box.Y, m_bounding_box.Width, m_bounding_box.Height), position, (float)m_layer, m_depth_bias);
	}

	// Our draw depth is how far from the top of the screen the bottom of our 
	// bounding box bottom is. We also factor in layer depth and bias.
/*
	Camera* camera	 = RenderPipeline::Get()->Get_Active_Camera();
	Rect2D    position = camera->Get_Bounding_Viewport();

	float  depth = (m_position.Y + m_bounding_box.Y + m_bounding_box.Height) - position.Y;
	return depth + (m_layer * g_layer_depth_multiplier) + (g_layer_depth_multiplier * 0.5f) + m_depth_bias;
*/
}