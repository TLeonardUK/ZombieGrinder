#version 120
// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

uniform mat4 g_world_view_projection_matrix;
uniform vec3 g_res;

varying vec2 texCoord;
varying vec4 t1;
varying vec4 t2;
varying vec4 t3;
varying vec4 t4;

void main()
{
	gl_Position = g_world_view_projection_matrix * gl_Vertex;

	vec2 tc = gl_MultiTexCoord0.xy;
	
	float x = (0.5 / g_res.x);
	float y = (0.5 / g_res.y);
	vec2 dg1 = vec2( x, y);
	vec2 dg2 = vec2(-x, y);
	vec2 dx = vec2(x, 0.0);
	vec2 dy = vec2(0.0, y);

	texCoord = tc;
	t1 = vec4(tc-dg1, tc-dy);
	t2 = vec4(tc-dg2, tc+dx);
	t3 = vec4(tc+dg1, tc+dy);
	t4 = vec4(tc+dg2, tc-dx);
}
