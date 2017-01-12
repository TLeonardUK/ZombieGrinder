from Libs.Packager import *
from Libs.shared import *

def main():
	config = Get_Shared_Config()
	config.BuildCode 			= True
	config.OS					= "win32"
	config.SolutionConfig		= "MasterDedicated"
	config.OutputName			= "Server"
	config.PurgePatterns		= [ "*.dmp" ]
									
	pack = Packager()
	pack.Run(config)

if __name__ == "__main__":
    main()