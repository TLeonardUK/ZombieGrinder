#version 120
// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

uniform sampler2D g_texture;
uniform vec3 g_resolution;
uniform vec3 g_intensity;

vec2 saturate(vec2 inValue)
{
	return clamp(inValue, vec2(0.0), vec2(1.0));
}
  
void main()
{ 
	vec2 strength = (1.0f / g_resolution.xy) * g_intensity.xy;
		
	vec2 red_uv = saturate(gl_TexCoord[0].xy - strength);
	vec2 green_uv = saturate(gl_TexCoord[0].xy);
	vec2 blue_uv = saturate(gl_TexCoord[0].xy + strength);

	vec4 sample_red = texture2D(g_texture, red_uv);
	vec4 sample_green = texture2D(g_texture, green_uv);
	vec4 sample_blue = texture2D(g_texture, blue_uv);

	gl_FragColor = vec4(sample_red.r, sample_green.g, sample_blue.b, 1.0);
}