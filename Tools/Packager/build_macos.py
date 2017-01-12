from Libs.Packager import *
from Libs.shared import *

def main():
	config = Get_Shared_Config()
	config.BuildCode 			= True
	config.OS					= "macos"
	config.PurgePatterns		= [ "*.dmp" ]
	config.Linked_Libraries		= [
		"-lstdc++",
		"-lpthread",
		"-framework CoreServices",
		"-framework AGL",
		"-framework OpenGL",
		"-framework Carbon",
		"-framework AppKit",
		"-framework IOKit",
		"-framework ForceFeedback",
		"$(SolutionDir)Generic/ThirdParty/FMod/Library/libfmod.dylib",
		"$(SolutionDir)Steamworks/redistributable_bin/osx32/libsteam_api.dylib",	
		"$(SolutionDir)Steamworks/public/steam/lib/osx32/libsdkencryptedappticket.dylib"
	]
	
	pack = Packager()
	pack.Run(config)

if __name__ == "__main__":
    main()