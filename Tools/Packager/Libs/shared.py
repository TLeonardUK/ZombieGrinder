from Libs.Packager import *

def Get_Shared_Config():
	config = PackagerConfig()
	config.ProjectName		= "Zombie Grinder"
	config.OutputDir		= "Output/"
	config.SymbolsDir		= "../Symbols/"
	config.SteamScriptDir		= "Config/"
	config.TmpDir			= "Tmp/"
	config.SteamDepotName		= "developer"
	config.SteamDepotPlatformName 	= ""
	config.Platform			= "steam"
	config.SteamworksDir		= "../Source/Steamworks/"
	config.SolutionOutputDir	= "../Output/Game/"
	config.SolutionBuilderOutputDir	= "../Output/Builder/"
	config.SolutionDir		= "../Source/"
	config.SolutionFile		= "Game.sln"
	config.SolutionConfig		= "Master"
	config.OutputName		= "Game"
	config.BuilderOutputName	= "Builder"
	config.ConfigDir		= "../Config/"
	config.DataDir			= "../Data/"
	config.BinaryDir		= "../Binary/"	
	config.Steam_Username		= ""
	config.Steam_Password		= ""
	config.PurgePatterns		= [ ]
	config.MacOSPackageName	   	 = "ZombieGrinder.app"

	config.VersionInfoFiles		= [ 
		VersionInfo("../Source/Game/Version.cpp", "../Source/Game/Version.h", "../Source/Game/Resources/Resource.rc", "Game"),		
		VersionInfo("../Source/Engine/Version.cpp", "../Source/Engine/Version.h", "", "Engine")	
	]
		
	return config
