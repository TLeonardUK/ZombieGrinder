#version 120
// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
// Combines all the parts of the gbuffer into a pretty final result!

uniform sampler2D g_diffuse;
uniform sampler2D g_distortion;
uniform sampler2D g_lighting;

uniform vec3 g_resolution;

uniform vec4 g_ambient_light;

/*
const float DISTORT_KERNEL_BASE[24] = float[24]
(
     1.0,  0.0,
     0.5,  0.8660,
    -0.5,  0.8660,
    -1.0,  0.0,
    -0.5, -0.8660,
     0.5, -0.8660,
    
     1.5,  0.8660,
     0.0,  1.7320,
    -1.5,  0.8660,
    -1.5, -0.8660,
     0.0, -1.7320,
     1.5, -0.8660
);
*/

const float DISTORT_BLUR_STRENGTH = 0.001;
const float DISTORT_STRENGTH = 256.0;

void main()
{    
	vec2 tex_position = gl_TexCoord[0].xy;
    vec4 distortion = texture2D(g_distortion, tex_position);
    vec4 lighting = texture2D(g_lighting, tex_position);
	vec4 final_color = vec4(0.0, 0.0, 0.0, 0.0);
		
	if (distortion.a <= 0.0)
	{
		final_color = texture2D(g_diffuse, tex_position);
	}
	else
	{
		// Crazy number = 127 / 255 ~ half color at 8bit depth.
		vec2 half_size = vec2(0.5);
	
		vec2 strength = (1.0 / g_resolution.xy) * DISTORT_STRENGTH;
	
		vec2 distort_offset = vec2
		(
			(-half_size.x + (distortion.r)) * strength.x,
			(-half_size.y + (distortion.g)) * strength.y
		);
	
		// Red and green channels of distortion map are offsets.
		tex_position.x += distort_offset.x; 
		tex_position.y -= distort_offset.y; 
    
		// Blue channel is a blur factor.
		if (distortion.b > 0.0f)
		{
			distortion.b *= DISTORT_BLUR_STRENGTH;

/*
			for (int i = 0; i < 12; i++)
			{
				vec2 base = vec2(DISTORT_KERNEL_BASE[(i * 2)], DISTORT_KERNEL_BASE[(i * 2) + 1]);
				final_color += texture2D(g_diffuse, tex_position + (distortion.b * base));
			}
*/	
	
			// This is unrolled because intel compilers suck and don't support arrays correctly :(		
			final_color += texture2D(g_diffuse, tex_position + (distortion.b * vec2( 1.0,  0.0)));
			final_color += texture2D(g_diffuse, tex_position + (distortion.b * vec2( 0.5,  0.8660)));
			final_color += texture2D(g_diffuse, tex_position + (distortion.b * vec2(-0.5,  0.8660)));
			final_color += texture2D(g_diffuse, tex_position + (distortion.b * vec2(-1.0,  0.0)));
			final_color += texture2D(g_diffuse, tex_position + (distortion.b * vec2(-0.5, -0.8660)));
			final_color += texture2D(g_diffuse, tex_position + (distortion.b * vec2( 0.5, -0.8660)));
			final_color += texture2D(g_diffuse, tex_position + (distortion.b * vec2( 1.5,  0.8660)));
			final_color += texture2D(g_diffuse, tex_position + (distortion.b * vec2( 0.0,  1.7320)));
			final_color += texture2D(g_diffuse, tex_position + (distortion.b * vec2(-1.5,  0.8660)));
			final_color += texture2D(g_diffuse, tex_position + (distortion.b * vec2(-1.5, -0.8660)));
			final_color += texture2D(g_diffuse, tex_position + (distortion.b * vec2( 0.0, -1.7320)));
			final_color += texture2D(g_diffuse, tex_position + (distortion.b * vec2( 1.5, -0.8660)));

			final_color /= 12.0;
		}
		else
		{
			final_color += texture2D(g_diffuse, tex_position);
		}
	}
	
	// Go go fake hdr! (aka shitty bloom)
	vec4 base = g_ambient_light + lighting;
	vec4 base_range = min(vec4(0.5), base) * vec4(2.0); // 1.0f - 0.0f range
	vec4 hdr_range = max(vec4(0.0), base - vec4(0.5)) * vec4(2.0);

	vec4 final_base = (final_color * base_range);
	vec4 final = final_base + (hdr_range * final_base); 

	vec3 ambient_base = clamp(((g_ambient_light * vec4(2.0)) * final_color).rgb, vec3(0.0), vec3(1.0));
	vec4 light_scale = vec4(vec3(1.0) - ambient_base, 1.0);

	gl_FragColor = final_color + ((final - final_color) * light_scale);
}