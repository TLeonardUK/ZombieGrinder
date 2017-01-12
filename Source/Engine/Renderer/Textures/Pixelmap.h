// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_TEXTURES_Pixelmap_
#define _ENGINE_RENDERER_TEXTURES_Pixelmap_

#include "Generic/Types/Color.h"
#include "Generic/Types/Rect2D.h"
#include "Engine/Renderer/Textures/TextureFormat.h"
#include "Engine/Renderer/Textures/PixelmapFormat.h"

#include "Engine/Resources/Compiled/CompiledPixelmap.h"

class Pixelmap;

// Used to represent a specific rectangle of a Pixelmap. Only valid for
// as long as the source Pixelmap exists.
struct PixelmapWindow
{
	MEMORY_ALLOCATOR(PixelmapWindow, "Rendering");

private:
	Pixelmap* PixelmapPtr;
	Rect2D	Area;
	bool	InvertX;
	bool	InvertY;

	friend class Pixelmap;

public:
	Color Get_Pixel(int x, int y);

	Rect2D Calculate_Bounds();

	int Get_Width()  { return (int)Area.Width; }
	int Get_Height() { return (int)Area.Height; }

};

// Pixelmaps are simply "maps" of pixels. They simply contain a simple 
// interface for storing and manipulating pixel data directly. They are
// primarily used as an intermediary format between loading texture data
// from files and converting the data into hardware textures.
class Pixelmap
{
	MEMORY_ALLOCATOR(Pixelmap, "Rendering");

private:
	unsigned char* 
		m_data;

	int   
		m_width,
		m_height,
		m_pitch,
		m_bpp,
		m_data_size;

	PixelmapFormat::Type
		m_format;

	TextureFormat::Type
		m_texture_format;

	bool
		m_owns_data;

	Color
		m_mask_color;

public:
	Pixelmap(CompiledPixelmap* config);

	Pixelmap(int width, int height, PixelmapFormat::Type format);

	// Data used to intialize Pixelmaps is from that point on "owned" by the Pixelmap and will
	// be disposed of when the Pixelmap is.
	Pixelmap(unsigned char* data, int width, int height, int pitch, PixelmapFormat::Type format, bool owns_data = true);	

	~Pixelmap();

	void Free_Data();

	unsigned char* Get_Data();
	int Get_Data_Size();
	int Get_Width();
	int Get_Pitch();
	int Get_Height();
	PixelmapFormat::Type Get_Format();
	TextureFormat::Type Get_Texture_Format();

	void  Clear(Color c);

	void  Set_Pixel(int x, int y, Color c);
	Color Get_Pixel(int x, int y);

	PixelmapWindow Window(Rect2D rect, bool invert_x = false, bool invert_y = false);
	void Paste(Vector2 position, PixelmapWindow rect, int smude_amount = 0);
	void PasteColorBlended(Color tint, Vector2 position, PixelmapWindow rect, int smude_amount = 0);	
	void PasteScaled(Vector2 scale, Vector2 position, PixelmapWindow rect, int smude_amount = 0);	

	void Flip_Vertical();

};

#endif