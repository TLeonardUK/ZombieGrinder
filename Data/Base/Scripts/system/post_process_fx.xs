// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------

[
	Name("Post process FX"), 
	Description("Base class for post process effects. These are statically instantiated at the start of the game and applied whenever the active var is set.") 
]
public native("Post_Process_FX") class Post_Process_FX
{
	// If true then this effect will be applied.
	property bool Active
	{
		native("Get_Active") bool Get();
		native("Set_Active") void Set(bool value);			
	}

	// Sets the shader to use for rendering this effect.
	property string Shader
	{
		native("Get_Shader") string Get();
		native("Set_Shader") void Set(string value);			
	}
	
	// Gets the user's camera this post proccess is setup for, or null if players are sharing cameras
	property int Camera_Index
	{
		native("Get_Camera_Index") int Get();
	}

	// Get allocated instance of type.
	public native("Get_Instance") static Post_Process_FX Get_Instance(Type type, int camera_index);

	// Invoked when its time to run the post process shader. This is responsible
	// for passing any uniforms in that are required.
	public abstract void On_Shader_Setup();

	// Sets the value of a floating point shader uniform. Should only be called from
	// inside On_Shader_Setup.
	public native("Set_Shader_Uniform_Float") void Set_Shader_Uniform(string name, float value);	
	public native("Set_Shader_Uniform_Vec3")  void Set_Shader_Uniform(string name, Vec3 value);	
}
