#version 120
// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

uniform sampler2D g_texture;
uniform bool g_texture_enabled;

uniform bool g_tiled;
uniform vec4 g_tiled_uv_limits;		// x y width height
uniform vec3 g_tiled_repeats;

uniform vec4 g_object_mask;

varying vec4 g_vertex_color;

void main()
{	
	if (g_texture_enabled == true)
	{
		vec2 uv = gl_TexCoord[0].st;

		if (g_tiled == true)
		{
			uv -= g_tiled_uv_limits.xy;							
			uv *= g_tiled_repeats.xy;							
			uv.xy = mod(uv.xy, g_tiled_uv_limits.zw);			
			uv += g_tiled_uv_limits.xy;										
		}
	 
		vec4 color  = texture2D(g_texture, uv);
		vec4 out_color = color * g_vertex_color;

		gl_FragData[0] = out_color;
		gl_FragData[1] = g_object_mask;
	}
	else
	{
		gl_FragData[0] = g_vertex_color;	
		gl_FragData[1] = g_object_mask;
	}	
}
