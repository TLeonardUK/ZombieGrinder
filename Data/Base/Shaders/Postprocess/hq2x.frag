#version 120
// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
//  Based on libretro's shaders:
//	https://github.com/libretro/common-shaders/blob/master/hqx/single-pass/shader-files/hqx.inc
// 	https://github.com/libretro/common-shaders/blob/master/scalehq/2xScaleHQ.cg
// ===================================================================

uniform sampler2D g_texture;

varying vec2 texCoord;
varying vec4 t1;
varying vec4 t2;
varying vec4 t3;
varying vec4 t4;

const vec3 dt = vec3(1.0, 1.0, 1.0);

const float mx = 0.025;       // start smoothing wt.
const float k = -1.150;       // wt. decrease factor
const float max_w = 0.55;     // max filter weigth
const float min_w = -0.10;    // min filter weigth
const float lum_add = 0.35;   // effects smoothing
 
vec3 GetPixel(vec2 pos)
{
	return texture2D(g_texture, pos).xyz;
}

void main()
{  
	vec3 c00 = GetPixel(t1.xy).xyz; 
	vec3 c10 = GetPixel(t1.zw).xyz; 
	vec3 c20 = GetPixel(t2.xy).xyz; 
	vec3 c01 = GetPixel(t4.zw).xyz; 
	vec3 c11 = GetPixel(texCoord).xyz; 
	vec3 c21 = GetPixel(t2.zw).xyz; 
	vec3 c02 = GetPixel(t4.xy).xyz; 
	vec3 c12 = GetPixel(t3.zw).xyz; 
	vec3 c22 = GetPixel(t3.xy).xyz; 

	float md1 = dot(abs(c00 - c22), dt);
	float md2 = dot(abs(c02 - c20), dt);

	float w1 = dot(abs(c22 - c11), dt) * md2;
	float w2 = dot(abs(c02 - c11), dt) * md1;
	float w3 = dot(abs(c00 - c11), dt) * md2;
	float w4 = dot(abs(c20 - c11), dt) * md1;

	float t1 = w1 + w3;
	float t2 = w2 + w4;
	float ww = max(t1, t2) + 0.1;

	c11 = (w1 * c00 + w2 * c20 + w3 * c22 + w4 * c02 + ww * c11) / (t1 + t2 + ww);

	float lc1 = k / (0.001 * dot(c10 + c12 + c11, dt) + lum_add);
	float lc2 = k / (0.001 * dot(c01 + c21 + c11, dt) + lum_add);

	w1 = clamp(lc1 * dot(abs(c11 - c10), dt) + mx, min_w, max_w);
	w2 = clamp(lc2 * dot(abs(c11 - c21), dt) + mx, min_w, max_w);
	w3 = clamp(lc1 * dot(abs(c11 - c12), dt) + mx, min_w, max_w);
	w4 = clamp(lc2 * dot(abs(c11 - c01), dt) + mx, min_w, max_w);

	gl_FragColor = vec4(w1 * c10 + w2 * c21 + w3 * c12 + w4 * c01 + (1.0 - w1 - w2 - w3 - w4) * c11, 1.0);
}