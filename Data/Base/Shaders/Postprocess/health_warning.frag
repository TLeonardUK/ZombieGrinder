#version 120
// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

// Source samplar.
uniform sampler2D g_texture;


// How dead our player is lol.
uniform float g_intensity;

// How much of the "red hurt" overlay to show.
uniform float g_red_intensity;

// How much of the "green hurt" overlay to show.
uniform float g_green_intensity;

void main()
{ 
	// Work out greyscale color of pixel.
	vec3  original 	= texture2D(g_texture, gl_TexCoord[0].xy).rgb;
	float sum 		= (original.r + original.g + original.b) / 3.0;
	vec3  target	= vec3(sum, sum, sum);
	
	// Final greyscale colour.
	vec3  result	= (target * g_intensity) + (original * (1.0 - g_intensity));
	
	vec2 screen_size = vec2(1.0f, 1.0f);
	
	// Calculate red overlay colour.
	vec2  center	 = vec2(screen_size.x / 2, screen_size.y / 2);
	float distance	 = distance(center, gl_TexCoord[0].xy);
	float min_radius = screen_size.x * 0.2;
	float max_radius = screen_size.x * 0.5;
	float delta	 = 1.0;
	if (distance < min_radius)
	{
		delta = 0.0;
	}
	else if (distance > max_radius)
	{
		delta = 1.0;
	}
	else
	{
		delta = (distance-min_radius) / (max_radius-min_radius);
	}
	
	// Apply the actual intensity of the red overlay.
	float red_delta = delta * g_red_intensity;
	float green_delta = delta * g_green_intensity;
	
	// Apply red overlay.
	result.r = result.r * (1.0 + (red_delta * 2.0));
	result.g = result.g * (1.0 - (red_delta * 2.0));
	result.b = result.b * (1.0 - (red_delta * 2.0));
	
	// Apply green overlay.
	result.r = result.r * (1.0 - (green_delta * 2.0));
	result.g = result.g * (1.0 + (green_delta * 2.0));
	result.b = result.b * (1.0 - (green_delta * 2.0));

	// Annnnd assign.
	gl_FragColor.rgb = result;
}