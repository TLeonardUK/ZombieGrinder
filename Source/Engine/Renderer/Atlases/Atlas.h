// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_ATLASES_ATLAS_
#define _ENGINE_RENDERER_ATLASES_ATLAS_

#include "Generic/Types/HashTable.h"
#include "Generic/Types/LinkedList.h"
#include "Generic/Types/Rect2D.h"
#include "Generic/Types/Vector2.h"
#include "Generic/Math/RectanglePacker.h"

#include "Engine/Config/ConfigFile.h"

#include "Engine/Scene/Animation.h"

#include "Engine/Resources/Compiled/Atlases/CompiledAtlas.h"

class Texture;
class Pixelmap;
class Atlas;
class Rect2D;

struct AtlasTexture
{
	MEMORY_ALLOCATOR(AtlasTexture, "Rendering");

public:
	Texture*		TexturePtr;
	Pixelmap*		PixelmapPtr;
	bool			Is_Dirty;
	RectanglePacker	Packer;
};

struct AtlasMultiLayerFrame
{
	int			FrameOffset;
	int			LayerOffset;
	AtlasFrame* Frame; 
};

struct AtlasFrame
{
	MEMORY_ALLOCATOR(AtlasFrame, "Rendering");
	
public:
	std::string							Name;
	unsigned int						NameHash;
	Rect2D								UV;
	Rect2D								Rect;
	AtlasTexture*						TexturePtr;
	int									TextureIndex;
	Vector2								Origin;
	float								DepthBias;
	bool								HasSemiTransparentPixels;
	int									Index;
	Atlas*								AtlasPtr;
	Rect2D								GridOrigin;
	Rect2D								PixelBounds;
	std::vector<AtlasMultiLayerFrame>	MultiLayerFrames;
};

struct AtlasAnimation
{
	MEMORY_ALLOCATOR(AtlasAnimation, "Rendering");
	
public:
	std::string					Name;
	unsigned int				NameHash;
	AnimationMode::Type			Mode;
	float						Speed;
	std::vector<AtlasFrame*>	Frames;
	Atlas*						AtlasPtr;
};

struct CompiledAtlasData
{
	MEMORY_ALLOCATOR(CompiledAtlasData, "Rendering");

public:
	int texture_size;

};

class Atlas
{
	MEMORY_ALLOCATOR(Atlas, "Rendering");

private:
	int m_texture_size;
	int m_max_textures;

	std::string									m_name;
	
	AtlasTexture**								m_textures;
	HashTable<AtlasFrame*, unsigned int>		m_frames;
	std::vector<AtlasFrame*>					m_frames_list;
	HashTable<AtlasAnimation*, unsigned int>	m_animations;
	std::vector<AtlasAnimation*>				m_animations_list;
	HashTable<AtlasAnimation*, int>				m_animations_by_start_frame;	
	int											m_current_dirty_texture_index;
	bool										m_textures_locked;

	void*										m_resource_data;

public:
//	friend class AtlasResourceCompiler;
//	friend class ResourceFactory;

	// Methods to construct atlases.
	void			Lock_Textures();
	AtlasTexture*	Get_Texture(int index);
	AtlasTexture**	Get_Textures(int& texture_count);
	void			Unlock_Textures();

	void Add_Frame	  (const char* name, Pixelmap* texture, Rect2D texture_window, Vector2 origin, float depth_bias, Rect2D grid_origin = Rect2D(0, 0, 0, 0));
	void Add_Animation(const char* name, float speed, AnimationMode::Type mode, std::vector<AtlasFrame*> frames);

	void Set_Name	  (const char* name);

	bool Load_Compiled_Config(CompiledAtlasHeader* config);

public:

	// Destructor!
	Atlas();
	Atlas(ConfigFile* config);
	~Atlas();

	// Retrieving frames.
	std::string	Get_Name();
	AtlasFrame* Get_Frame(const char* name);
	AtlasFrame* Get_Frame(unsigned int name_hash);
	AtlasFrame* Get_Frame_By_Index(int index);
	int			Get_Frame_Count();
	
	int Get_Frame_Index(AtlasFrame* frame);
	AtlasAnimation* Get_Animation_By_Start_Frame(int frame);
	
	HashTable<AtlasFrame*, unsigned int>&	  Get_Frames			();
	std::vector<AtlasFrame*>&				  Get_Frames_List		();
	HashTable<AtlasAnimation*, unsigned int>& Get_Animations		();
	std::vector<AtlasAnimation*>&			  Get_Animations_List	();

};

#endif

