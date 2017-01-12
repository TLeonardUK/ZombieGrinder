from Libs.Packager import *
from Libs.shared import *

def main():
	config = Get_Shared_Config()
	config.BuildCode 			= True
	config.OS					= "linux32"
	config.PurgePatterns		= [ "*.dmp" ]
	config.SolutionConfig		= "Debug"
	config.Linked_Libraries		= [
		"-lstdc++",
		"-lm",
		"-ldl",
		"-lpthread",
		"-lGL",		
		"-lX11",
		"-lXxf86vm",
		"-luuid",
		"$(SolutionDir)Generic/ThirdParty/FMod/Library/libfmod.so",
		"$(SolutionDir)Steamworks/redistributable_bin/linux32/libsteam_api.so",	
		"$(SolutionDir)Steamworks/public/steam/lib/linux32/libsdkencryptedappticket.so"
	]
	
	pack = Packager()
	pack.Run(config)

if __name__ == "__main__":
    main()