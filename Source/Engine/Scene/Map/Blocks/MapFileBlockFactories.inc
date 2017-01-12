// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Scene/Map/Blocks/MapFileBlock.h"
#include "Engine/Scene/Map/Blocks/MapFileHeaderBlock.h"

// This little bastard of a file gets included in one of the early game-engine init
// source file. It forces the static vars below to be instantiated so we can
// access the diffferent block types!
//
// Pretty fucking sure there is a nicer way to do this. Fucking lazy instantiation.

// Base blocks.
DEFINE_MAP_BLOCK_FACTORY(MapFileHeaderBlock);
