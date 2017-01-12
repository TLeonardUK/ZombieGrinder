// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

START_PACKET_ENUM(ModifyTilemapAction)
	PACKET_ENUM_ELEMENT(Pencil)		// Places tiles in a rectangle.
	PACKET_ENUM_ELEMENT(Eraser)		// Removes tiles in a rectangle.
	PACKET_ENUM_ELEMENT(Flood)		// Flood fills areas.
	PACKET_ENUM_ELEMENT(Paste)		// Pastes the given Tile array into a rectangle.
	PACKET_ENUM_ELEMENT(Circle)		// Draw a circle
END_PACKET_ENUM()

START_PACKET_ENUM(ModifyTilemapMode)
	PACKET_ENUM_ELEMENT(Tilemap)		
	PACKET_ENUM_ELEMENT(Collision)		
END_PACKET_ENUM()

START_PACKET(ModifyTilemapTileState, false)
	PACKET_FIELD(u8,	Collision);
	PACKET_FIELD(int,	Collision_Frame);
	PACKET_FIELD(int,	Frame);
	PACKET_FIELD(bool,	Flip_Horizontal);
	PACKET_FIELD(bool,	Flip_Vertical);
	PACKET_FIELD(Color, Tile_Color);
END_PACKET()

START_PACKET(NetPacket_C2S_ModifyTilemap, false)
	PACKET_ENUM_FIELD	(ModifyTilemapAction, Action);
	PACKET_ENUM_FIELD	(ModifyTilemapMode, Mode);

	PACKET_FIELD  		 (u8, Layer_Index);
	PACKET_FIELD		 (u8, Collision);
	PACKET_FIELD		 (bool, Flip_Horizontal);
	PACKET_FIELD		 (bool, Flip_Vertical);
	PACKET_FIELD		 (Color, Tile_Color);
	PACKET_FIELD		 (u16,	Radius);

	PACKET_FIELD		 (Rect2D, Tilemap_Selection);
	PACKET_FIELD		 (Rect2D, Tileset_Selection);

	PACKET_VECTOR_FIELD	 (ModifyTilemapTileState, Tiles);
END_PACKET()

/**
START_PACKET(NetPacket_C2S_ModifyTilemapAbsolute, false)

	PACKET_ENUM_FIELD(ModifyTilemapAction, Action);
	PACKET_ENUM_FIELD(ModifyTilemapMode, Mode);

	PACKET_FIELD	 (u8, Collision);

	PACKET_FIELD  	 (u8, Layer_Index);
	PACKET_FIELD	 (bool, Flip_Horizontal);
	PACKET_FIELD	 (bool, Flip_Vertical);
	PACKET_FIELD	 (Color, Tile_Color);

	PACKET_FIELD	 (Rect2D, Tilemap_Selection);
	PACKET_FIELD	 (Rect2D, Tileset_Selection);

END_PACKET()
*/