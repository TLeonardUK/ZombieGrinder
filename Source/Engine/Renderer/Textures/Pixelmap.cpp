// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Renderer/Textures/Pixelmap.h"

#include "Generic/Math/Math.h"

#include <cstring>

int BPPForPixelmapFormat[5] = 
{
	1,
	4,
	3,
	1,
	1
};

TextureFormat::Type TextureFormatForPixelmapFormat[5] = 
{
	TextureFormat::Luminosity,
	TextureFormat::R8G8B8A8,
	TextureFormat::R8G8B8,
	TextureFormat::DXT3,
	TextureFormat::DXT5
};

Color PixelmapWindow::Get_Pixel(int x, int y)
{
	float ox = Area.X + x;
	float oy = Area.Y + y;

	if (InvertX == true)
	{
		ox = Area.X + (Area.Width - (x + 1));
	}

	if (InvertY == true)
	{
		oy = Area.Y + (Area.Height - (y + 1));
	}

	return PixelmapPtr->Get_Pixel((int)ox, (int)oy);
}

Rect2D PixelmapWindow::Calculate_Bounds()
{
	Vector2 max = Vector2(-1, -1);
	Vector2 min = Vector2(0, 0);
	bool found = false;

	for (int x = 0; x < Area.Width; x++)
	{	
		for (int y = 0; y < Area.Height; y++)
		{
			Color p = Get_Pixel(x, y);
			if (p.A > 0)
			{
				if (x < min.X || found == false) min.X = (float)x;
				if (y < min.Y || found == false) min.Y = (float)y;
				if (x > max.X || found == false) max.X = (float)x;
				if (y > max.Y || found == false) max.Y = (float)y;
				found = true;
			}
		}
	}

	return Rect2D(min.X, min.Y, (max.X - min.X) + 1, (max.Y - min.Y) + 1);
}

Pixelmap::Pixelmap(int width, int height, PixelmapFormat::Type format)
	: m_width(width)
	, m_height(height)
	, m_format(format)
	, m_owns_data(true)
{
	m_bpp = BPPForPixelmapFormat[(int)format];
	m_texture_format = TextureFormatForPixelmapFormat[(int)format];
	m_data_size = m_width * m_height * m_bpp;
#ifndef DEDICATED_SERVER_BUILD
	m_data = new unsigned char[m_data_size];
#endif
	m_pitch = m_width * m_bpp;
}

Pixelmap::Pixelmap(CompiledPixelmap* config)
	: m_width(config->width)
	, m_height(config->height)
	, m_format(config->format)
	, m_data(NULL)
	, m_owns_data(true)
{
	m_bpp = BPPForPixelmapFormat[(int)m_format];
	m_texture_format = TextureFormatForPixelmapFormat[(int)m_format];
	m_pitch = m_width * m_bpp;
	
	m_data_size = m_width * m_height * m_bpp;
#ifndef DEDICATED_SERVER_BUILD
	m_data = new unsigned char[m_data_size];
	memcpy(m_data, config->data, m_data_size);
#endif
}

Pixelmap::Pixelmap(unsigned char* data, int width, int height, int pitch, PixelmapFormat::Type format, bool owns_data)
	: m_width(width)
	, m_height(height)
	, m_pitch(pitch)
	, m_format(format)
	, m_data(data)
	, m_owns_data(owns_data)
{
	m_bpp = BPPForPixelmapFormat[(int)format];
	m_texture_format = TextureFormatForPixelmapFormat[(int)format];
	m_data_size = m_width * m_height * m_bpp;
}

Pixelmap::~Pixelmap()
{
	Free_Data();
}

void Pixelmap::Free_Data()
{
	if (m_owns_data == true)
	{
		SAFE_DELETE_ARRAY(m_data);
	}
}

unsigned char* Pixelmap::Get_Data()
{
	return m_data;
}

int Pixelmap::Get_Data_Size()
{
	return m_data_size;
}

int Pixelmap::Get_Width()
{
	return m_width;
}

int Pixelmap::Get_Pitch()
{
	return m_pitch;
}

int Pixelmap::Get_Height()
{
	return m_height;
}

PixelmapFormat::Type Pixelmap::Get_Format()
{
	return m_format;
}

TextureFormat::Type Pixelmap::Get_Texture_Format()
{
	return m_texture_format;
}

void Pixelmap::Clear(Color c)
{
	DBG_ASSERT_STR(m_format != PixelmapFormat::DXT3 && m_format != PixelmapFormat::DXT5, "Cannot modify compiled pixelmap formats.");

	for (int x = 0; x < m_width; x++)
	{
		for (int y = 0; y < m_height; y++)
		{
			Set_Pixel(x, y, c);
		}
	}
}

void Pixelmap::Set_Pixel(int x, int y, Color c)
{
#ifndef DEDICATED_SERVER_BUILD

	DBG_ASSERT_STR(m_format != PixelmapFormat::DXT3 && m_format != PixelmapFormat::DXT5, "Cannot modify compiled pixelmap formats.");

	int offset = (m_pitch * (m_height - (y + 1))) + (x * m_bpp);

	DBG_ASSERT(offset >= 0 && offset <= m_data_size - m_bpp);

	if (m_bpp >= 1)
		m_data[offset] = c.R;
	if (m_bpp >= 2)
		m_data[offset + 1] = c.G;
	if (m_bpp >= 3)
		m_data[offset + 2] = c.B;
	if (m_bpp >= 4)
		m_data[offset + 3] = c.A;

#endif
}

Color Pixelmap::Get_Pixel(int x, int y)
{
#ifndef DEDICATED_SERVER_BUILD

	DBG_ASSERT_STR(m_format != PixelmapFormat::DXT3 && m_format != PixelmapFormat::DXT5, "Cannot modify compiled pixelmap formats.");

	Color result = m_mask_color;
	int offset = (m_pitch * (m_height - (y + 1))) + (x * m_bpp);

	DBG_ASSERT(offset >= 0 && offset <= m_data_size - m_bpp);

	if (m_bpp >= 1)
		result.R = m_data[offset];
	if (m_bpp >= 2)
		result.G = m_data[offset + 1];
	if (m_bpp >= 3)
		result.B = m_data[offset + 2];
	if (m_bpp >= 4)
		result.A = m_data[offset + 3];
	return result;

#else

	return Color::White;

#endif
}

PixelmapWindow Pixelmap::Window(Rect2D rect, bool invert_x, bool invert_y)
{
	PixelmapWindow window;
	window.PixelmapPtr = this;
	window.Area = rect;
	window.InvertX = invert_x;
	window.InvertY = invert_y;
	return window;
}

void Pixelmap::Paste(Vector2 position, PixelmapWindow rect, int smudge_amount)
{
	//DBG_ASSERT(position.X >= 0 && position.X + rect.Get_Width() <= m_width);
	//DBG_ASSERT(position.Y >= 0 && position.Y + rect.Get_Height() <= m_height);

	for (int x = -smudge_amount; x < rect.Get_Width() + smudge_amount; x++)
	{
		for (int y = -smudge_amount; y < rect.Get_Height() + smudge_amount; y++)
		{
			int dest_x = (int)(position.X + x);
			int dest_y = (int)(position.Y + y);

			if (smudge_amount == 0)
			{
				if (x < 0 || y < 0 || x >= rect.Get_Width() || y >= rect.Get_Height())
				{
					continue;
				}
				if (dest_x < 0 || dest_y < 0 || dest_x >= m_width || dest_y >= m_height)
				{
					continue;
				}
			}

			Color source = rect.Get_Pixel
			(
				Min(rect.Get_Width() - 1, Max(0, x)), 
				Min(rect.Get_Height() - 1, Max(0, y))
			);
			Set_Pixel(dest_x, dest_y, source);
		}
	}
}

void Pixelmap::PasteScaled(Vector2 scale, Vector2 position, PixelmapWindow rect, int smudge_amount)
{
	//DBG_ASSERT(position.X >= 0 && position.X + rect.Get_Width() <= m_width);
	//DBG_ASSERT(position.Y >= 0 && position.Y + rect.Get_Height() <= m_height);

	int real_width = (int)floorf(rect.Get_Width() * scale.X);
	int real_height = (int)floorf(rect.Get_Height() * scale.Y);

	for (int x = -smudge_amount; x < real_width + smudge_amount; x++)
	{
		for (int y = -smudge_amount; y < real_height + smudge_amount; y++)
		{
			int dest_x = (int)(position.X + x);
			int dest_y = (int)(position.Y + y);

			if (smudge_amount == 0)
			{
				if (x < 0 || y < 0 || x >= real_width || y >= real_height)
				{
					continue;
				}
				if (dest_x < 0 || dest_y < 0 || dest_x >= m_width || dest_y >= m_height)
				{
					continue;
				}
			}

			Color source = rect.Get_Pixel
			(
				Min(rect.Get_Width() - 1, Max(0, (int)(x / scale.X))), 
				Min(rect.Get_Height() - 1, Max(0, (int)(y / scale.Y)))
			);

			Set_Pixel(dest_x, dest_y, source);
		}
	}
}

void Pixelmap::PasteColorBlended(Color tint, Vector2 position, PixelmapWindow rect, int smudge_amount)
{
//	DBG_ASSERT(position.X >= 0 && position.X + rect.Get_Width() <= m_width);
//	DBG_ASSERT(position.Y >= 0 && position.Y + rect.Get_Height() <= m_height);

	float factor_r = tint.R / 255.0f;
	float factor_g = tint.G / 255.0f;
	float factor_b = tint.B / 255.0f;
	float factor_a = tint.A / 255.0f;

	for (int x = -smudge_amount; x < rect.Get_Width() + smudge_amount; x++)
	{
		for (int y = -smudge_amount; y < rect.Get_Height() + smudge_amount; y++)
		{
			int dest_x = (int)(position.X + x);
			int dest_y = (int)(position.Y + y);

			if (smudge_amount == 0)
			{
				if (x < 0 || y < 0 || x >= rect.Get_Width() || y >= rect.Get_Height())
				{
					continue;
				}
				if (dest_x < 0 || dest_y < 0 || dest_x >= m_width || dest_y >= m_height)
				{
					continue;
				}
			}

			Color source = rect.Get_Pixel
			(
				Min(rect.Get_Width() - 1, Max(0, x)), 
				Min(rect.Get_Height() - 1, Max(0, y))
			);
			Color dest = Get_Pixel
			(
				(int)(position.X + x),
				(int)(position.Y + y)
			);

			source = Color
			(
				source.R * factor_r,
				source.G * factor_g,
				source.B * factor_b,
				source.A * factor_a
			);

			float d = source.A / 255.0f;
			float recp_d = 1.0f - d;
			Color mixed = Color
			(
				(dest.R * recp_d) + (source.R * d),
				(dest.G * recp_d) + (source.G * d),
				(dest.B * recp_d) + (source.B * d),
				(dest.A * recp_d) + (source.A * d)
			);

			Set_Pixel(dest_x, dest_y, mixed);
		}
	}
}
void Pixelmap::Flip_Vertical()
{	
#ifndef DEDICATED_SERVER_BUILD

	DBG_ASSERT_STR(m_format != PixelmapFormat::DXT3 && m_format != PixelmapFormat::DXT5, "Cannot modify compiled pixelmap formats.");

	int row_size = m_pitch * m_bpp;

	char* temp_data = new char[row_size];

	int half_rows = (int)floorf(m_height / 2.0f);

	for (int row = 0; row < half_rows; row++)
	{
		int source_row = m_height - (row + 1);
		if (source_row == row)
		{
			continue;
		}

		unsigned char* source_row_ptr = m_data + (row_size * source_row);
		unsigned char* row_ptr		  = m_data + (row_size * row);
		
		memcpy(temp_data, row_ptr, row_size);
		memcpy(row_ptr, source_row_ptr, row_size);
		memcpy(source_row_ptr, temp_data, row_size);
	}

	SAFE_DELETE_ARRAY(temp_data);

#endif
}