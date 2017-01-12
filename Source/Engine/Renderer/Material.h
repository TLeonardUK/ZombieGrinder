// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_MATERIAL_
#define _ENGINE_MATERIAL_

#include "Generic/Types/Vector3.h"

class Texture;
class TextureHandle;

// Defines the properties of a material being taken as an input to the renderer.
class Material
{
	MEMORY_ALLOCATOR(Material, "Rendering");

private:
	TextureHandle*	m_texture;
	float			m_shininess;
	Vector3			m_specular;

public:
	Material(TextureHandle* texture);
	Material(TextureHandle* texture, float shininess);
	Material(TextureHandle* texture, float shininess, Vector3 specular);

	TextureHandle* Get_Texture();
	void Set_Texture(TextureHandle* texture);

	float Get_Shininess();
	void Set_Shininess(float shininess);

	Vector3 Get_Specular();
	void Set_Specular(Vector3 specular);

};

#endif

