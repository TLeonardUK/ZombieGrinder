#version 120
// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

uniform mat4 g_world_view_projection_matrix;
varying vec4 g_vertex_color;

void main()
{
	gl_Position    = g_world_view_projection_matrix  * gl_Vertex; 
	gl_TexCoord[0] = gl_MultiTexCoord0;
	g_vertex_color = gl_Color;
}