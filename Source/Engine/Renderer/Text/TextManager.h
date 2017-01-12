// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_TEXTMANAGER_
#define _ENGINE_RENDERER_TEXTMANAGER_

#include "Generic/Types/Vector3.h"
#include "Generic/Types/Point.h"
#include "Generic/Types/Rectangle.h"
#include "Generic/Types/HashTable.h"
#include "Generic/Patterns/Singleton.h"

class Texture;
class TextureHandle;

class TextManager
{
private:

public:
	TextManager();

	void Load_Font(const char* name);
	void Get_Font (const char* name);

	TextBuffer* Create_Text_Buffer	(const char* text, TextFont* font, Point size, TextModifier* modifier);
	void		Render_Text			(const char* text, TextFont* font, Rect position, TextModifier* modifier);
	void		Render_Text			(TextBuffer* buffer, Point position);

};

#endif

