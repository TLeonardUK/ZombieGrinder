#version 120
// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

uniform sampler2D g_texture;
uniform sampler2D g_object_mask;
uniform sampler2D g_filler_texture;
uniform vec3 g_resolution;
uniform vec3 g_intensity;

void main()
{ 
	vec2 coords = gl_TexCoord[0].xy;
	vec4 base = texture2D(g_texture, coords);
	vec4 objectmask = texture2D(g_object_mask, coords);
	
	// Gold tint.
	float lightness = (base.r + base.g + base.b) / 3.0;
	vec4 tint = vec4(1.0, 0.82, 0.175, 1.0) * (lightness); // Bias lighter colors to become more gold.
	base *= tint;
	
	if (objectmask.r > 0.5)
	{
		// Object filler.
		vec2 filler_tile_coords = coords * vec2(12.0f, 12.0f);
		vec4 filler = texture2D(g_filler_texture, filler_tile_coords);
		base = (base * 0.9) + (filler * 0.1);
		
		// Object outline.
		vec2 px_size 	  = vec2(1.5, 1.5) / g_resolution.xy;
		vec4 left_sample  = texture2D(g_object_mask, coords + vec2( px_size.x, 	0.0));
		vec4 right_sample = texture2D(g_object_mask, coords + vec2(-px_size.x, 	0.0));
		vec4 up_sample    = texture2D(g_object_mask, coords + vec2( 0.0, 		-px_size.y));
		vec4 down_sample  = texture2D(g_object_mask, coords + vec2( 0.0, 		 px_size.y));
		
		// If not all samples are outside or inside an object, then we're at an edge!
		vec4 total = (left_sample + right_sample + up_sample + down_sample) / 4.0;
		if (total.r > 0.0 && total.r < 1.0)
		{
			base = (base * 0.3) + (vec4(1.0, 0.88, 0.3, 1.0) * 0.7);
		}		
	}
		
	gl_FragColor = base;
}