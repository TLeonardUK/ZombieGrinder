// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------

[
	Name("SpriteComponent"), 
	Description("When attached to an actor it displays a sprite relative to the actors location.") 
]
public native("SpriteComponent") class SpriteComponent
{ 
	// Creates a new sprite component and binds it to the executing actor.
	// Shouldn't be called directly, use a Components {} block.
	public native("Create") SpriteComponent();
	
	// Gets/sets if the sprite is rendering.
	property serialized(1) bool Visible
	{
		public native("Set_Visible") void Set(bool val);
		public native("Get_Visible") bool Get();
	}
	
	// Gets/sets if the sprite is rendering in editor.
	property serialized(1) bool Editor_Visible
	{
		public native("Set_Editor_Visible") void Set(bool val);
		public native("Get_Editor_Visible") bool Get();
	}
	
	// Gets/sets if the sprite is scaled to fit.
	property serialized(3) bool Scale
	{
		public native("Set_Scale") void Set(bool val);
		public native("Get_Scale") bool Get();
	}
	
	// Gets/sets the sprites actual rendering frame!
	[ EditType("Image") ]
	property serialized(1) string Frame
	{
		public native("Set_Frame") void   Set(string val);
		public native("Get_Frame") string Get();  
	}
	
	// Gets/sets the sprites flip state.
	property serialized(1) bool Flip_Horizontal
	{
		public native("Set_Flip_Horizontal") void Set(bool val);
		public native("Get_Flip_Horizontal") bool Get();
	}
	
	property serialized(1) bool Flip_Vertical
	{
		public native("Set_Flip_Vertical") void Set(bool val);
		public native("Get_Flip_Vertical") bool Get();
	}
	
	property serialized(110) bool Sort_By_Transparency
	{
		public native("Set_Sort_By_Transparency") void Set(bool val);
		public native("Get_Sort_By_Transparency") bool Get();
	}
	
	// Gets/sets the sprites object-mask used in post-process shaders for fancy effects.
	property serialized(900) Vec4 Object_Mask
	{
		public native("Set_Object_Mask") 	void Set(Vec4 val);
		public native("Get_Object_Mask") 	Vec4 Get();
	}

	// Gets/sets the sprites color tint.
	property serialized(1) Vec4 Color
	{
		public native("Set_Color") 	void Set(Vec4 val);
		public native("Get_Color") 	Vec4 Get();
	}

	// Gets/sets the sprites color tint multiplier.
	property Vec4 Color_Scalar
	{
		public native("Set_Color_Scalar") 	void Set(Vec4 val);
		public native("Get_Color_Scalar") 	Vec4 Get();
	}  

	// Gets/sets the area of the parents bounding box this sprite is rendered within. 
	// If not set it will use its parents full bounding box.
	property serialized(5) Vec4 Area
	{
		public native("Set_Area") 	void Set(Vec4 val);
		public native("Get_Area") 	Vec4 Get();
	}

	// Gets or sets the drawing offset.
	property serialized(20) Vec2 Offset
	{
		public native("Set_Offset") void Set(Vec2 val);
		public native("Get_Offset") Vec2 Get();
	}
	
	// Gets or sets the drawing scale.
	property serialized(40) Vec2 Draw_Scale
	{
		public native("Set_Draw_Scale") void Set(Vec2 val);
		public native("Get_Draw_Scale") Vec2 Get();
	}
	
	// Gets/sets the sprites depth bias.
	property serialized(1) float Depth_Bias
	{
		public native("Set_Depth_Bias") void Set(float val);
		public native("Get_Depth_Bias") float Get();
	}
	
	// Gets/sets the sprite animation.
	[ EditType("Animation") ]
	property serialized(2) string Animation  
	{
		public native("Set_Animation") void   Set(string val);
		public native("Get_Animation") string Get();
	}
	
	// Gets/sets the sprite animation speed
	property serialized(2) float Animation_Speed
	{
		public native("Set_Animation_Speed") void   Set(float val);
		public native("Get_Animation_Speed") float  Get();
	}
	
	property serialized(800) bool Allow_Static
	{
		public native("Set_Allow_Static") void Set(bool val);
		public native("Get_Allow_Static") bool Get();
	}

	property bool Is_Oneshot_Animation_Finished
	{
		public native("Is_Oneshot_Animation_Finished") bool Get();
	}

	public native("Play_Oneshot_Animation") void Play_Oneshot_Animation(string anim, float speed);
}