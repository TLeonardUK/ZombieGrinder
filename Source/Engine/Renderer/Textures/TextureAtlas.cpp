// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Renderer/Textures/TextureAtlas.h"
#include "Engine/Renderer/Textures/Texture.h"
#include "Engine/Renderer/Textures/TextureHandle.h"

TextureAtlas::TextureAtlas(TextureHandle* texture, int cells_u, int cells_v)
	: m_texture(texture)
	, m_cell_count_u(cells_u)
	, m_cell_count_v(cells_v)
	, m_cell_uv_count(cells_u * cells_v)
{
	float texture_width		= (float)m_texture->Get()->Get_Width();
	float texture_height	= (float)m_texture->Get()->Get_Height();

	m_cell_uvs				= new float[m_cell_uv_count * 4];

	m_cell_width			= (float)texture_width  / (float)m_cell_count_u;
	m_cell_height			= (float)texture_height / (float)m_cell_count_v;

	float cell_u_width		= m_cell_width / texture_width;
	float cell_v_height		= m_cell_height / texture_height;

	for (int i = 0, offset = 0; i < m_cell_uv_count; i++, offset += 4)
	{
		int u = i % m_cell_count_u;
		int v = i / m_cell_count_u;

		float u_offset = (u * m_cell_width) / texture_width;
		float v_offset = (v * m_cell_height) / texture_height;

		// Work out pixel positions.
		m_cell_uvs[offset + 0] = u_offset;
		m_cell_uvs[offset + 1] = 1.0f - v_offset;
		m_cell_uvs[offset + 2] = (u_offset + cell_u_width);
		m_cell_uvs[offset + 3] = 1.0f - (v_offset + cell_v_height);
	}
}

TextureAtlas::~TextureAtlas()
{
	SAFE_DELETE_ARRAY(m_cell_uvs);
}

void TextureAtlas::Get_UV_For_Index(int index, float& left, float& top, float& right, float& bottom)
{
	DBG_ASSERT(index >= 0 && index <= m_cell_uv_count);
	int offset = (index * 4); 

	left    = m_cell_uvs[offset + 0];
	top     = m_cell_uvs[offset + 1];
	right   = m_cell_uvs[offset + 2];
	bottom  = m_cell_uvs[offset + 3];
}
	