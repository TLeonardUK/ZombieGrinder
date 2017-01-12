#version 120
// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

uniform sampler2D g_texture;
uniform sampler2D g_lut_texture;
uniform vec3 g_resolution;
uniform vec3 g_intensity;

void main()
{ 
	vec4 base = texture2D(g_texture, gl_TexCoord[0].xy);

	// Calculate green scale from LUT.
	float intensity = clamp((base.r + base.g + base.b) / 3.0f, 0.02, 0.98);		
	vec4 result = texture2D(g_lut_texture, vec2(intensity, 0.5));

	// Could do some extra pixelisation here, but this works pretty good.
	
	gl_FragColor = result;
}