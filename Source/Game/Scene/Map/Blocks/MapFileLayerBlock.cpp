// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Scene/Map/Blocks/MapFileLayerBlock.h"

#include "Engine/Resources/ResourceFactory.h"

#include "Engine/IO/Stream.h"

MapFileLayerBlock::MapFileLayerBlock()
	: Tiles(NULL)
{
}

MapFileLayerBlock::~MapFileLayerBlock()
{
	SAFE_DELETE(Tiles);
}

bool MapFileLayerBlock::Decode(MapVersion::Type version, Stream* stream)
{
	Width  = stream->Read<int>();
	Height = stream->Read<int>();

	SAFE_DELETE(Tiles);
	Tiles  = new MapTile[Width * Height];
	
	for (int y = 0; y < Height; y++)
	{
		for (int x = 0; x < Width; x++)
		{
			MapTile& tile = Tiles[(y * Width) + x];

			tile.Frame = stream->Read<u16>();
			tile.TileColor.R = stream->Read<u8>();
			tile.TileColor.G = stream->Read<u8>();
			tile.TileColor.B = stream->Read<u8>();
			tile.TileColor.A = stream->Read<u8>();
			tile.FlipHorizontal = stream->Read<bool>();
			tile.FlipVertical = stream->Read<bool>();
			tile.CollisionFrame = stream->Read<u16>();
			tile.Collision = (MapTileCollisionType::Type)stream->Read<int>();
		}
	}

	return true;
}

bool MapFileLayerBlock::Encode(MapVersion::Type version, Stream* stream)
{
	stream->Write<int>(Width);
	stream->Write<int>(Height);

	for (int y = 0; y < Height; y++)
	{
		for (int x = 0; x < Width; x++)
		{
			MapTile& tile = Tiles[(y * Width) + x];

			stream->Write<u16>(tile.Frame);
			stream->Write<u8>(tile.TileColor.R);
			stream->Write<u8>(tile.TileColor.G);
			stream->Write<u8>(tile.TileColor.B);
			stream->Write<u8>(tile.TileColor.A);
			stream->Write<bool>(tile.FlipHorizontal);
			stream->Write<bool>(tile.FlipVertical);
			stream->Write<u16>(tile.CollisionFrame);
			stream->Write<int>(tile.Collision);
		}
	}

	return true;
}

u32 MapFileLayerBlock::Get_ID()
{
	return COMBINE_MAP_FILE_BLOCK_ID('L', 'Y', 'E', 'R');
}

