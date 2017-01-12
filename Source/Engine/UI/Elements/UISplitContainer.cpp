// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/UI/Elements/UISplitContainer.h"
#include "Engine/UI/Elements/UISplitContainerItem.h"
#include "Engine/Renderer/Atlases/Atlas.h"
#include "Engine/Renderer/Atlases/AtlasHandle.h"
#include "Engine/Renderer/Atlases/AtlasRenderer.h"
#include "Engine/Renderer/PrimitiveRenderer.h"
#include "Engine/UI/UIFrame.h"
#include "Engine/UI/UIManager.h"
#include "Engine/Engine/GameEngine.h"
#include "Engine/Input/Input.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Localise/Locale.h"
#include "Generic/Math/Math.h"

UISplitContainer::UISplitContainer()
	: m_resizable(false)
	, m_direction(SplitContainerDirection::Horizontal)
	, m_split_seperator_width(6)
	, m_split_offset(0)
	, m_moving_split(false)
	, m_split_offset_min(0)
	, m_split_offset_max(0)
{
}

UISplitContainer::~UISplitContainer()
{
}

void UISplitContainer::Refresh()
{
	Rect2D our_container = Calculate_Screen_Box();

	// Grab some general stuff we need.
	m_manager			= GameEngine::Get()->Get_UIManager();
	m_atlas				= m_manager->Get_Atlas();
	m_atlas_renderer	= AtlasRenderer(m_atlas);

	//if (m_split_offset <= 1.0f)
	//{
	//	m_split_offset *= our_container.Width;
	//}

	// Grab all atlas frames.
	m_background_frame = UIFrame("splitcontainer_background_#");

	// Find size of fixed container.
	bool  container_fixed	   = false;
	float container_fixed_size = 0.0f;

	bool item_collapsed = false;

	for (std::vector<UIElement*>::iterator iter = m_children.begin(); iter != m_children.end(); iter++)
	{
		UISplitContainerItem* element = static_cast<UISplitContainerItem*>(*iter);
		if (element->m_fixed == true)
		{
			if (container_fixed == true)
			{
				DBG_LOG("Unable to calculate split containers dimensions, multiple fixed container items.");
				UIElement::Refresh();
				return;
			}

			container_fixed = true;
			container_fixed_size = element->m_size;
		}
		if (element->m_collapsed == true)
		{
			item_collapsed = true;
		}
	}

	// If no items are fixed we split equally.
	float non_fixed_container_size = 0.0f;
	if (container_fixed == false)
	{
		if (m_direction == SplitContainerDirection::Vertical)
		{
			container_fixed_size = our_container.Width * 0.5f;
		}
		else
		{
			container_fixed_size = our_container.Height * 0.5f;
		}
		non_fixed_container_size = container_fixed_size;
	}
	else
	{
		// Calculate the floating containers size.
		if (container_fixed_size <= 1)
		{
			non_fixed_container_size = 1.0f - container_fixed_size;
		}
		else
		{
			if (m_direction == SplitContainerDirection::Vertical)
			{
				non_fixed_container_size = our_container.Width - container_fixed_size;
			}
			else
			{
				non_fixed_container_size = our_container.Height - container_fixed_size;
			}
		}
	}

	// If resizable then adjust sizes.
	if (m_resizable == true)
	{
		container_fixed_size -= (m_split_seperator_width / 2);
		non_fixed_container_size -= (m_split_seperator_width / 2);
	}

	// Work out min/max splitter positions.
	// TODO: This is wrong as fuck, we don't know which child is fixed and which is not
	//		 this may need flipping. We should calculate this based on which item is fixed.
	m_split_offset_min = -container_fixed_size;
	m_split_offset_max = non_fixed_container_size;

	// Recalculate child boxes.
	float	x_offset	= 0;
	float	y_offset	= 0;
	int		index		= 0;
	int		bar_offset  = 0;

	for (std::vector<UIElement*>::iterator iter = m_children.begin(); iter != m_children.end(); iter++)
	{
		UISplitContainerItem* element = static_cast<UISplitContainerItem*>(*iter);

		Rect2D box;		
		box.X = our_container.X + x_offset;
		box.Y = our_container.Y + y_offset;

		if (m_direction == SplitContainerDirection::Vertical)
		{
			if (element->m_fixed == true)
			{
				box.Width = container_fixed_size;
			}
			else
			{
				box.Width = non_fixed_container_size;
			}

			if (m_resizable == true && index == 0)
			{
				box.Width += m_split_offset;
				if (element->m_fixed == true)
				{
					container_fixed_size -= m_split_offset;
				}
				else
				{
					non_fixed_container_size -= m_split_offset;
				}
			}

			if (index == 0)
			{
				bar_offset = (int)(box.X + box.Width);
			}

			box.Height = our_container.Height;
			x_offset += box.Width;

			if (m_resizable == true)
			{
				x_offset += m_split_seperator_width;
			}
		}
		else
		{
			if (element->m_fixed == true)
			{
				box.Height = container_fixed_size;
			}
			else
			{
				box.Height = non_fixed_container_size;
			}

			if (m_resizable == true && index == 0)
			{
				box.Height += m_split_offset;
				if (element->m_fixed == true)
				{
					container_fixed_size -= m_split_offset;
				}
				else
				{
					non_fixed_container_size -= m_split_offset;
				}
			}
			
			if (index == 0)
			{
				bar_offset = (int)(box.Y + box.Height);
			}

			box.Width = our_container.Width;
			y_offset += box.Height;
			
			if (m_resizable == true)
			{
				y_offset += m_split_seperator_width;
			}
		}
		
		if (element->Get_Collapsed() == true)
		{
			element->Set_Screen_Box(box);
		}
		else
		{
			if (item_collapsed == true)
			{
				element->Set_Screen_Box(our_container);
			}
			else
			{
				element->Set_Screen_Box(box);
			}
		}
		index++;
	}

	// Calculate the split seperator position.
	if (m_resizable == true)
	{
		if (m_direction == SplitContainerDirection::Vertical)
		{
			m_split_bar_rect = Rect2D
			(
				(float)(bar_offset),
				ceilf(our_container.Y),
				ceilf(m_split_seperator_width),
				ceilf(our_container.Height)
			);
		}
		else
		{
			m_split_bar_rect = Rect2D
			(
				ceilf(our_container.X),
				(float)(bar_offset),
				ceilf(our_container.Width),
				ceilf(m_split_seperator_width)
			);
		}
	}

	// Get children to calculate their screen box.
	UIElement::Refresh();
	UIElement::After_Refresh();
}

void UISplitContainer::After_Refresh()
{
	// After-refresh children.
	UIElement::After_Refresh();
}

void UISplitContainer::Tick(const FrameTime& time, UIManager* manager, UIScene* scene)
{
	Input*		input	 = Input::Get();
	MouseState* mouse	 = input->Get_Mouse_State();
	Vector2		position = mouse->Get_Position();

	// Moving split?
	if (m_resizable == true)
	{
		if (m_moving_split == true)
		{
			if (!mouse->Is_Button_Down(InputBindings::Mouse_Left))
			{
				m_moving_split = false;
			}
			else 
			{
				float original = m_split_offset;

				// Calculate absolute position.
				if (m_direction == SplitContainerDirection::Vertical)
				{
					float mouse_offset = (m_split_bar_rect.X + (m_split_seperator_width / 2));
					m_split_offset += ceilf(position.X - mouse_offset);
				}
				else
				{
					float mouse_offset = (m_split_bar_rect.Y + (m_split_seperator_width / 2));
					m_split_offset += ceilf(position.Y - mouse_offset);
				}

				// Cap split offset.
				m_split_offset = Max(m_split_offset_min, Min(m_split_offset_max, m_split_offset));

				// Refresh layout.
				if (original != m_split_offset)
				{					
					Refresh();
				}
			}
		}

		if (m_split_bar_rect.Intersects(position))
		{
			if (m_direction == SplitContainerDirection::Horizontal)
			{
				manager->Set_Cursor(UICursorType::SplitH);
			}
			else
			{
				manager->Set_Cursor(UICursorType::SplitV);
			}

			if (m_moving_split == false)
			{
				if (!mouse->Is_Button_Down(InputBindings::Mouse_Left))
				{
					m_moving_split = true;
				}
			}
		}
	}

	// Update children.
	UIElement::Tick(time, manager, scene);
}

void UISplitContainer::Draw(const FrameTime& time, UIManager* manager, UIScene* scene)
{	
	bool item_collapsed = false;

	for (std::vector<UIElement*>::iterator iter = m_children.begin(); iter != m_children.end(); iter++)
	{
		UISplitContainerItem* element = static_cast<UISplitContainerItem*>(*iter);
		if (element->Get_Collapsed() == true)
		{
			item_collapsed = true;
			break;
		}
	}

	// Render spliting bar.
	if (m_resizable == true && item_collapsed == false)
	{			
		// Draw background.
		m_background_frame.Draw_Frame(m_atlas_renderer, m_split_bar_rect);
	}

	// Draw children.
	UIElement::Draw(time, manager, scene);
}
