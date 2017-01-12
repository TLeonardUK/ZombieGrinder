// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Scene/Actors/Components/Drawable/SpriteComponent.h"
#include "Engine/Renderer/Atlases/Atlas.h"
#include "Engine/Renderer/Atlases/AtlasRenderer.h"
#include "Engine/Scene/Animation.h"

#include "Engine/Engine/GameEngine.h"

#include "Game/Runner/Game.h"
#include "Game/Scene/GameScene.h"

#include "Engine/Scene/Camera.h"

#include "Game/Network/GameNetManager.h"
#include "Game/Network/GameNetClient.h"

#include "Game/Scene/Actors/ScriptedActor.h"
#include "XScript/VirtualMachine/CVMBinary.h"

#include "Engine/Resources/ResourceFactory.h"

#include "Engine/Profiling/ProfilingManager.h"

#include "Generic/Helper/PersistentLogHelper.h"

SpriteComponent::SpriteComponent()
	: m_flip_horizontal(false)
	, m_flip_vertical(false)
	, m_color(Color::White)
	, m_color_scalar(Color::White)
	, m_frame(NULL)
	, m_visible(true)
	, m_editor_visible(true)
	, m_animation_speed(1.0f)
	, m_depth_bias(0.0f)
	, m_oneshot_active(false)
	, m_scale(false)
	, m_area(0, 0, 0, 0)
	, m_offset(0.0f, 0.0f)
	, m_draw_scale(1.0f, 1.0f)
	, m_last_update_frame(0)
	, m_sort_by_transparency(false)
	, m_static_frames(0)
	, m_allow_static(true)
	, m_object_mask(0.0f, 0.0f, 0.0f, 1.0f)
{
	Set_Render_Slot("geometry");

	// Add a very small random depth bias to prevent z-flicking of sprites at the same depth.
	static int draw_depth_counter = 0;	 
	m_z_fight_bias = ((draw_depth_counter++) % 500) * 0.001f;
}

SpriteComponent::~SpriteComponent()
{
}

void SpriteComponent::Serialize_Demo(BinaryStream* stream, DemoVersion::Type version, bool bSaving, float frameDelta)
{
	Component::Serialize_Demo(stream, version, bSaving, frameDelta);

	stream->Serialize<bool>(bSaving, m_allow_static);
	stream->Serialize<float>(bSaving, m_object_mask.X);
	stream->Serialize<float>(bSaving, m_object_mask.Y);
	stream->Serialize<float>(bSaving, m_object_mask.Z);
	stream->Serialize<float>(bSaving, m_object_mask.W);
	stream->Serialize<unsigned char>(bSaving, m_color.R);
	stream->Serialize<unsigned char>(bSaving, m_color.G);
	stream->Serialize<unsigned char>(bSaving, m_color.B);
	stream->Serialize<unsigned char>(bSaving, m_color.A);
	stream->Serialize<unsigned char>(bSaving, m_color_scalar.R);
	stream->Serialize<unsigned char>(bSaving, m_color_scalar.G);
	stream->Serialize<unsigned char>(bSaving, m_color_scalar.B);
	stream->Serialize<unsigned char>(bSaving, m_color_scalar.A);
	
	int frame_hash = m_frame == NULL ? 0 : m_frame->NameHash;
	stream->Serialize<int>(bSaving, frame_hash);

	if (!bSaving)
	{
		m_frame = frame_hash == 0 ? NULL : ResourceFactory::Get()->Get_Atlas_Frame(frame_hash);
	}

	m_animation.Serialize_Demo(stream, version, bSaving, frameDelta);

	stream->Serialize<bool>(bSaving, m_oneshot_active);
	m_oneshot_anim.Serialize_Demo(stream, version, bSaving, frameDelta);

	stream->Serialize<bool>(bSaving, m_sort_by_transparency);
	stream->Serialize<bool>(bSaving, m_flip_horizontal);
	stream->Serialize<bool>(bSaving, m_flip_vertical);
	stream->Serialize<float>(bSaving, m_depth_bias);
	stream->Serialize<float>(bSaving, m_z_fight_bias);
	stream->Serialize<bool>(bSaving, m_visible);
	stream->Serialize<bool>(bSaving, m_scale);

	if (bSaving)
	{
		stream->Serialize<float>(bSaving, m_area.X);
		stream->Serialize<float>(bSaving, m_area.Y);
		stream->Serialize<float>(bSaving, m_area.Width);
		stream->Serialize<float>(bSaving, m_area.Height);
		stream->Serialize<float>(bSaving, m_offset.X);
		stream->Serialize<float>(bSaving, m_offset.Y);
		stream->Serialize<float>(bSaving, m_draw_scale.X);
		stream->Serialize<float>(bSaving, m_draw_scale.Y);
	}
	else
	{
		if (frameDelta == 0.0f)
		{
			m_demo_interp_area = m_area;
			m_demo_interp_offset = m_offset;
			m_demo_interp_draw_scale = m_draw_scale;
		}

		Rect2D to_area;
		Vector2 to_offset;
		Vector2 to_draw_scale;

		stream->Serialize<float>(bSaving, to_area.X);
		stream->Serialize<float>(bSaving, to_area.Y);
		stream->Serialize<float>(bSaving, to_area.Width);
		stream->Serialize<float>(bSaving, to_area.Height);
		stream->Serialize<float>(bSaving, to_offset.X);
		stream->Serialize<float>(bSaving, to_offset.Y);
		stream->Serialize<float>(bSaving, to_draw_scale.X);
		stream->Serialize<float>(bSaving, to_draw_scale.Y);

		m_area.X = Math::Lerp(m_demo_interp_area.X, to_area.X, frameDelta);
		m_area.Y = Math::Lerp(m_demo_interp_area.Y, to_area.Y, frameDelta);
		m_area.Width = Math::Lerp(m_demo_interp_area.Width, to_area.Width, frameDelta);
		m_area.Height = Math::Lerp(m_demo_interp_area.Height, to_area.Height, frameDelta);

 		m_offset.X = Math::Lerp(m_demo_interp_offset.X, to_offset.X, frameDelta);
		m_offset.Y = Math::Lerp(m_demo_interp_offset.Y, to_offset.Y, frameDelta);

		m_draw_scale.X = Math::Lerp(m_demo_interp_draw_scale.X, to_draw_scale.X, frameDelta);
		m_draw_scale.Y = Math::Lerp(m_demo_interp_draw_scale.Y, to_draw_scale.Y, frameDelta);
	}
}

int SpriteComponent::Get_ID()
{
	static int hash = StringHelper::Hash("SpriteComponent");
	return hash;
}

bool SpriteComponent::Is_Used_In_Demo()
{
	return true;
}

void SpriteComponent::Pre_Draw()
{
	FrameTime& time = *GameEngine::Get()->Get_Time();

	// Advance animation time (only on first camera if in split screen).
	if (time.Get_Frame() != m_last_update_frame)
	{
		m_animation.Advance(time, m_animation_speed);
		if (m_oneshot_active == true)
		{
			m_oneshot_anim.Advance(time, m_oneshot_anim_speed);
			if (m_oneshot_anim.Is_Complete())
			{
				m_oneshot_active = false;
			}
		}
		m_last_update_frame = time.Get_Frame();
	}
}

void SpriteComponent::Set_Object_Mask(Vector4 color)
{
	m_object_mask = color;
}

Vector4	SpriteComponent::Get_Object_Mask()
{
	return m_object_mask;
}

void SpriteComponent::Set_Color(Color color)
{
	m_color = color;
}

Color SpriteComponent::Get_Color()
{
	return m_color;
}

void SpriteComponent::Set_Color_Scalar(Color color)
{
	m_color_scalar = color;
}

Color SpriteComponent::Get_Color_Scalar()
{
	return m_color_scalar;
}

void SpriteComponent::Set_Area(Rect2D color)
{
	m_area = color;
}

Rect2D SpriteComponent::Get_Area()
{
	return m_area;
}

void SpriteComponent::Set_Offset(Vector2 color)
{
	m_offset = color;
}

Vector2 SpriteComponent::Get_Offset()
{
	return m_offset;
}

void SpriteComponent::Set_Draw_Scale(Vector2 color)
{
	m_draw_scale = color;
}

Vector2 SpriteComponent::Get_Draw_Scale()
{
	return m_draw_scale;
}

AtlasFrame*	SpriteComponent::Get_Frame()
{
	return m_frame;
}

void SpriteComponent::Set_Frame(AtlasFrame* frame)
{
	m_frame = frame;
}

void SpriteComponent::Set_Sort_By_Transparency(bool value)
{
	m_sort_by_transparency = value;
}

bool SpriteComponent::Get_Sort_By_Transparency()
{
	return m_sort_by_transparency;
}

void SpriteComponent::Set_Allow_Static(bool value)
{
	m_allow_static = value;
}

bool SpriteComponent::Get_Allow_Static()
{
	return m_allow_static;
}

AtlasAnimation*	SpriteComponent::Get_Animation()
{
	return m_animation.Get_Atlas_Animation();
}

void SpriteComponent::Set_Animation(AtlasAnimation* frame)
{
	if (frame != m_animation.Get_Atlas_Animation())
	{
		m_animation = Animation(frame);
	}
}

float SpriteComponent::Get_Animation_Speed()
{
	return m_animation_speed;
}

void SpriteComponent::Set_Animation_Speed(float speed)
{
	m_animation_speed = fabs(speed);
	m_animation.Set_Reversed(speed < 0.0f);
}

void SpriteComponent::Set_Flip(bool h, bool v)
{
	m_flip_horizontal = h;
	m_flip_vertical = v;
}

void SpriteComponent::Get_Flip(bool& h, bool& v)
{
	h = m_flip_horizontal;
	v = m_flip_vertical;
}

void SpriteComponent::Set_Visible(bool value)
{
	m_visible = value;
}

bool SpriteComponent::Get_Visible()
{
	return m_visible;
}

void SpriteComponent::Set_Scale(bool value)
{
	m_scale = value;
}

bool SpriteComponent::Get_Scale()
{
	return m_scale;
}

void SpriteComponent::Set_Editor_Visible(bool value)
{
	m_editor_visible = value;
}

bool SpriteComponent::Get_Editor_Visible()
{
	return m_editor_visible;
}

void SpriteComponent::Set_Depth_Bias(float value)
{
	m_depth_bias = value;
}

float SpriteComponent::Get_Depth_Bias()
{
	return m_depth_bias;
}

float SpriteComponent::Get_Draw_Depth()
{
	return m_parent->Get_Draw_Depth() + m_depth_bias + m_z_fight_bias;
}

void SpriteComponent::Play_Oneshot_Animation(AtlasAnimation* name, float speed)
{
	m_oneshot_anim = Animation(name, speed < 0.0f);
	m_oneshot_anim_speed = fabs(speed);
	m_oneshot_active = true;
}

void SpriteComponent::Add_Draw_Instances(const FrameTime& time, std::vector<DrawInstance>& instances) 
{
	if (!m_active)
	{
		return;
	}

	float draw_depth = Get_Draw_Depth();

	AtlasFrame* frame = m_frame;
	if (frame == NULL)
	{
		frame = m_animation.Get_Atlas_Frame();
	}

	Color output_color = m_color * m_color_scalar;

	if (frame != NULL)
	{
		draw_depth += frame->DepthBias;

		if (frame->MultiLayerFrames.size() == 0)
		{
			DrawInstance instance;
			instance.draw_depth = draw_depth;
			instance.draw_index = 0;
			instance.drawable = this;
			instance.transparent = (frame->HasSemiTransparentPixels || (output_color.A < 255)) && m_sort_by_transparency;
			instances.push_back(instance);
		}
		else
		{
			for (unsigned int i = 0; i < frame->MultiLayerFrames.size(); i++)
			{
				AtlasMultiLayerFrame f = frame->MultiLayerFrames.at(i);

				DrawInstance instance;
				instance.draw_depth = draw_depth + (f.LayerOffset * Actor::Get_Global_Layer_Depth_Multiplier());
				instance.draw_index = i;
				instance.drawable = this;
				instance.transparent = (f.Frame->HasSemiTransparentPixels || (output_color.A < 255)) && m_sort_by_transparency;
				instances.push_back(instance);
			}
		}
	}
}

bool SpriteComponent::Is_Oneshot_Finished()
{
	return !m_oneshot_active;
}

void SpriteComponent::Draw(const FrameTime& time, RenderPipeline* pipeline, const DrawInstance& instance)
{
	if (!m_active)
	{
		return;
	}

	GameNetClient* client = GameNetManager::Get()->Game_Client();
	bool in_editor_mode = client != NULL && client->Get_Server_State().In_Editor_Mode;

	// Cull if not visible.
	bool editor_visible = false;
	if (in_editor_mode && !Game::Get()->Get_Game_Scene()->Get_Editor_Force_Hide())
	{
		editor_visible = (m_editor_visible == true);
	}
	const bool is_not_visible = (m_visible == false && editor_visible == false);
	const bool no_frame = (m_frame == NULL && m_animation.Get_Atlas_Frame() == NULL);
	if (m_parent == NULL || is_not_visible || no_frame)
	{
		return;
	}

	Vector3 position   = m_parent->Get_Position();
	float   draw_depth = Get_Draw_Depth();//m_parent->Get_Draw_Depth();
	Rect2D	bounds	   = m_parent->Get_Bounding_Box();
	float   rotation   = m_parent->Get_Rotation().Z;

	position.X += m_offset.X;
	position.Y += m_offset.Y;

	if (position.X == m_last_static_position.X && position.Y == m_last_static_position.Y &&
		bounds == m_last_static_bounds &&
		m_draw_scale == m_last_static_draw_scale)
	{
		m_static_frames++;
	}
	else
	{
		m_static_frames = 0;
	}

	m_last_static_position = position;
	m_last_static_bounds = bounds;
	m_last_static_draw_scale = m_draw_scale;

	if (m_area.X != 0 || m_area.Y != 0 || m_area.Width != 0 || m_area.Height != 0)
	{
		bounds = m_area;
	}

	AtlasRenderer renderer;

	AtlasFrame* frame = m_frame;
	if (frame == NULL)
	{
		frame = m_animation.Get_Atlas_Frame();
	}

	if (m_oneshot_active == true)
	{
		frame = m_oneshot_anim.Get_Atlas_Frame();
	}

#ifdef OPT_SHOW_REPLICATED_DEBUG_STATE

	Vector3 center = position + Vector3(bounds.Center().X, bounds.Center().Y, 0.0f);

	// Debug state stuff.
	ScriptedActor* scripted_parent = dynamic_cast<ScriptedActor*>(m_parent);
	if (scripted_parent->Get_Owner() != NULL)
	{
		scripted_parent = NULL;
	}

	if (scripted_parent != NULL)
	{
		if (scripted_parent->Get_Script_Symbol()->symbol->class_data->is_replicated)
		{
			bool bLastSprite = true;

			for (int i = m_parent->Get_Component_Count() - 1; i >= 0; i--)
			{
				Component* component = m_parent->Get_Component_Index(i);
				if (dynamic_cast<SpriteComponent*>(component) != NULL && 
					dynamic_cast<SpriteComponent*>(component)->Get_Visible())
				{
					bLastSprite = (component == this);
					break;
				}
			}

			if (bLastSprite)
			{
				SpriteDebugState state;
				state.Position = center;
				state.Velocity = (center - m_last_debug_position) / time.Get_Frame_Time();

				if (state.Velocity.Length() < 8.0f) // Ignore huge jumps, don't show us anything useful.
				{
					m_debug_states.push_back(state);
				}

				if (m_debug_states.size() >= max_debug_states)
				{
					m_debug_states.erase(m_debug_states.begin());
				}
			}
		}
	}

	m_last_debug_position = center;

	if (m_debug_states.size() == 1)
	{
		m_min_debug_magnitute = 999999;
		m_max_debug_magnitute = -999999;
	}

	if (m_debug_states.size() >= 2)
	{
		for (unsigned int i = 1; i < m_debug_states.size(); i++)
		{
			SpriteDebugState& state = m_debug_states[i];
			float magnitute = state.Velocity.Length();
			if (magnitute < m_min_debug_magnitute)
			{
				m_min_debug_magnitute = magnitute;
			}
			if (magnitute > m_max_debug_magnitute)
			{
				m_max_debug_magnitute = magnitute;
			}
		}

		m_min_debug_magnitute = 0.0f;
		m_max_debug_magnitute = 0.1f;

		PrimitiveRenderer velocity_renderer;
		for (unsigned int i = 1; i < m_debug_states.size(); i++)
		{
			SpriteDebugState& last_state = m_debug_states[i - 1];
			SpriteDebugState& state = m_debug_states[i];

			float magnitute = state.Velocity.Length();
			float delta = Clamp((magnitute - m_min_debug_magnitute) / (m_max_debug_magnitute - m_min_debug_magnitute), 0.0f, 1.0f);

			Color color;
			color.R = (u8)(Math::Lerp(255.0f, 0.0f, delta));
			color.G = (u8)(Math::Lerp(0.0f, 255.0f, delta));
			color.B = (u8)(0.0f);
			color.A = (u8)(255.0f);

			velocity_renderer.Draw_Line(last_state.Position, state.Position, 3.0f, color);
		}
	}

#endif

	if (frame != NULL)
	{
		draw_depth += frame->DepthBias;

		if (frame->MultiLayerFrames.size() == 0)
		{
			// Center sprite in our bounding box.
			float draw_w = frame->Rect.Width;
			float draw_h = frame->Rect.Height;

			if (m_scale)
			{
				draw_w = bounds.Width;
				draw_h = bounds.Height;
			}

			draw_w *= m_draw_scale.X;
			draw_h *= m_draw_scale.Y;

			float draw_x = (position.X + bounds.X + (bounds.Width * 0.5f)) - (draw_w * 0.5f);
			float draw_y = (position.Y + bounds.Y + (bounds.Height * 0.5f)) - (draw_h * 0.5f);

			// If we are static then round to nearest pixel boundry. Prevents nasty aliasing 
			// when the camera moves in fractional pixels.
			if (m_static_frames > static_frame_threshold && m_allow_static)
			{
				draw_x = floorf(draw_x);
				draw_y = floorf(draw_y);
			}

		/*	if (frame->Name.find("turret") != std::string::npos && m_color_scalar.A >= 0.95f && m_color.A >= 0.95f)
			{
				DBG_LOG("Sprite=%s Layer=%i FrameDepth=%.2f (m_depth_bias=%.1f + m_z_fight_bias=%.1f + m_parent=%.1f) = depth=%.1f ", 
					frame->Name.c_str(),
					Get_Parent()->Get_Layer(),
					frame->DepthBias,
					m_depth_bias,
					m_z_fight_bias,
					(float)Get_Parent()->Get_Depth_Bias(),
					Get_Draw_Depth());
			}
		*/

			renderer.Draw_Frame(
				frame, 
				Rect2D(draw_x, draw_y, draw_w, draw_h), 
				draw_depth, 
				m_color * m_color_scalar, 
				m_flip_horizontal, 
				m_flip_vertical, 
				RendererOption::E_Src_Alpha_One_Minus_Src_Alpha, 
				Vector2(1.0f, 1.0f), 
				rotation,
				m_object_mask
			);
		}
		else
		{
			for (unsigned int i = 0; i < frame->MultiLayerFrames.size(); i++)
			{
				AtlasMultiLayerFrame f = frame->MultiLayerFrames.at(i);
	
				if (i != instance.draw_index)
				{
					continue;
				}

				// Center sprite in our bounding box.
				float draw_w = f.Frame->Rect.Width;
				float draw_h = f.Frame->Rect.Height;

				if (m_scale)
				{
					draw_w = bounds.Width;
					draw_h = bounds.Height;
				}

				draw_w *= m_draw_scale.X;
				draw_h *= m_draw_scale.Y;

				float draw_x = (position.X + bounds.X + (bounds.Width * 0.5f)) - (draw_w * 0.5f);
				float draw_y = (position.Y + bounds.Y + (bounds.Height * 0.5f)) - (draw_h * 0.5f);

				// If we are static then round to nearest pixel boundry. Prevents nasty aliasing 
				// when the camera moves in fractional pixels.
				if (m_static_frames > static_frame_threshold)
				{
					draw_x = floorf(draw_x);
					draw_y = floorf(draw_y);
				}

				renderer.Draw_Frame(
					f.Frame, 
					Rect2D(draw_x, draw_y, draw_w, draw_h), 
					draw_depth + (f.LayerOffset * Actor::Get_Global_Layer_Depth_Multiplier()), 
					m_color * m_color_scalar, 
					m_flip_horizontal, 
					m_flip_vertical, 
					RendererOption::E_Src_Alpha_One_Minus_Src_Alpha, 
					Vector2(1.0f, 1.0f), 
					rotation
				);
			}
		}
	}

	/*
	ScriptedActor* actor = dynamic_cast<ScriptedActor*>(m_parent);
	if (m_frame == NULL && m_animation.Get_Frame() != NULL && actor && actor->Get_Script_Symbol()->symbol->class_data->is_replicated && m_animation.Get_Atlas_Animation()->Name.find("dungeon") != std::string::npos)
	{
		DBG_ONSCREEN_LOG(actor->Get_Replication_Info().unique_id, Color::Green, 1.0f, "name=%s, speed=%.2f, reversed=%i, last_speed=%.2f, last_callstack=%s", 
			m_animation.Get_Atlas_Animation()->Name.c_str(), 
			m_animation_speed, 
			m_animation.Get_Reversed(),
			m_debug_last_anim_time,
			m_debug_anim_set_at.c_str());
	}
	*/
}