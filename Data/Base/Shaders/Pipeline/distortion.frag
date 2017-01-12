#version 120
// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

uniform sampler2D g_texture;
varying vec4 g_vertex_color;

void main()
{
	vec4 color  = texture2D(g_texture, gl_TexCoord[0].st);
	vec4 out_color = color * g_vertex_color;

	gl_FragData[0] = out_color;
}
