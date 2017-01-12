// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

#ifndef STATE_SETTING
#error "STATE_SETTING must be defined when including this file!"
#endif

// These define all the different state-enum-values that can be configured in the shader
// pipeline xml file.

//			  Name					Enum			

// Solid-blend
STATE_SETTING("ONE_ONE",						E_One_One)
// Alpha-blend
STATE_SETTING("SRC_ALPHA_ONE_MINUS_SRC_ALPHA",	E_Src_Alpha_One_Minus_Src_Alpha) // TODO: gross 
// Light-blend
STATE_SETTING("SRC_ALPHA_ONE",					E_Src_Alpha_One) // TODO: gross 

// Samplers	
STATE_SETTING("LOWER_OR_EQUAL",					E_Lower_Or_Equal)
STATE_SETTING("LOWER",							E_Lower)
STATE_SETTING("GREATER_OR_EQUAL",				E_Greater_Or_Equal)
STATE_SETTING("GREATER",						E_Greater)
STATE_SETTING("EQUAL",							E_Equal)
STATE_SETTING("NOT_EQUAL",						E_Not_Equal)
STATE_SETTING("NEVER",							E_Never)
STATE_SETTING("ALWAYS",							E_Always)

STATE_SETTING("BACK",							E_Back)
STATE_SETTING("FRONT",							E_Front)

STATE_SETTING("FULL",							E_Full)
STATE_SETTING("OUTPUT",							E_Output)

STATE_SETTING("NONE",							E_None)