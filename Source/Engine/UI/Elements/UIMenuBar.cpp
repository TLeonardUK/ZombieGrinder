// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/UI/Elements/UIMenuBar.h"
#include "Engine/UI/Elements/UIMenuItem.h"
#include "Engine/Renderer/Atlases/Atlas.h"
#include "Engine/Renderer/Atlases/AtlasHandle.h"
#include "Engine/Renderer/Atlases/AtlasRenderer.h"
#include "Engine/UI/UIManager.h"
#include "Engine/Engine/GameEngine.h"

UIMenuBar::UIMenuBar()
	: m_item_padding(5, 5, 5, 5)
{
	m_accepts_mouse_input = true;
}

UIMenuBar::~UIMenuBar()
{
}
	
void UIMenuBar::Refresh()
{
	m_manager			= GameEngine::Get()->Get_UIManager();
	m_atlas				= m_manager->Get_Atlas();
	m_screen_box		= Calculate_Screen_Box();
	m_atlas_renderer	= AtlasRenderer(m_atlas);
	
	// Grab all atlas frames.
	m_background_frame = UIFrame("menubar_background_#");

	// Get children to calculate their screen box.
	UIElement::Refresh();
}

void UIMenuBar::After_Refresh()
{
	// Offset children to create a line within the bar.
	float item_offset = 0.0f;
	for (std::vector<UIElement*>::iterator iter = m_children.begin(); iter != m_children.end(); iter++)
	{
		UIMenuItem* item = dynamic_cast<UIMenuItem*>(*iter);
		if (item != NULL)
		{
			Rect2D box	= item->Get_Screen_Box();
			box.Y		= m_screen_box.Y + m_item_padding.Y;
			box.Height	= m_screen_box.Height - (m_item_padding.Height);
			box.X		= item_offset + m_item_padding.X;
			item->Set_Screen_Box(box);

			item_offset += box.Width + m_item_padding.Width;
		}
	}

	// After-refresh children.
	UIElement::After_Refresh();
}

bool UIMenuBar::Is_Context_Open()
{
		for (std::vector<UIElement*>::iterator iter = m_children.begin(); iter != m_children.end(); iter++)
	{
		UIMenuItem* item = dynamic_cast<UIMenuItem*>(*iter);
		if (item != NULL)
		{
			if (item->Is_Context_Open())
			{
				return true;
			}
		}
	}

	return false;
}

void UIMenuBar::Tick(const FrameTime& time, UIManager* manager, UIScene* scene)
{
	// Update items.	
	UIElement::Tick(time, manager, scene);
}

void UIMenuBar::Draw(const FrameTime& time, UIManager* manager, UIScene* scene)
{
	// Draw background.
	m_background_frame.Draw_Frame(m_atlas_renderer, m_screen_box);
}

void UIMenuBar::Post_Draw(const FrameTime& time, UIManager* manager, UIScene* scene)
{
	// Draw each menu item.
	UIElement::Draw(time, manager, scene);
}


