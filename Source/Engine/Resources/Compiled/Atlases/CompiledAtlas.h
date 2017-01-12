// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RESOURCES_COMPILED_ATLAS_
#define _ENGINE_RESOURCES_COMPILED_ATLAS_

#include "Engine/Resources/Compiled/CompiledPixelmap.h"
#include "Generic/Types/Rect2D.h"
#include "Generic/Types/Vector2.h"

struct CompiledAtlasMultiLayerFrame
{
	int frame_index;
	int frame_offset;
	int layer_offset;
};

struct CompiledAtlasFrame
{
	const char*						name;
	u32								name_hash;
	u32								texture_index;
	u32								index;
	u32								has_semitransparent_pixels;
	float							depth_bias;
	Rect2D							rect;
	Rect2D							uv;
	Vector2							origin;
	Rect2D							grid_origin;
	Rect2D							pixel_bounds;
	int								multi_layer_frame_count;
	CompiledAtlasMultiLayerFrame*	multi_layer_frames;
};

struct CompiledAtlasAnimation
{
	const char*				name;
	u32						name_hash;
	AnimationMode::Type		mode;
	float					speed;
	u32						frame_count;
	CompiledAtlasFrame**	frames;
};

struct CompiledAtlasHeader
{
	const char*				name;
	u32						name_hash;
	u32						texture_count;
	u32						frame_count;
	u32						animation_count;
	CompiledAtlasFrame*		frames;
	CompiledAtlasAnimation*	animations;
	CompiledPixelmap*		pixmaps;
};

#endif