// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

#ifndef STATE_SETTING
#error "STATE_SETTING must be defined when including this file!"
#endif

// These define all the different state-settings that can be configured in the shader
// pipeline xml file.

//			  Name					Enum			Type					

// Samplers	
STATE_SETTING("Clear-Color-Value",	ClearColorValue,	"color")
STATE_SETTING("Clear-Depth-Value",	ClearDepthValue,	"float")
STATE_SETTING("Depth-Function",		DepthFunction,		"enum")
STATE_SETTING("Cull-Face",			CullFace,			"enum")
STATE_SETTING("Depth-Test",			DepthTest,			"bool")
STATE_SETTING("Depth-Write",		DepthWrite,			"bool")
STATE_SETTING("Alpha-Test",			AlphaTest,			"bool")
STATE_SETTING("Clear-Color",		Clear,				"bool")
STATE_SETTING("Clear-Depth",		ClearDepth,			"bool")
STATE_SETTING("Blend",				Blend,				"bool")
STATE_SETTING("Blend-Function",		BlendFunction,		"enum")
STATE_SETTING("Viewport",			Viewport,			"enum")
