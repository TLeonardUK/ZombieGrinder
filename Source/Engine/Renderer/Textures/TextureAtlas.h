// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_TEXTURES_TEXTUREATLAS_
#define _ENGINE_RENDERER_TEXTURES_TEXTUREATLAS_

class Texture;
class TextureHandle;

class TextureAtlas
{
	MEMORY_ALLOCATOR(TextureAtlas, "Rendering");

private:
	TextureHandle*			m_texture;

	int					m_cell_count_u;
	int					m_cell_count_v;

	float*				m_cell_uvs;
	int					m_cell_uv_count;

	float				m_cell_width;
	float				m_cell_height;

public:
	
	// Constructor!
	TextureAtlas(TextureHandle* texture, int cells_u, int cells_v);

	// Destructor!
	~TextureAtlas();

	// Members that have to be overidden.
	void Get_UV_For_Index(int index, float& left, float& top, float& right, float& bottom);
	
};

#endif