// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/Helper/UICharacterPreviewRenderer.h"

#include "Engine/Engine/GameEngine.h"

#include "Engine/Localise/Locale.h"

#include "Engine/Resources/ResourceFactory.h"

#include "Game/Scene/Actors/ScriptedActor.h"

#include "Engine/Display/GfxDisplay.h"
#include "Generic/Math/Math.h"
#include "Generic/Math/Random.h"

#include "Game/Profile/ItemManager.h"
#include "Game/Profile/Profile.h"

UICharacterPreviewRenderer::UICharacterPreviewRenderer()
	: m_direction_change_timer(0.0f)
	, m_direction(0)
	, m_last_update_frame(0)
{
	for (int i = 0; i < 8; i++)
	{
		std::string name = "avatar_legs_" + std::string(Direction_Short_Mnemonics[i]);
		m_leg_animations[i] = ResourceFactory::Get()->Get_Atlas_Animation(name.c_str());

		name = "avatar_body_idle_" + std::string(Direction_Short_Mnemonics[i]);
		m_idle_animations[i] = ResourceFactory::Get()->Get_Atlas_Animation(name.c_str());

		DBG_ASSERT(m_leg_animations[i] != NULL);
		DBG_ASSERT(m_idle_animations[i] != NULL);
	}
}

int UICharacterPreviewRenderer::Get_Direction()
{
	return m_direction;
}

void UICharacterPreviewRenderer::Draw_Item(const FrameTime& time, Item* item, Rect2D item_box)
{
	AtlasRenderer atlas_renderer;

	if (item->archetype->is_tintable)
	{
		atlas_renderer.Draw_Frame(item->archetype->icon_animations[m_direction]->Frames[0], item_box, 0.0f, Color::White, false, false, RendererOption::E_Src_Alpha_One_Minus_Src_Alpha, Vector2(1,1));	
		atlas_renderer.Draw_Frame(item->archetype->icon_tint_animations[m_direction]->Frames[0], item_box, 0.0f, item->primary_color, false, false, RendererOption::E_Src_Alpha_One_Minus_Src_Alpha, Vector2(1,1));	
	}
	else
	{
		atlas_renderer.Draw_Frame(item->archetype->icon_animations[m_direction]->Frames[0], item_box, 0.0f, Color::White, false, false, RendererOption::E_Src_Alpha_One_Minus_Src_Alpha, Vector2(1,1));	
	}
}

void UICharacterPreviewRenderer::Draw_Body(const FrameTime& time, Item* item, Rect2D item_box)
{
	AtlasRenderer atlas_renderer;

	AtlasFrame* anim = NULL;
	AtlasFrame* tint_anim = NULL;

	Color tint_color = Color::White;
	bool tintable = false;

	if (item != NULL && item->archetype->body_animation_name != "")
	{
		anim = item->archetype->body_animations[m_direction]->Frames[0];
		tint_color = item->primary_color;
		tintable = item->archetype->is_tintable;

		if (tintable)
		{			
			tint_anim = item->archetype->body_tint_animations[m_direction]->Frames[0];
		}
	}
	else
	{
		anim = m_idle_animations[m_direction]->Frames[0];
		tintable = false;
	}

	if (tintable)
	{
		atlas_renderer.Draw_Frame(anim, item_box, 0.0f, Color::White, false, false, RendererOption::E_Src_Alpha_One_Minus_Src_Alpha, Vector2(1,1));	
		atlas_renderer.Draw_Frame(tint_anim, item_box, 0.0f, tint_color, false, false, RendererOption::E_Src_Alpha_One_Minus_Src_Alpha, Vector2(1,1));	
	}
	else
	{
		atlas_renderer.Draw_Frame(anim, item_box, 0.0f, Color::White, false, false, RendererOption::E_Src_Alpha_One_Minus_Src_Alpha, Vector2(1,1));	
	}
}

void UICharacterPreviewRenderer::Draw(const FrameTime& time, UIManager* manager, Profile* profile, Rect2D box)
{
	Vector2 scale = manager->Get_UI_Scale();
	
	AtlasRenderer atlas_renderer;

	Item* head_item			= profile->Get_Item_Slot(ItemSlot::Head);
	Item* accessory_item	= profile->Get_Item_Slot(ItemSlot::Accessory);
	Item* weapon_item		= profile->Get_Item_Slot(ItemSlot::Weapon);

	Rect2D item_box;
	item_box.Width	= box.Width * 1.0f;
	item_box.Height = item_box.Width;
	item_box.X		= (box.X + (box.Width * 0.5f)) - (item_box.Width * 0.5f);
	item_box.Y		= (box.Y + (box.Height * 0.4f)) - (item_box.Height * 0.5f);
	
	// Update direction.
	if (time.Get_Frame() != m_last_update_frame)
	{
		m_direction_change_timer += time.Get_Frame_Time();
		if (m_direction_change_timer >= DIRECTION_CHANGE_INTERVAL)
		{
			m_direction = (m_direction + 1) % 8;
			m_direction_change_timer = 0.0f;
		}
		m_last_update_frame = time.Get_Frame();
	}

	int should_draw_weapon_on_top = (m_direction == Directions::S || m_direction == Directions::SE || m_direction == Directions::SW || m_direction == Directions::E || m_direction == Directions::W);
	
	// Draw legs.
	atlas_renderer.Draw_Frame(m_leg_animations[m_direction]->Frames[0], item_box, 0.0f, Color::White, false, false, RendererOption::E_Src_Alpha_One_Minus_Src_Alpha, Vector2(1,1));	
	
	// Draw weapon if below.
	if (!should_draw_weapon_on_top)
	{
		Draw_Body(time, weapon_item, item_box);
	}

	// Draw head.
	Draw_Item(time, head_item, item_box);

	// Draw accessory.
	if (accessory_item != NULL && accessory_item->archetype->is_icon_overlay)
	{
		Draw_Item(time, accessory_item, item_box);
	}

	// Draw weapon if on top.
	if (should_draw_weapon_on_top)
	{
		Draw_Body(time, weapon_item, item_box);
	}
}

void UICharacterPreviewRenderer::Draw_Head(const FrameTime& time, UIManager* manager, Profile* profile, Rect2D box)
{
	Vector2 scale = manager->Get_UI_Scale();
	
	AtlasRenderer atlas_renderer;

	Item* head_item			= profile->Get_Item_Slot(ItemSlot::Head);
	Item* accessory_item	= profile->Get_Item_Slot(ItemSlot::Accessory);

	Rect2D item_box;
	item_box.Width	= box.Width * 1.25f;
	item_box.Height = item_box.Width;
	item_box.X		= (box.X + (box.Width * 0.5f)) - (item_box.Width * 0.5f);
	item_box.Y		= (box.Y + (box.Height * 0.45f)) - (item_box.Height * 0.5f);
	
	// Update direction.
	if (time.Get_Frame() != m_last_update_frame)
	{
		m_direction_change_timer += time.Get_Frame_Time();
		if (m_direction_change_timer >= DIRECTION_CHANGE_INTERVAL)
		{
			m_direction = (m_direction + 1) % 8;
			m_direction_change_timer = 0.0f;
		}
		m_last_update_frame = time.Get_Frame();
	}

	// Draw head.
	Draw_Item(time, head_item, item_box);

	// Draw accessory.
	if (accessory_item != NULL && accessory_item->archetype->is_icon_overlay)
	{
		Draw_Item(time, accessory_item, item_box);
	}
}
