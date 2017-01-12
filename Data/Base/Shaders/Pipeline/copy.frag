#version 120
// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

uniform sampler2D g_texture;

void main()
{    
    gl_FragColor = texture2D(g_texture, gl_TexCoord[0].xy);
}