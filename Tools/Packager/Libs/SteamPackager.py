import os, stat
import sys
import shutil

from Libs.Process import *
from Libs.VersionHelper import *

class SteamPackager:

	SteamCMD_BuildMachineDir = "C:/Steamworks/tools/ContentBuilder/builder/"
	SteamCMD_Dir = "tools/ContentBuilder/builder/"
	SteamCMD_File = "steamcmd.exe"
	
	def Remove_Read_Only_File(self, fname):
		if (fname == ""):
			return
		os.chmod(fname, stat.S_IWRITE | stat.S_IREAD)
				
	def Remove_Read_Only_Dir(self, dir):
		for root, dirs, files in os.walk(dir):
			for fname in files:
				full_path = os.path.join(root, fname)
				self.Remove_Read_Only_File(full_path)
			
	def Upload(self, config):
		
		version = GetChangelistNumber()
		
		# Remove old temporary folder.
		tmp_dir = "Tmp/"
		if (os.path.isdir(tmp_dir)):
			print("Removing old temporary folder.")
			self.Remove_Read_Only_Dir(tmp_dir)
			shutil.rmtree(tmp_dir)
			
		# Create new temporary folder.
		self.Remove_Read_Only_Dir(tmp_dir)
		os.makedirs(tmp_dir)		

		output_dir = tmp_dir + "Output/"
		os.makedirs(output_dir)
			
		content_dir = tmp_dir + "Content/"
		os.makedirs(content_dir)
			
		self.Remove_Read_Only_Dir(config.OutputDir)			
		self.Remove_Read_Only_Dir(tmp_dir)
		
		# Copy over the data files.
		data_dir = tmp_dir + "/Content/data/Data"
		os.makedirs(data_dir)		
		os.makedirs(data_dir + "/Base")			
		shutil.copy(config.DataDir + "/GeoIP.dat", data_dir + "/GeoIP.dat")
		shutil.copy(config.DataDir + "/Base/Base.dat", data_dir + "/Base/Base.dat")
		shutil.copy(config.DataDir + "/Base/Base.xml", data_dir + "/Base/Base.xml")	
		shutil.copytree(config.DataDir + "/Web", data_dir + "/Web")		
			
		# Copy over docs.
		shutil.copytree(config.DataDir + "/../Docs", tmp_dir + "/Content/data/Docs")		
		shutil.copy(config.DataDir + "/../changelog.txt", tmp_dir + "/Content/data/Docs/changelog.txt")
			
		# Copy over the binary files.
		shutil.copytree(config.BinaryDir + "/Linux32", tmp_dir + "/Content/linux32/Binary/")	
		shutil.copytree(config.BinaryDir + "/MacOS", tmp_dir + "/Content/macos/Binary/")	
		shutil.copytree(config.BinaryDir + "/Win32", tmp_dir + "/Content/win32/Binary/")
		
		# Copy all binaries to another folder ready for checkin.
		self.Remove_Read_Only_Dir(config.SymbolsDir)		
		shutil.copy(config.BinaryDir + "/Win32/"+config.OutputName+".pdb", config.SymbolsDir + "/Win32/"+config.OutputName+"_" + str(version) + ".pdb")
		shutil.copy(config.BinaryDir + "/Linux32/"+config.OutputName+"", config.SymbolsDir + "/Linux32/"+config.OutputName+"_" + str(version))
		shutil.copy(config.BinaryDir + "/MacOS/ZombieGrinder.app/Contents/MacOS/"+config.OutputName+"", config.SymbolsDir + "/MacOS/"+config.OutputName+"_" + str(version))
		
		# Strip all binaries of symbols.	
		print("Stripping executable symbols. ...")			
		
		# win32
		self.Remove_Read_Only_File(tmp_dir + "/Content/win32/Binary/"+config.OutputName+".pdb")
		os.unlink(tmp_dir + "/Content/win32/Binary/"+config.OutputName+".pdb")
	
		# linux
		self.Remove_Read_Only_File(tmp_dir + "/Content/linux32/Binary/"+config.OutputName+".symbols")
		os.unlink(tmp_dir + "/Content/linux32/Binary/"+config.OutputName+".symbols")
#		proc = Process(None, "strip --strip-debug --strip-unneeded \"Game\" -o \"Game.stripped\" ", tmp_dir + "/Content/linux32/Binary/", True)
#		proc.Start();
#		proc.Wait();
				
#		self.Remove_Read_Only_File(tmp_dir + "/Content/linux32/Binary/Game")
#		os.unlink(tmp_dir + "/Content/linux32/Binary/Game")
#		os.rename(tmp_dir + "/Content/linux32/Binary/Game.stripped", tmp_dir + "/Content/linux32/Binary/Game")
		
		# macos
		self.Remove_Read_Only_File(tmp_dir + "/Content/macos/Binary/ZombieGrinder.app/Contents/MacOS/"+config.OutputName+".symbols")
		os.unlink(tmp_dir + "/Content/macos/Binary/ZombieGrinder.app/Contents/MacOS/"+config.OutputName+".symbols")
#		proc = Process(None, "strip --strip-debug --strip-unneeded \"Game\" -o \"Game.stripped\" ", tmp_dir + "/Content/macos/Binary/ZombieGrinder.app/Contents/MacOS/", True)
#		proc.Start();
#		proc.Wait();
		
#		self.Remove_Read_Only_File(tmp_dir + "/Content/macos/Binary/ZombieGrinder.app/Contents/MacOS/Game")
#		os.unlink(tmp_dir + "/Content/macos/Binary/ZombieGrinder.app/Contents/MacOS/Game")
#		os.rename(tmp_dir + "/Content/macos/Binary/ZombieGrinder.app/Contents/MacOS/Game.stripped", tmp_dir + "/Content/macos/Binary/ZombieGrinder.app/Contents/MacOS/Game")
				
		# Package MacOS application.
		# TODO		
			
		# Copy over the steam vdf's
		for file in os.listdir(config.SteamScriptDir):
			if (os.path.isdir(config.SteamScriptDir + file)):
				shutil.copytree(config.SteamScriptDir + file, tmp_dir + file)
			else:
				shutil.copy(config.SteamScriptDir + file, tmp_dir + file)
		
		# Upload tiem!
		if (config.SteamDepotPlatformName != ""):
			self.Upload_VDF(config, tmp_dir + "/app_" + config.SteamDepotName + "_" + config.SteamDepotPlatformName + ".vdf")	
		else:
			self.Upload_VDF(config, tmp_dir + "/app_" + config.SteamDepotName + ".vdf")	
			
	def Process_Mac_App(self, config, mac_app_path):
		pass
				
	def Upload_VDF(self, config, vdf_file):
	
		steamcmd_dir = config.SteamworksDir + self.SteamCMD_Dir
		if (os.path.isdir(self.SteamCMD_BuildMachineDir)):
			steamcmd_dir = self.SteamCMD_BuildMachineDir
		
		steamcmd_path = steamcmd_dir + self.SteamCMD_File
		
		# Read steam-guard code.
		file = open("Config/steam_guard_code.cfg", mode="r")
		steam_guard_code = file.read()
		file.close()
		
		# Build the data.
		cmd_line = self.SteamCMD_File + " +set_steam_guard_code "+steam_guard_code+" +login " + config.Steam_Username + " " + config.Steam_Password + " +run_app_build \"" + os.path.abspath(vdf_file) + "\" +quit" 		
		proc = Process(steamcmd_path, cmd_line, os.path.dirname(vdf_file));
		proc.Start();
		proc.Wait();
		
		if (proc.Get_Result() != 0):
			print("Failed to compile steam package.")
			sys.exit(0)