// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/UI/UIFrame.h"
#include "Engine/Renderer/Atlases/AtlasRenderer.h"
#include "Engine/Renderer/Atlases/AtlasHandle.h"
#include "Engine/Resources/ResourceFactory.h"

HashTable<UIFrame, int> UIFrame::m_hashed_frames;

UIFrame::UIFrame()
{
}

UIFrame::UIFrame(std::string name_pattern)
{
	int hash = StringHelper::Hash(name_pattern.c_str());
	UIFrame original_frame;

	if (m_hashed_frames.Get(hash, original_frame))
	{
		*this = original_frame;
		return;
	}

	int hash_mark = name_pattern.find('#');
	DBG_ASSERT_STR(hash_mark != std::string::npos, "Could not find replaceable hash mark in name pattern '%s'.", name_pattern.c_str());

	std::string split_left  = name_pattern.substr(0, hash_mark);
	std::string split_right = name_pattern.substr(hash_mark + 1);

	for (int i = 0; i < 9; i++)
	{
		std::string name = split_left + StringHelper::To_String(i) + split_right;
		m_frames[i] = ResourceFactory::Get()->Get_Atlas_Frame(name.c_str());
		DBG_ASSERT_STR(m_frames[i] != NULL, "Could not find expected atlas frame '%s'.", name.c_str());
	}

	m_hashed_frames.Set(hash, *this);
}

void UIFrame::Draw_Frame(AtlasRenderer& renderer, Rect2D rect, Vector2 scale, Color color)
{
	renderer.Begin_Batch();

	float size_x = m_frames[0]->Rect.Width * scale.X;
	float size_y = m_frames[0]->Rect.Height * scale.Y;

	// Deal with 3 frame sizes being larger than rect, eg when showing prog bar with 1% on it.
	if (size_x * 3 > rect.Width)
	{
		float s = rect.Width / (size_x * 3);
		size_x *= s;
	}

	// Center
	renderer.Draw_Frame(m_frames[4], Rect2D
	(
		rect.X + size_x, 
		rect.Y + size_y, 
		rect.Width - size_x - size_x, 
		rect.Height - size_y - size_y
	),
	0.0f,
	color);
	
	// Top-Left
	renderer.Draw_Frame(m_frames[0], Rect2D
	(
		rect.X, 
		rect.Y, 
		size_x, 
		size_y
	),
	0.0f,
	color);

	// Top-Middle
	renderer.Draw_Frame(m_frames[1], Rect2D
	(
		rect.X + size_x, 
		rect.Y, 
		rect.Width - size_x - size_x, 
		size_y
	),
	0.0f,
	color);

	// Top-Right
	renderer.Draw_Frame(m_frames[2], Rect2D
	(
		rect.X + rect.Width - size_x, 
		rect.Y, 
		size_x, 
		size_y
	),
	0.0f,
	color);

	// Left
	renderer.Draw_Frame(m_frames[3], Rect2D
	(
		rect.X, 
		rect.Y + size_y, 
		size_x, 
		rect.Height - size_y - size_y
	),
	0.0f,
	color);

	// Right
	renderer.Draw_Frame(m_frames[5], Rect2D
	(
		rect.X + rect.Width - size_x, 
		rect.Y + size_y, 
		size_x, 
		rect.Height - size_y - size_y
	),
	0.0f,
	color);
	
	// Bottom-Left
	renderer.Draw_Frame(m_frames[6], Rect2D
	(
		rect.X, 
		rect.Y + rect.Height - size_y, 
		size_x, 
		size_y
	),
	0.0f,
	color);
	
	// Bottom-Middle
	renderer.Draw_Frame(m_frames[7], Rect2D
	(
		rect.X + size_x, 
		rect.Y + rect.Height - size_y, 
		rect.Width - size_x - size_x, 
		size_y
	),
	0.0f,
	color);

	// Bottom-Right
	renderer.Draw_Frame(m_frames[8], Rect2D
	(
		rect.X + rect.Width - size_x, 
		rect.Y + rect.Height - size_y, 
		size_x, 
		size_y
	),
	0.0f,
	color);

	renderer.End_Batch();
}

void UIFrame::Draw_Frame_Flat_Top(AtlasRenderer& renderer, Rect2D rect, Vector2 scale, Color color)
{
	renderer.Begin_Batch();

	float size_x = m_frames[0]->Rect.Width * scale.X;
	float size_y = m_frames[0]->Rect.Height * scale.Y;

	// Center
	renderer.Draw_Frame(m_frames[4], Rect2D
	(
		rect.X + size_x, 
		rect.Y, 
		rect.Width - size_x - size_x, 
		rect.Height - size_y
	),
	0.0f,
	color);

	// Left
	renderer.Draw_Frame(m_frames[3], Rect2D
	(
		rect.X, 
		rect.Y, 
		size_x, 
		rect.Height - size_y
	),
	0.0f,
	color);

	// Right
	renderer.Draw_Frame(m_frames[5], Rect2D
	(
		rect.X + rect.Width - size_x, 
		rect.Y, 
		size_x, 
		rect.Height - size_y
	),
	0.0f,
	color);
	
	// Bottom-Left
	renderer.Draw_Frame(m_frames[6], Rect2D
	(
		rect.X, 
		rect.Y + rect.Height - size_y, 
		size_x, 
		size_y
	),
	0.0f,
	color);
	
	// Bottom-Middle
	renderer.Draw_Frame(m_frames[7], Rect2D
	(
		rect.X + size_x, 
		rect.Y + rect.Height - size_y, 
		rect.Width - size_x - size_x, 
		size_y
	),
	0.0f,
	color);

	// Bottom-Right
	renderer.Draw_Frame(m_frames[8], Rect2D
	(
		rect.X + rect.Width - size_x, 
		rect.Y + rect.Height - size_y, 
		size_x, 
		size_y
	),
	0.0f,
	color);

	renderer.End_Batch();
}

void UIFrame::Draw_Frame_Flat_Bottom(AtlasRenderer& renderer, Rect2D rect, Vector2 scale, Color color)
{
	renderer.Begin_Batch();

	float size_x = m_frames[0]->Rect.Width * scale.X;
	float size_y = m_frames[0]->Rect.Height * scale.Y;

	// Center
	renderer.Draw_Frame(m_frames[4], Rect2D
	(
		rect.X + size_x, 
		rect.Y + size_y, 
		rect.Width - size_x - size_x, 
		rect.Height - size_y 
	),
	0.0f,
	color);
	
	// Top-Left
	renderer.Draw_Frame(m_frames[0], Rect2D
	(
		rect.X, 
		rect.Y, 
		size_x, 
		size_y
	),
	0.0f,
	color);

	// Top-Middle
	renderer.Draw_Frame(m_frames[1], Rect2D
	(
		rect.X + size_x, 
		rect.Y, 
		rect.Width - size_x - size_x, 
		size_y
	),
	0.0f,
	color);

	// Top-Right
	renderer.Draw_Frame(m_frames[2], Rect2D
	(
		rect.X + rect.Width - size_x, 
		rect.Y, 
		size_x, 
		size_y
	),
	0.0f,
	color);

	// Left
	renderer.Draw_Frame(m_frames[3], Rect2D
	(
		rect.X, 
		rect.Y + size_y, 
		size_x, 
		rect.Height - size_y 
	),
	0.0f,
	color);

	// Right
	renderer.Draw_Frame(m_frames[5], Rect2D
	(
		rect.X + rect.Width - size_x, 
		rect.Y + size_y, 
		size_x, 
		rect.Height - size_y 
	),
	0.0f,
	color);

	renderer.End_Batch();
}