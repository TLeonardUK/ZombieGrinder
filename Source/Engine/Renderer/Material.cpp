// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Renderer/Material.h"
#include "Engine/Renderer/Textures/Texture.h"
#include "Engine/Renderer/Textures/TextureHandle.h"

Material::Material(TextureHandle* texture)
	: m_texture(texture)
	, m_shininess(0)
	, m_specular(0, 0, 0)
{
}

Material::Material(TextureHandle* texture, float shininess)
	: m_texture(texture)
	, m_shininess(shininess)
	, m_specular(0, 0, 0)
{
}

Material::Material(TextureHandle* texture, float shininess, Vector3 specular)
	: m_texture(texture)
	, m_shininess(shininess)
	, m_specular(specular)
{
}

TextureHandle* Material::Get_Texture()
{
	return m_texture;
}

void Material::Set_Texture(TextureHandle* texture)
{
	m_texture = texture;
}

float Material::Get_Shininess()
{
	return m_shininess;
}

void Material::Set_Shininess(float shininess)
{
	m_shininess = shininess;
}

Vector3 Material::Get_Specular()
{
	return m_specular;
}

void Material::Set_Specular(Vector3 specular)
{
	m_specular = specular;
}
