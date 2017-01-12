import subprocess
import os
import os.path
 
class Process:

	Handle = None
	CmdLine = None
	Executable = None
	WorkingDir = None
	Shell = False
	RedirectOutput = False

	def __init__(self, exe, cmd_line, working_dir, shell=False, redirectoutput=False):
		self.CmdLine = cmd_line
		self.Executable = exe
		self.WorkingDir = working_dir
		self.Shell = shell
		self.RedirectOutput = redirectoutput

	def Start(self):
		print("Running: " + str(self.Executable))#		+ " " + str(self.CmdLine))
		print("Directory: " + self.WorkingDir)
		self.Handle = subprocess.Popen(self.CmdLine, 0, self.Executable, None, subprocess.PIPE if self.RedirectOutput else None, None, None, False, self.Shell, self.WorkingDir)
		
	def Wait(self):
		self.Handle.wait()
	
	def Get_Result(self):
		return self.Handle.returncode
		
	def Get_StdOut(self):
		return self.Handle.stdout