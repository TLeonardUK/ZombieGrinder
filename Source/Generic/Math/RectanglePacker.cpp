// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Generic/Math/RectanglePacker.h"
#include "Generic/Math/Math.h"

RectanglePacker::RectanglePacker()
{
}

RectanglePacker::RectanglePacker(int width, int height)
	: m_width(width)
	, m_height(height)
	, m_spacing(2)
	, m_offset_x(m_spacing)
	, m_offset_y(m_spacing)
	, m_max_row_height(0)
{
#ifdef OPT_FASTER_RECT_PACKER
	m_free_rects.push_back(Rect2D(m_spacing, m_spacing, m_width - (m_spacing), m_height - (m_spacing)));
#endif
}

RectanglePacker::RectanglePacker(int width, int height, int spacing)
	: m_width(width)
	, m_height(height)
	, m_max_row_height(0)
	, m_spacing(spacing)
	, m_offset_x(m_spacing)
	, m_offset_y(m_spacing)
{
#ifdef OPT_FASTER_RECT_PACKER
	m_free_rects.push_back(Rect2D(m_spacing, m_spacing, m_width - (m_spacing), m_height - (m_spacing)));
#endif
}

bool RectanglePacker::Is_Space_Free(Rect2D other, int& x_end, int& y_end)
{
	for (std::vector<Rect2D>::iterator iter = m_packed_rects.begin(); iter != m_packed_rects.end(); iter++)
	{
		Rect2D& rect = *iter;
		if (rect.Intersects(other))
		{
			x_end = (int)(rect.X + rect.Width);
			y_end = (int)(rect.Y + rect.Height);
			return false;
		}
	}
	return true;
}

bool RectanglePacker::Pack(Vector2 size, Rect2D& output)
{
#ifdef OPT_FASTER_RECT_PACKER

	// Do any of the free rects have enough space to pack this.

	float targetSizeX = size.X + (m_spacing * 2.0f);
	float targetSizeY = size.Y + (m_spacing * 2.0f);

	//float minimumSplitExcess = (m_spacing * 2.0f) + 5.0f;
	float minimumSplitExcess = 0.0f;

	for (unsigned int i = 0; i < m_free_rects.size(); i++)
	{
		Rect2D rect = m_free_rects[i];

		if (rect.Width >= targetSizeX &&
			rect.Height >= targetSizeY)
		{
			float excessX = rect.Width - targetSizeX;
			float excessY = rect.Height - targetSizeY;

			// Choose split direction based on which split direction
			// will produce the largets minimum axis.

			// ------|-----|
			//       |     |
			// ------|     |
			//       |     |
			//       |     |
			//       |     |
			// ------|-----|
			 if (excessX > excessY)
			 {
				 // Split off the right hand chunk into its own free rect.
				 if (excessX > minimumSplitExcess)
				 {
					 rect.Width = targetSizeX;

					 Rect2D split(rect.X + targetSizeX, rect.Y, excessX, rect.Height);
					 m_free_rects.push_back(split);
				 }

				 // Split off the bottom chunk into its own free rect.
				 if (excessY > minimumSplitExcess)
				 {
					 rect.Height = targetSizeY;

					 Rect2D split(rect.X, rect.Y + targetSizeY, rect.Width, excessY);
					 m_free_rects.push_back(split);
				 }
			 }

			// ------|---------|
			//       |         |
			// ----------------|
			//                 |
			// ----------------|
			 else
			 {
				 // Split off the bottom chunk into its own free rect.
				 if (excessY > minimumSplitExcess)
				 {
					 rect.Height = targetSizeY;

					 Rect2D split(rect.X, rect.Y + targetSizeY, rect.Width, excessY);
					 m_free_rects.push_back(split);
				 }

				 // Split off the right hand chunk into its own free rect.
				 if (excessX > minimumSplitExcess)
				 {
					 rect.Width = targetSizeX;

					 Rect2D split(rect.X + targetSizeX, rect.Y, excessX, rect.Height);
					 m_free_rects.push_back(split);
				 }
			 }

			// Split this rect into smaller pieces if we have enough free space bordering the part we are going to take.
			m_free_rects.erase(m_free_rects.begin() + i);

			// Return the center of this rect.
			output.X = (rect.X + (rect.Width * 0.5f)) - (size.X * 0.5f);
			output.Y = (rect.Y + (rect.Height * 0.5f)) - (size.Y * 0.5f);
			output.Width = size.X;
			output.Height = size.Y;

			return true;
		}
	}

#else
	int scan_height = (m_height - (m_spacing * 2)) - (int)size.Y;
	int scan_width = (m_width - (m_spacing * 2)) - (int)size.X;

	// Brute force crappy ass packing algorithm >_>
	for (int y = m_spacing; y < scan_height; y++)
	{
		int min_y = 0;

		for (int x = m_spacing; x < scan_width; x++)
		{
			Rect2D padded_rect = Rect2D(
				(float)x - m_spacing, 
				(float)y - m_spacing, 
				(float)size.X + (m_spacing * 2), 
				(float)size.Y + (m_spacing * 2)
			);

			int x_end = 0;
			int y_end = 0;
			if (Is_Space_Free(padded_rect, x_end, y_end))
			{
				output = Rect2D((float)x, (float)y, size.X, size.Y);
				m_packed_rects.push_back(output);
				return true;
			}
			else
			{
				x = x_end + (int)m_spacing;
				min_y = Min(min_y, y_end);
			}
		}

		if (min_y != 0)
			y = min_y;
	}

	/*
	// Can we just plop it on the next space on the current row?
	if (m_offset_x + size.X + m_spacing < m_width &&
		m_offset_y + size.Y + m_spacing < m_height)
	{
		output.X		= (float)m_offset_x;
		output.Y		= (float)m_offset_y;
		output.Width	= size.X;
		output.Height	= size.Y;
		
		m_offset_x += (int)(output.Width + m_spacing);

		if (output.Height > m_max_row_height)
		{
			m_max_row_height = (int)output.Height;
		}

		return true;
	}

	// Ok, move to next row!
	m_offset_x = m_spacing;
	m_offset_y += m_max_row_height + m_spacing;
	m_max_row_height = (int)size.Y;

	if (m_offset_x + size.X + m_spacing < m_width &&
		m_offset_y + size.Y + m_spacing < m_height)
	{
		output.X		= (float)m_offset_x;
		output.Y		= (float)m_offset_y;
		output.Width	= size.X;
		output.Height	= size.Y;
				
		m_offset_x += (int)(output.Width + m_spacing);

		if (output.Height > m_max_row_height)
		{
			m_max_row_height = (int)output.Height;
		}

		return true;
	}
	*/
#endif

	// Gay, no space :(
	return false;
}
