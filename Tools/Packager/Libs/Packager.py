import time
import os, stat
import shutil
import fnmatch

from Libs.MSBuilder import *
from Libs.MakeBuilder import *
from Libs.SteamPackager import *
from Libs.VersionHelper import *
from Libs.ChangelogHelper import *
from Libs.Process import *

class VersionInfo:
	def __init__(self, cpp, hpp, res, proj):
	
		self.CPP_Path 		= cpp
		self.HPP_Path 		= hpp
		self.Resource_Path 	= res
		self.Project 		= proj

class PackagerConfig:
	def __init__(self):

		# Needs to be setup by caller.
		self.ProjectName		= ""
		self.OS					= ""
		self.OutputDir			= ""
		self.SteamScriptDir		= ""
		self.SteamDepotName		= ""
		self.Platform			= ""
		self.SteamworksDir		= ""
		self.SolutionDir		= ""
		self.SolutionOutputDir	= ""
		self.SolutionFile		= ""
		self.SolutionConfig		= ""
		self.ForceRebuild		= False
		self.DebugBuild			= False
		self.DataDir			= ""
		self.BinaryDir			= ""
		self.TmpDir				= ""
		self.PurgePatterns		= []
		self.Steam_Username		= ""
		self.Steam_Password		= ""
		self.Linked_Libraries	= []
		self.BuildData			= False
		self.BuildCode			= False
		self.IncrementVersion	= False
		self.VersionInfoFiles	= []
		
		# Internal
		self.BuildDir			= ""
		self.BuildContentDir	= ""
		self.BuildOutputDir		= ""
		self.BuildTmpDir		= ""
		self.BuildVersionNumber 		= ""
	
class Packager:

	def Run(self, config):
	
		print("Packaging " + config.ProjectName + " for " + config.OS + " (" + config.Platform + ")")

		# Increment version.
		if (config.IncrementVersion == True):
			self.IncrementVersionNumber(config)
			return
		
		# Get version number.
		self.GetVersionNumber(config)
		
		# Setup build directory.
		self.CreateBuildDir(config)
		
		# Build source code into binary.
		if (config.BuildCode == True):
			self.BuildCode(config)
		
		# Build data.
		if (config.BuildData == True):
			self.BuildData(config)
		
		# Package everything up.
		if (config.BuildCode == True):
			self.CopyBinaryToOutput(config)
	
	def GetVersionNumber(self, config):
		for info in config.VersionInfoFiles:
			config.BuildVersionNumber = GetChangelistNumber()#GetVersionNumber(info.CPP_Path)
			break

		print("Version Number: %i" % config.BuildVersionNumber)
	
	def IncrementVersionNumber(self, config):
		print("Incrementing version number ...")
		for info in config.VersionInfoFiles:
			self.Remove_Read_Only_File(info.HPP_Path)
			self.Remove_Read_Only_File(info.CPP_Path)
			self.Remove_Read_Only_File(info.Resource_Path)
			UpdateVersionInfo(info.HPP_Path, info.CPP_Path, info.Resource_Path, info.Project, GetChangelistNumber())
				
	def Remove_Read_Only_File(self, fname):
		if (fname == ""):
			return
		os.chmod(fname, stat.S_IWRITE | stat.S_IREAD)
				
	def Remove_Read_Only_Dir(self, dir):
		for root, dirs, files in os.walk(dir):
			for fname in files:
				full_path = os.path.join(root, fname)
				self.Remove_Read_Only_File(full_path)
			
	def CreateBuildDir(self, config):

		if (config.DebugBuild == True):
			config.BuildDir = config.OutputDir + config.OS + "/Debug/"
		else:
			config.BuildDir = config.OutputDir + config.OS + "/" + str(config.BuildVersionNumber) + "/"
			if (os.path.isdir(config.BuildDir)):
				shutil.rmtree(config.BuildDir)
		
		if (not os.path.isdir(config.BuildDir)):
			os.makedirs(config.BuildDir)
		
		print("Build Directory: " + config.BuildDir)
		
		self.Remove_Read_Only_Dir(config.BuildDir)
		
		config.BuildContentDir = config.BuildDir + "Content/"
		#config.BuildOutputDir = config.BuildDir + "Output/"
		config.BuildTmpDir = config.BuildDir + "Tmp/"

		#if (config.DebugBuild == False):
		if (os.path.isdir(config.BuildContentDir)):
			shutil.rmtree(config.BuildContentDir)

		#if (os.path.isdir(config.BuildOutputDir)):
		#	shutil.rmtree(config.BuildOutputDir)

		if (os.path.isdir(config.BuildTmpDir)):
			shutil.rmtree(config.BuildTmpDir)

		if (not os.path.isdir(config.BuildContentDir)):
			os.makedirs(config.BuildContentDir)

		#if (not os.path.isdir(config.BuildOutputDir)):
		#	os.makedirs(config.BuildOutputDir)

		if (not os.path.isdir(config.BuildTmpDir)):
			os.makedirs(config.BuildTmpDir)
			
	def BuildCode(self, config):
		print("Building source code ...")
		
		builder = None
		
		output_bin_dir	= config.BuildContentDir + "Binary/"
		
		if (config.OS == "win32"):
			builder = MSBuilder()
		elif (config.OS == "linux32"):
			builder = MakeBuilder()
		elif (config.OS == "macos"):
			builder = MakeBuilder()
			
		builder.Run(config)
			
		# Fix macos fmod install name. FFS
		if (config.OS == "macos"):
			print("Patching fmod rpath issue -_-")
			proc = Process(None, "install_name_tool -change @rpath/libfmod.dylib @loader_path/libfmod.dylib \""+config.OutputName+"\"", config.SolutionOutputDir + config.SolutionConfig + "/macos/", True);
			proc.Start();
			proc.Wait();
		
	def BuildData(self, config):
		print("Building game data ...")
		
		output_data_dir = config.BuildContentDir + "Data/"
		output_bin_dir 	= config.BuildContentDir + "Binary/"
		
		os.mkdir(output_data_dir)
		
		# Update changelog.
		print("Generating changelists ...");
		GenerateChangelog(os.path.abspath("..\\changelog.txt"));
		
		# Run data builder.
		print("Building data ...")
		cmd_line = "exe_path Rebuild Base" 		
		proc = Process(os.path.abspath("..\\Output\\Builder\\Master\\win32\\Builder.exe"), cmd_line, os.path.abspath("..\\Binary\\Win32"));
		proc.Start();
		proc.Wait();
		
		if (proc.Get_Result() != 0):
			print("Failed to compile data.")
			sys.exit(0)
			
		# Copy data folder.
		print("Copying data folder ...")
		#shutil.copytree(config.DataDir, output_data_dir)
		os.mkdir(output_data_dir + "Base/")
		shutil.copy(config.DataDir + "Base/Base.dat", output_data_dir + "Base/Base.dat")
		shutil.copy(config.DataDir + "Base/Base.xml", output_data_dir + "Base/Base.xml")
		shutil.copy(config.DataDir + "GeoIP.dat", output_data_dir + "GeoIP.dat")
		
	def CopyBinaryToOutput(self, config):
	
		# Copy to binaries folder as well as we will likely be checking in the updated files.
		print("Copying output executable to binaries folder ...")
		if (config.OS == "win32"):
			shutil.copy(config.SolutionOutputDir + config.SolutionConfig + "/win32/"+config.OutputName+".exe", config.BinaryDir + "Win32/"+config.OutputName+".exe")
			shutil.copy(config.SolutionOutputDir + config.SolutionConfig + "/win32/"+config.OutputName+".pdb", config.BinaryDir + "Win32/"+config.OutputName+".pdb")
			shutil.copy(config.SolutionBuilderOutputDir + config.SolutionConfig + "/win32/"+config.BuilderOutputName+".exe", config.BinaryDir + "Win32/"+config.BuilderOutputName+".exe")
			shutil.copy(config.SolutionBuilderOutputDir + config.SolutionConfig + "/win32/"+config.BuilderOutputName+".pdb", config.BinaryDir + "Win32/"+config.BuilderOutputName+".pdb")

		elif (config.OS == "linux32"):
			proc = Process(None, "strip --strip-debug --strip-unneeded \""+config.OutputName+"\" -o \""+config.OutputName+".stripped\" ", config.SolutionOutputDir + config.SolutionConfig + "/linux32/", True)
			proc.Start();
			proc.Wait();

			shutil.copy(config.SolutionOutputDir + config.SolutionConfig + "/linux32/"+config.OutputName+".stripped" , config.BinaryDir + "Linux32/"+config.OutputName+"")
			shutil.copy(config.SolutionOutputDir + config.SolutionConfig + "/linux32/"+config.OutputName+"" , config.BinaryDir + "Linux32/"+config.OutputName+".symbols")
			
			proc = Process(None, "strip --strip-debug --strip-unneeded \""+config.BuilderOutputName+"\" -o \""+config.BuilderOutputName+".stripped\" ", config.SolutionBuilderOutputDir + config.SolutionConfig + "/linux32/", True)
			proc.Start();
			proc.Wait();

			shutil.copy(config.SolutionBuilderOutputDir + config.SolutionConfig + "/linux32/"+config.BuilderOutputName+".stripped" , config.BinaryDir + "Linux32/"+config.BuilderOutputName+"")
			shutil.copy(config.SolutionBuilderOutputDir + config.SolutionConfig + "/linux32/"+config.BuilderOutputName+"" , config.BinaryDir + "Linux32/"+config.BuilderOutputName+".symbols")

		elif (config.OS == "macos"):
			proc = Process(None, "strip \""+config.OutputName+"\" -o \""+config.OutputName+".stripped\" ", config.SolutionOutputDir + config.SolutionConfig + "/macos/", True);
			proc.Start();
			proc.Wait();

			shutil.copy(config.SolutionOutputDir + config.SolutionConfig + "/macos/"+config.OutputName+".stripped" , config.BinaryDir + "MacOS/" + config.MacOSPackageName + "/Contents/MacOS/"+config.OutputName+"")
			shutil.copy(config.SolutionOutputDir + config.SolutionConfig + "/macos/"+config.OutputName+"" , config.BinaryDir + "MacOS/" + config.MacOSPackageName + "/Contents/MacOS/"+config.OutputName+".symbols")
		
			proc = Process(None, "strip \""+config.BuilderOutputName+"\" -o \""+config.BuilderOutputName+".stripped\" ", config.SolutionBuilderOutputDir + config.SolutionConfig + "/macos/", True);
			proc.Start();
			proc.Wait();

			shutil.copy(config.SolutionOutputDir + config.SolutionConfig + "/macos/"+config.OutputName+".stripped" , config.BinaryDir + "MacOS/" + config.MacOSPackageName + "/Contents/MacOS/"+config.OutputName+"")
			shutil.copy(config.SolutionOutputDir + config.SolutionConfig + "/macos/"+config.OutputName+"" , config.BinaryDir + "MacOS/" + config.MacOSPackageName + "/Contents/MacOS/"+config.OutputName+".symbols")
		
	def BuildOutput(self, config):
		print("Building output folder ...")
		
		output_bin_dir 	= config.BuildContentDir + "Binary/"
					
		# Copy binary folder.
		print("Copying binary folder ...")
		if (os.path.isdir(output_bin_dir)):
			self.Remove_Read_Only_Dir(output_bin_dir)
			shutil.rmtree(output_bin_dir)

		# Copy misc binary files over.
		if (config.OS == "win32"):
			shutil.copytree(config.BinaryDir + "Win32/", output_bin_dir)
		if (config.OS == "linux32"):			
			shutil.copytree(config.BinaryDir + "Linux32/", output_bin_dir)
		if (config.OS == "macos"):			
			shutil.copytree(config.BinaryDir + "MacOS/", output_bin_dir)
	
		# MacOS requires a silly application package directory layout ;_;
		if (config.OS == "macos"):	
			# Change output directory to the MacOS folder in the package.
			output_bin_dir = output_bin_dir + config.MacOSPackageName + "/Contents/MacOS/"		
			
		# Copy symbols.
		print("Copying output symbols ...")
		if (not os.path.isdir(config.SymbolsDir)):
			os.mkdir(config.SymbolsDir)
		if (config.OS == "win32"):
			if (not os.path.isdir(config.SymbolsDir + "win32/")):
				os.mkdir(config.SymbolsDir + "win32/")
			shutil.copy(config.SolutionOutputDir + config.SolutionConfig + "/win32/"+config.OutputName+".pdb" , config.SymbolsDir + "/win32/"+config.OutputName+"_" + str(config.BuildVersionNumber) + ".pdb")
		elif (config.OS == "linux32"):
			if (not os.path.isdir(config.SymbolsDir + "linux32/")):
				os.mkdir(config.SymbolsDir + "linux32/")
			shutil.copy(config.SolutionOutputDir + config.SolutionConfig + "/linux32/"+config.OutputName+"" , config.SymbolsDir + "/linux32/"+config.OutputName+"_" + str(config.BuildVersionNumber))
		elif (config.OS == "macos"):
			if (not os.path.isdir(config.SymbolsDir + "macos/")):
				os.mkdir(config.SymbolsDir + "macos/")
			shutil.copy(config.SolutionOutputDir + config.SolutionConfig + "/macos/"+config.OutputName+"" , config.SymbolsDir + "/macos/"+config.OutputName+"_" + str(config.BuildVersionNumber))

		# Strip symbols from executable on linux and mac.
		print("Stripping executable symbols. ...")
		if (config.OS == "linux32"):
			proc = Process(None, "strip --strip-debug --strip-unneeded \""+config.OutputName+"\" -o \""+config.OutputName+".stripped\" ", config.SolutionOutputDir + config.SolutionConfig + "/linux32/", True)
			proc.Start();
			proc.Wait();
		elif (config.OS == "macos"):
			proc = Process(None, "strip \""+config.OutputName+"\" -o \""+config.OutputName+".stripped\" ", config.SolutionOutputDir + config.SolutionConfig + "/macos/", True);
			proc.Start();
			proc.Wait();
			
		# Copy output executable.
		print("Copying output executable ...")
		if (config.OS == "win32"):
			shutil.copy(config.SolutionOutputDir + config.SolutionConfig + "/win32/"+config.OutputName+".exe" , output_bin_dir + ""+config.OutputName+".exe")
		elif (config.OS == "linux32"):
			shutil.copy(config.SolutionOutputDir + config.SolutionConfig + "/linux32/"+config.OutputName+".stripped" , output_bin_dir + ""+config.OutputName+"")
		elif (config.OS == "macos"):
			shutil.copy(config.SolutionOutputDir + config.SolutionConfig + "/macos/"+config.OutputName+".stripped" , output_bin_dir + ""+config.OutputName+"")
			
		# Copy to binaries folder as well as we will likely be checking in the updated files.
		print("Copying output executable to binaries folder ...")
		if (config.OS == "win32"):
			shutil.copy(config.SolutionOutputDir + config.SolutionConfig + "/win32/"+config.OutputName+".exe", config.BinaryDir + "Win32/"+config.OutputName+".exe")
		elif (config.OS == "linux32"):
			shutil.copy(config.SolutionOutputDir + config.SolutionConfig + "/linux32/"+config.OutputName+".stripped" , config.BinaryDir + "Linux32/"+config.OutputName+"")
		elif (config.OS == "macos"):
			shutil.copy(config.SolutionOutputDir + config.SolutionConfig + "/macos/"+config.OutputName+".stripped" , config.BinaryDir + "MacOS/" + config.MacOSPackageName + "/Contents/MacOS/"+config.OutputName+"")
			
		# Run purge patterns on all files in directory.
		for root, dirs, files in os.walk(config.BuildContentDir):  
			for file in files:
				for pattern in config.PurgePatterns:
					if fnmatch.fnmatch(file, pattern):
						print("Purging file: "+root+"/"+file) 
						os.chmod(root+"/"+file, stat.S_IWRITE)
						os.unlink(root+"/"+file)
						break
		
	def BuildPlatform(self, config):
		print("Building platform folder ...")
			
		# Run the actual packager.
		plat = None
		if (config.Platform == "steam"):
			plat = SteamPackager()	
			
		print("Running platform packager ...")	
		plat.Build(config)
