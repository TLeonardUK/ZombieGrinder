// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/UI/Elements/UIPanel.h"
#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Renderer/Atlases/Atlas.h"
#include "Engine/Renderer/Atlases/AtlasHandle.h"
#include "Engine/Renderer/Atlases/AtlasRenderer.h"
#include "Engine/Renderer/Textures/TextureHandle.h"
#include "Engine/Renderer/Textures/Texture.h"
#include "Engine/Resources/ResourceFactory.h"
#include "Engine/UI/UIFrame.h"
#include "Engine/UI/UIManager.h"
#include "Engine/Engine/GameEngine.h"
#include "Engine/Input/Input.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Localise/Locale.h"

UIPanel::UIPanel()
	: m_backcolor(Color::White)
	, m_backimage(NULL)
	, m_backtexture(NULL)
	, m_backtexturehandle(NULL)
	, m_bg_scale_mode(UIPanelBGScaleMode::Scale)
	, m_backimage_name("")
	, m_foreimage(NULL)
	, m_foreimage_name("")
	, m_forecolor(Color::White)
	, m_frame_name("")
	, m_frame_color(Color::White)
	, m_flat_bottom(false)
{
}

UIPanel::~UIPanel()
{
}

void UIPanel::Set_Foreground_Image(AtlasFrame* frame)
{
	m_foreimage = frame;
}

void UIPanel::Set_Background_Image(AtlasFrame* frame)
{
	m_backimage = frame;
	m_backtexture = NULL;
	m_backtexturehandle = NULL;
}

void UIPanel::Set_Background_Image(TextureHandle* frame)
{
	m_backtexturehandle = frame;
	m_backimage = NULL;
	m_backtexture = NULL;
}

void UIPanel::Set_Background_Image(Texture* frame)
{
	m_backtexture = frame;
	m_backtexturehandle = NULL;
	m_backimage = NULL;
}

void UIPanel::Set_Background_Color(Color color)
{
	m_backcolor = color;
}

void UIPanel::Set_Foreground_Color(Color color)
{
	m_forecolor = color;
}

void UIPanel::Set_Frame_Color(Color color)
{
	m_frame_color = color;
}

void UIPanel::Refresh()
{
	// Calculate screen-space box.
	m_screen_box = Calculate_Screen_Box();

	// Load background image.
	if (m_backimage_name != "")
	{
		m_backimage = ResourceFactory::Get()->Get_Atlas_Frame(m_backimage_name.c_str());
	}
	if (m_foreimage_name != "")
	{
		m_foreimage = ResourceFactory::Get()->Get_Atlas_Frame(m_foreimage_name.c_str());
	}
	if (m_frame_name != "")
	{
		m_frame = UIFrame(m_frame_name);
	}

	// Get children to calculate their screen box.
	UIElement::Refresh();
}

void UIPanel::After_Refresh()
{
	// After-refresh children.
	UIElement::After_Refresh();
}

void UIPanel::Tick(const FrameTime& time, UIManager* manager, UIScene* scene)
{
	// Update children.
	UIElement::Tick(time, manager, scene);
}

void UIPanel::Draw(const FrameTime& time, UIManager* manager, UIScene* scene)
{
	// Draw the background.
	if (m_backimage != NULL || m_backtexture != NULL || m_backtexturehandle != NULL)
	{
		Vector2 size;
		Vector2 origin;

		if (m_backtexture != NULL)
		{
			size = Vector2(m_backtexture->Get_Width(), m_backtexture->Get_Height());
			origin = Vector2(0, 0);
		}
		else if (m_backimage != NULL)
		{
			size = Vector2(m_backimage->Rect.Width, m_backimage->Rect.Height);
			origin = m_backimage->Origin;
		}
		else
		{
			size = Vector2(m_backtexturehandle->Get()->Get_Width(), m_backtexturehandle->Get()->Get_Height());
			origin = Vector2(0, 0);
		}

		switch (m_bg_scale_mode)
		{	
		case UIPanelBGScaleMode::Center:
			{
				float center_x = m_screen_box.X + (m_screen_box.Width * 0.5f);
				float center_y = m_screen_box.Y + (m_screen_box.Height * 0.5f);

				center_x -= (size.X * 0.5f);
				center_y -= (size.Y * 0.5f);

				center_x += (origin.X);
				center_y += (origin.Y);

				if (m_backtexture != NULL)
				{
					m_atlas_renderer.Draw(m_backtexture, Rect2D(0, 0, 1, 1), Vector2(0, 0), Rect2D(center_x, center_y, size.X, size.Y), 0.0f, m_backcolor);
				}
				else if (m_backimage != NULL)
				{
					m_atlas_renderer.Draw_Frame(m_backimage, Rect2D(center_x, center_y, size.X, size.Y), 0.0f, m_backcolor);
				}
				else
				{
					m_atlas_renderer.Draw(m_backtexturehandle->Get(), Rect2D(0, 0, 1, 1), Vector2(0, 0), Rect2D(center_x, center_y, size.X, size.Y), 0.0f, m_backcolor);
				}
				break;
			}
		case UIPanelBGScaleMode::FullWidth:
			{
				float scale = 1.0f;
				scale = (float)m_screen_box.Width / (float)size.X;

				Rect2D box = Rect2D
				(
					m_screen_box.X + (m_screen_box.Width * 0.5f)  - ((size.X  * scale) * 0.5f),
					m_screen_box.Y + (m_screen_box.Height * 0.5f) - ((size.Y * scale) * 0.5f),
					size.X * scale,
					size.Y * scale
				);
				
				if (m_backtexture != NULL)
				{
					m_atlas_renderer.Draw(m_backtexture, Rect2D(0, 0, 1, 1), Vector2(0, 0), box, 0.0f, m_backcolor);
				}
				else if (m_backimage != NULL)
				{
					m_atlas_renderer.Draw_Frame(m_backimage, box, 0.0f, m_backcolor);
				}
				else
				{
					m_atlas_renderer.Draw(m_backtexturehandle->Get(), Rect2D(0, 0, 1, 1), Vector2(0, 0), box, 0.0f, m_backcolor);
				}
				break;
			}
		case UIPanelBGScaleMode::FullHeight:
			{
				float scale = 1.0f;
				scale = (float)m_screen_box.Height / (float)size.Y;

				Rect2D box = Rect2D
				(
					m_screen_box.X + (m_screen_box.Width * 0.5f)  - ((size.X * scale) * 0.5f),
					m_screen_box.Y + (m_screen_box.Height * 0.5f) - ((size.Y * scale) * 0.5f),
					size.X * scale,
					size.Y * scale
				);

				if (m_backtexture != NULL)
				{
					m_atlas_renderer.Draw(m_backtexture, Rect2D(0, 0, 1, 1), Vector2(0, 0), box, 0.0f, m_backcolor);
				}
				else if (m_backimage != NULL)
				{
					m_atlas_renderer.Draw_Frame(m_backimage, box, 0.0f, m_backcolor);
				}
				else
				{
					m_atlas_renderer.Draw(m_backtexturehandle->Get(), Rect2D(0, 0, 1, 1), Vector2(0, 0), box, 0.0f, m_backcolor);
				}
				break;
			}
		case UIPanelBGScaleMode::Zoom:
			{
				float x_diff = (size.X - m_screen_box.Height);
				float y_diff = (size.Y - m_screen_box.Width);

				float x_scale = (float)m_screen_box.Width / (float)size.X;
				float y_scale = (float)m_screen_box.Height / (float)size.Y;
				
				float scale = 1.0f;
				float other_scale = 1.0f;

				if (size.X <= size.Y)
				{
					scale = x_scale;
					other_scale = y_scale;
				}
				else
				{					
					scale = y_scale;
					other_scale = x_scale;
				}

				// Hacked to shit. CBA to find out what it's not working correctly on the hostgame screen atm.
				if (size.X * scale > m_screen_box.Width ||
					size.Y * scale > m_screen_box.Height)
				{
					scale = other_scale;
				}

				Rect2D box = Rect2D
				(
					m_screen_box.X + (m_screen_box.Width * 0.5f)  - ((size.X  * scale) * 0.5f),
					m_screen_box.Y + (m_screen_box.Height * 0.5f) - ((size.Y * scale) * 0.5f),
					size.X * scale,
					size.Y * scale
				);
				 
				/*
				// Hacked to shit. CBA to find out what it's not working correctly on the hostgame screen atm.
				if (box.Height > m_screen_box.Height)
				{					
					if (x_diff > y_diff)
					{
						scale = (float)m_screen_box.Height / (float)size.Y;
					}
					else
					{
						scale = (float)m_screen_box.Width / (float)size.X;
					}

					box = Rectangle
					(
						m_screen_box.X + (m_screen_box.Width * 0.5f)  - ((size.X  * scale) * 0.5f),
						m_screen_box.Y + (m_screen_box.Height * 0.5f) - ((size.Y * scale) * 0.5f),
						size.X * scale,
						size.Y * scale
					);				
				}*/

				if (m_backtexture != NULL)
				{
					m_atlas_renderer.Draw(m_backtexture, Rect2D(0, 0, 1, 1), Vector2(0, 0), box, 0.0f, m_backcolor);
				}
				else if (m_backimage != NULL)
				{
					m_atlas_renderer.Draw_Frame(m_backimage, box, 0.0f, m_backcolor);
				}
				else
				{
					m_atlas_renderer.Draw(m_backtexturehandle->Get(), Rect2D(0, 0, 1, 1), Vector2(0, 0), box, 0.0f, m_backcolor);
				}
				break;
			}
		case UIPanelBGScaleMode::Scale:
			// Fallthrough
		default:
			{

				if (m_backtexture != NULL)
				{
					m_atlas_renderer.Draw(m_backtexture, Rect2D(0, 0, 1, 1), Vector2(0, 0), m_screen_box, 0.0f, m_backcolor);
				}
				else if (m_backimage != NULL)
				{
					m_atlas_renderer.Draw_Frame(m_backimage, m_screen_box, 0.0f, m_backcolor);
				}
				else
				{
					m_atlas_renderer.Draw(m_backtexturehandle->Get(), Rect2D(0, 0, 1, 1), Vector2(0, 0), m_screen_box, 0.0f, m_backcolor);
				}
				break;
			}
		}
	}
	else
	{
		if (m_frame_name != "")
		{
			if (m_flat_bottom == true)
			{
				m_frame.Draw_Frame_Flat_Bottom(m_atlas_renderer, m_screen_box, manager->Get_UI_Scale(), m_frame_color);
			}
			else
			{
				m_frame.Draw_Frame(m_atlas_renderer, m_screen_box, manager->Get_UI_Scale(), m_frame_color);
			}
		}
		else
		{
			Color c = RenderPipeline::Get()->Get_Render_Batch_Global_Color();
			m_primitive_renderer.Draw_Solid_Quad(m_screen_box, m_backcolor);
		}
	}

	// Draw foreground.
	if (m_foreimage != NULL)
	{		
		m_atlas_renderer.Draw_Frame(m_foreimage, m_screen_box, 0.0f, m_forecolor);
	}

	// Notify of draw.
	if (On_Draw.Registered())
	{
		UIPanelDrawItem data;
		data.view = this;
		data.view_bounds = m_screen_box;
		data.time = time;
		data.manager = manager;
		data.scene = scene;
		data.ui_scale = manager->Get_UI_Scale();

		On_Draw.Fire(&data);
	}

	// Draw children.
 	UIElement::Draw(time, manager, scene);
}
