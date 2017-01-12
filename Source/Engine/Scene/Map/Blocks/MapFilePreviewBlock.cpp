// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Scene/Map/Blocks/MapFilePreviewBlock.h"

#include "Engine/Resources/ResourceFactory.h"

#include "Engine/IO/Stream.h"

#include "Engine/Renderer/Textures/Pixelmap.h"
#include "Engine/Renderer/Textures/PixelmapFactory.h"

MapFilePreviewBlock::MapFilePreviewBlock()
	: Preview(NULL)
{
}

MapFilePreviewBlock::~MapFilePreviewBlock()
{
	SAFE_DELETE(Preview);
}

bool MapFilePreviewBlock::Decode(MapVersion::Type version, Stream* stream)
{
	if (stream->Read<bool>())
	{
		Preview = PixelmapFactory::Load(stream);
	}
	return true;
}

bool MapFilePreviewBlock::Encode(MapVersion::Type version, Stream* stream)
{
	stream->Write<bool>(Preview != NULL);
	if (Preview != NULL)
	{
		PixelmapFactory::Save(stream, Preview);
	}
	return true;
}

u32 MapFilePreviewBlock::Get_ID()
{
	return COMBINE_MAP_FILE_BLOCK_ID('P', 'R', 'V', 'W');
}

