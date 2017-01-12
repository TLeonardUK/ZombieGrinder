// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_MATH_RECTANGLEPACKER_
#define _GENERIC_MATH_RECTANGLEPACKER_

#include "Generic/Types/Rect2D.h"
#include "Generic/Types/Vector2.h"

#include <vector>

#define OPT_FASTER_RECT_PACKER

class RectanglePacker
{
	MEMORY_ALLOCATOR(RectanglePacker, "Generic");

private:
	int m_width;
	int m_height;
	int m_spacing;

	int m_max_row_height;
	int m_offset_x;
	int m_offset_y;

	std::vector<Rect2D> m_packed_rects;

#ifdef OPT_FASTER_RECT_PACKER
	std::vector<Rect2D> m_free_rects;
#endif

public:
	RectanglePacker();
	RectanglePacker(int width, int height);
	RectanglePacker(int width, int height, int spacing);

	bool Is_Space_Free(Rect2D other, int& x_end, int& y_end);
	bool Pack(Vector2 size, Rect2D& location);

};

#endif