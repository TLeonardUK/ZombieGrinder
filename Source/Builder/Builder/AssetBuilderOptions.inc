// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

#ifndef OPTION
#error "OPTION must be defined when including this file!"
#endif

// These define all the different configuration options used by the engine.

// All options are accessed in the engine using an f_ prefix and using the deref operator to get/set it's value. eg.
//	int some_blah = *f_render_frame_rate;
//	*f_render_frame_rate = 100;

//	   Type				Name								Flags				Default Value						Description				

// Data setting
//OPTION(StringOption,	package,							OptionFlags::NONE,  "../Data/Base/Base.xml",			"XML configuration file of package to build.")
//OPTION(BoolOption,		rebuild,							OptionFlags::NONE,	false,								"If set to true then all assets will be rebuilt regardless of if they are out of date.")
//OPTION(BoolOption,		build_cpp_script_interfaces,		OptionFlags::NONE,	false,								"Builds CPP interface files for all native script classes. Saves a bit of copy-pasting tedium :).")


