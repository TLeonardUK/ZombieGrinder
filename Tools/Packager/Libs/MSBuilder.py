import os
import sys

from Libs.Process import *

class MSBuilder:

	#MSBuild_Path = "C:/Windows/Microsoft.NET/Framework/v4.0.30319/MSBuild.exe"
	#MSBuild_Path = "C:/Windows/Microsoft.NET/Framework64/v4.0.30319"
	
	MSBuild_Path = "C:/Program Files (x86)/MSBuild/14.0/Bin/MSBuild.exe"
	
	def Run(self, config):

		print("Using MSBuild for code build.")
		
		if (config.ForceRebuild == True):
			cmd_line = config.SolutionFile + " /t:Rebuild /p:Configuration=" + config.SolutionConfig
		else:
			cmd_line = config.SolutionFile + " /t:Build /p:Configuration=" + config.SolutionConfig
		
		proc = Process(self.MSBuild_Path, cmd_line, config.SolutionDir);
		proc.Start();
		proc.Wait();
		
		if (proc.Get_Result() != 0):
			print("Failed to compile solution.")
			sys.exit(0)