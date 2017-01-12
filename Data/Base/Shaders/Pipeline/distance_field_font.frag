#version 120
// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

uniform sampler2D 	g_texture;
uniform float 		g_scale;
uniform float 		g_spread;
uniform bool		g_draw_shadow;

varying vec4 g_vertex_color;

vec2 g_shadow_offset = vec2(-0.0025, 0.0025);
vec4 g_shadow_color  = vec4(0, 0, 0, 1);

vec4 sample(vec2 uv, vec4 color)
{		
	if (g_scale < 1.0)
	{
		vec4 center = texture2D(g_texture, uv);
	
		// Supersampling code; 
		// Credit: http://www.reddit.com/r/gamedev/comments/1q6j70/sane_way_of_rendering_accurate_text_in_gles2/
		float dscale  = 0.454; // Half of 1/sqrt2
		float friends = 0.5;   // Scale value to apply to neighbours.
		float smoothing = 0.25 / (g_spread * g_scale);
		
		vec2 duv = dscale * (dFdx(uv) + dFdy(uv));
		vec4 box = vec4(uv - duv, uv + duv);

		vec4 c = texture2D(g_texture, box.xy) +
				 texture2D(g_texture, box.zw) +
				 texture2D(g_texture, box.xw) +
				 texture2D(g_texture, box.zy);
				 
		float sum = 4.0;
				 
		vec4 result = color * (center + friends * c) / (1.0 + sum * friends);
		
		float alpha = smoothstep(0.3 - smoothing, 0.9 + smoothing, result.a);  
		
		return vec4(1.0, 1.0, 1.0, alpha) * color;		
	}
	else
	{
		float distance = texture2D(g_texture, uv).r;
		float smoothing = 0.25 / (g_spread * g_scale);
		float alpha = smoothstep(0.5 - smoothing, 0.5 + smoothing, distance);  
		
		return vec4(vec3(1.0, 1.0, 1.0), alpha) * color;	
	}
}

void main()
{
	vec4 color  = sample(gl_TexCoord[0].st, g_vertex_color);
	
	if (g_draw_shadow)
	{
		vec4 shadow = sample(gl_TexCoord[0].st + g_shadow_offset, g_shadow_color);
		gl_FragColor = mix(shadow, color, color.a);
	}
	else
	{
		gl_FragColor = color;
	}
}