from Libs.Packager import *
from Libs.shared import *

def main():

	# Upload game depot first.
	config = Get_Shared_Config()
	config.OS				= "data"
	config.BuildData 		= False
	config.SteamDepotName	= "nightly"
	config.OutputName		= "Game"
	
	pack = SteamPackager()
	pack.Upload(config)

	# Upload dedicated-server depot next.
	config = Get_Shared_Config()
	config.OS				= "data"
	config.BuildData 		= False
	config.SteamDepotName	= "nightly_dedicated"
	config.OutputName		= "Server"
	
	pack = SteamPackager()
	pack.Upload(config)

if __name__ == "__main__":
    main()