#version 120
// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

// Source samplar.
uniform sampler2D g_texture;

// How dead our player is.
uniform float p_intensity;

// How dark screen should be.
uniform float p_dark_intensity;

void main()
{ 
	// Work out greyscale color of pixel.
	vec3  original 	= texture2D(g_texture, gl_TexCoord[0].xy).rgb;
	vec3  result	= original;
	
	vec2 screen_size = vec2(1.0f, 1.0f);
	
	// Calculate black overlay colour.
	vec2  center	 = vec2(screen_size.x / 2, screen_size.y / 2);
	float distance	 = distance(center, gl_TexCoord[0].xy);
	float min_radius = screen_size.x * 0.1;
	float max_radius = screen_size.x * 0.6;
	float delta	 = 1.0;
	if (distance < min_radius)
	{
		delta = 0.0f;
	}
	else if (distance > max_radius)
	{
		delta = 1.0;
	}
	else
	{
		delta = (distance-min_radius) / (max_radius-min_radius);
	}
	
	// Bloom up original pixel color.
	float bloom_delta = (1.0f - p_dark_intensity);
	result.r += bloom_delta;
	result.g += bloom_delta;
	result.b += bloom_delta;
	
	result = clamp(result, 0.0f, 1.0f);
	
	// Work out black!
	float white_delta = delta * (p_intensity * 3);
	result.r = (result.r * (1.0 - white_delta)) * p_dark_intensity;
	result.g = (result.g * (1.0 - white_delta)) * p_dark_intensity;
	result.b = (result.b * (1.0 - white_delta)) * p_dark_intensity;
	
	// Annnnd assign.
	gl_FragColor.rgb = result;
}