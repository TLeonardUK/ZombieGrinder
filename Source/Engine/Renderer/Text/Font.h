// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_TEXT_FONT_
#define _ENGINE_RENDERER_TEXT_FONT_

#include "Generic/Types/Rect2D.h"
#include "Generic/Types/Vector2.h"

class Texture;

struct FontGlyph
{
	MEMORY_ALLOCATOR(FontGlyph, "Rendering");

public:
	unsigned int Glyph;
	Rect2D		 UV;
	Texture*	 TexturePtr;
	Vector2		 Size;
	float		 Advance;
};

struct FontFlags
{
	enum Type
	{
		NONE = 0
	};
};

class Font
{
	MEMORY_ALLOCATOR(Font, "Rendering");

private:
	
protected:
	friend class ResourceFactory;

	virtual void Set_Name(const char* name) = 0;

public:

	// Destructor!
	virtual ~Font();

	// Actual font stuff!
	virtual std::string Get_Name() = 0;

	virtual float Get_Base_Height() = 0;
	virtual float Get_Shadow_Scale() = 0;

	virtual FontGlyph Get_Glyph(unsigned int character) = 0;
	virtual Vector2	  Get_Kerning(unsigned int prev, unsigned int next) = 0;

};

#endif

