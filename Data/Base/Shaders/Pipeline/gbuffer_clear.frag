#version 120
// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

// gl_FragData[0] = Diffuse
// gl_FragData[1] = Distort
// gl_FragData[2] = Lighting
// gl_FragData[3] = Object-Mask

uniform vec4 		g_clear_color;

void main()
{
	gl_FragData[0] = g_clear_color;
	gl_FragData[1] = vec4(0.5, 0.5, 0.0, 0.0);
	gl_FragData[2] = vec4(0.0, 0.0, 0.0, 1.0);
	gl_FragData[3] = vec4(0.0, 0.0, 0.0, 0.0);
}
