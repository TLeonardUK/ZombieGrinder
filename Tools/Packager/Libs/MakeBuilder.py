import os
import sys
import fnmatch
import pprint

from Libs.Process import *

import xml.etree.ElementTree as ET

class ProjectBuildConfig:
	def __init__(self):
		self.Name				= ""
		self.Platform			= ""

		self.Properties			= {}
	
class ProjectConfig:
	def __init__(self):
		self.Name 	 	 		= ""
		self.File	 	 		= ""
		self.GUID	 	 		= ""

		self.Dependency_GUIDs	= []
		
		self.Header_Files		= []	
		self.Source_Files 		= []

		self.ProjectDir			= ""
		
		self.BuildDir 			= ""
		
		self.BuildConfigs		= []
		
class SolutionConfig:
	def __init__(self):
		self.Projects 			= []
	
class MakeBuilder:

	def Parse_Project(self, config, path, result):
		print("Parsing project : " + path)

		lines = []
		with open(path, 'r') as file:
			lines = file.read().split("\n")		
			
		project_config_key 		= "<ProjectConfiguration "
		clcompile_config_key 	= "<ClCompile "
		clinclude_config_key 	= "<ClInclude "
		propertygroupnocond_key = "<PropertyGroup>"
		propertygroup_key 		= "<PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=="
		end_propertygroup_key 	= "</PropertyGroup>"
		itemdefgroup_key 		= "<ItemDefinitionGroup Condition=\"'$(Configuration)|$(Platform)'=="
		end_itemdefgroup_key 	= "</ItemDefinitionGroup>"
		resourcegroup_key		= "<ResourceCompile>"
		end_resourcegroup_key	= "</ResourceCompile>"
		
		propertygroup_build = None
		in_propertygroup = False
		in_resourcegroup = False
		
		for line in lines:
			line = line.strip()
			if (line[:len(clinclude_config_key)] == clinclude_config_key):
				open_quote 	= line.find('"')
				close_quote = line.find('"', open_quote + 1)
				file 		= line[open_quote + 1:close_quote].replace("\\", '/')
				result.Header_Files.append(file)
				
			elif (line[:len(clcompile_config_key)] == clcompile_config_key):
				open_quote 	= line.find('"')
				close_quote = line.find('"', open_quote + 1)
				file 		= line[open_quote + 1:close_quote].replace("\\", '/')
				result.Source_Files.append(file)
				
			elif (line[:len(project_config_key)] == project_config_key):
				open_quote 		= line.find('"')
				close_quote 	= line.find('"', open_quote + 1)
				line 			= line[open_quote + 1:close_quote].split("|")

				build_config 			= ProjectBuildConfig()
				build_config.Name 		= line[0] 
				build_config.Platform	= line[1] 
				result.BuildConfigs.append(build_config)
				
			elif (line[:len(propertygroupnocond_key)] == propertygroupnocond_key):
				propertygroup_build = None
				in_propertygroup = True
	
			elif (line[:len(propertygroup_key)] == propertygroup_key):
				open_quote 		= line.find('\'', len(propertygroup_key))
				close_quote 	= line.find('\'', open_quote + 1)
				line 			= line[open_quote + 1:close_quote].split("|")
				
				config_name 	= line[0]
				config_platform = line[1]
				
				propertygroup_build = None
				for build in result.BuildConfigs:
					if (build.Name == config_name and build.Platform == config_platform):
						propertygroup_build = build
						in_propertygroup = True
						break
									
			elif (line[:len(end_propertygroup_key)] == end_propertygroup_key):
				propertygroup_build = None
				in_propertygroup = False
				
			elif (line[:len(resourcegroup_key)] == resourcegroup_key):
				in_resourcegroup = True
									
			elif (line[:len(end_resourcegroup_key)] == end_resourcegroup_key):
				in_resourcegroup = False				
				
			elif (line[:len(itemdefgroup_key)] == itemdefgroup_key):
				open_quote 		= line.find('\'', len(itemdefgroup_key))
				close_quote 	= line.find('\'', open_quote + 1)
				line 			= line[open_quote + 1:close_quote].split("|")
				
				config_name 	= line[0]
				config_platform = line[1]
				
				propertygroup_build = None
				for build in result.BuildConfigs:
					if (build.Name == config_name and build.Platform == config_platform):
						propertygroup_build = build
						in_propertygroup = True
						break
									
			elif (line[:len(end_itemdefgroup_key)] == end_itemdefgroup_key):
				propertygroup_build = None
				in_propertygroup = False
				
			else:
				if (in_propertygroup == True and in_resourcegroup == False):
					open_quote 		= line.find('<')
					close_quote 	= line.find('>', open_quote + 1)
					open2_quote 	= line.find('<', close_quote + 1)
					if (open2_quote > close_quote):					
						property_name	= line[open_quote + 1:close_quote]
						property_value	= line[close_quote + 1:open2_quote]
						
						# Check if property_name has a conditional assigned to it, if so fuuuuuuuuu.
						if (property_name.find("Condition=") >= 0):	
							close_quote		= property_name.rfind('\'')
							open_quote		= property_name.rfind('\'', 0, close_quote - 1)
							condition		= property_name[open_quote + 1:close_quote]
							
							property_name 	= property_name.split(' ')[0].strip()
							
							condition 		= condition.split("|")
							config_name 	= condition[0]
							config_platform = condition[1]
							
							propertygroup_build = None
							for build in result.BuildConfigs:
								if (build.Name == config_name and build.Platform == config_platform):
									propertygroup_build = build
									break
									
						if (propertygroup_build != None):
							propertygroup_build.Properties[property_name] = property_value
		
	def Parse_Solution(self, config, path):
		print("Parsing solution: " + path)
	
		lines = []
		with open(path, 'r') as file:
			lines = file.read().split("\n")
			
		result = SolutionConfig()
		
		in_deps = False
		
		for line in lines:
			line = line.strip()
			
			project_key = "Project("
			dep_key	 	= "ProjectSection(ProjectDependencies)"
			end_dep_key = "EndProjectSection"
			
			if (line[:len(project_key)] == project_key):
				if (line.find('vcxproj') > 0):
					properties = line[line.find('=') + 1:].split(',')
					
					project = ProjectConfig()
					project.Name = properties[0].strip().replace('"', '')
					project.File = properties[1].strip().replace('"', '')
					project.GUID = properties[2].strip().replace('"', '')
					project.ProjectDir = os.path.dirname(os.path.abspath((config.SolutionDir + project.File).replace('\\', '/'))) + "/"
					
					self.Parse_Project(config, config.SolutionDir + project.File.replace('\\', '/'), project)
									
					result.Projects.append(project)
					
			elif (line[:len(dep_key)] == dep_key):
				in_deps = True
				
			elif (line[:len(dep_key)] == end_dep_key):
				in_deps = False
			
			else:
				if (in_deps == True):
					dep_guid = line.split('=')[0].strip()
					result.Projects[len(result.Projects) - 1].Dependency_GUIDs.append(dep_guid)
				
		return result
				
	def Sort_By_Build_Order(self, projects):
	
		build_order = [ ]
		
		# Go through each project.
		for proj in projects:
		
			dependent = False
			dependent_index = -1
		
			# Look through all current projects, we go before any dependent projects, or
			# if non are dependent, we go last.
			idx = 0
			for other_proj in build_order:
			
				for dep in other_proj.Dependency_GUIDs:
					if (dep == proj.GUID):
						dependent = True
						dependent_index = idx
						break
			
				if (dependent == True):
					break
						
				idx += 1
				
			if (dependent == True):
				build_order.insert(dependent_index, proj)
			else:
				build_order.append(proj)
		
		return build_order
			
	def Expand_Vars(self, config, project, build_config, value):
		value = value.replace("$(SolutionDir)", 				os.path.abspath(config.SolutionDir) + "/")
		value = value.replace("$(ProjectDir)",	 				os.path.abspath(project.ProjectDir) + "/")
		value = value.replace("$(ProjectName)", 				project.Name)
		value = value.replace("$(Configuration)", 				config.SolutionConfig)
		value = value.replace("$(Platform)", 					"win32")
		value = value.replace("$(IncludePath)", 				os.path.abspath(config.SolutionDir) + "/;" + os.path.abspath(project.ProjectDir) + "/;" + os.path.abspath(config.SolutionDir + project.Name) + "/")
		value = value.replace("%(AdditionalIncludeDirectories)",os.path.abspath(config.SolutionDir) + "/;" + os.path.abspath(project.ProjectDir) + "/;" + os.path.abspath(config.SolutionDir + project.Name) + "/")
		value = value.replace("%(PreprocessorDefinitions)", 	"")
		value = value.replace("%(AdditionalDependencies)", 		"")
		value = value.replace("%(AdditionalOptions)", 			"")
		return value
			
	def GetAbsPathRelativeTo(self, path, dir):
		if (not os.path.isabs(path)):
			path = dir + path	
		#print(path + "=="+os.path.abspath(path))
		return os.path.abspath(path.replace("\\", "/"))			
			
	def Create_Make_File(self, config, project, build_config):
		source_file_string 		= ""
		object_file_string 		= ""
		ld_flags		   		= "-m32 -funwind-tables"
		ar_flags		   		= "-rv"
		c_flags					= "-m32 -funwind-tables"
		output_file_name		= project.Name
		is_static_lib			= False
		source_dir				= os.path.abspath(config.SolutionDir + "/" + project.Name).replace("\\", "/")
		base_output_dir			= os.path.abspath(config.OutputDir.replace("\\", "/"))
		output_dir				= self.GetAbsPathRelativeTo(self.Expand_Vars(config, project, build_config, build_config.Properties['OutDir']), project.ProjectDir).replace("\\", "/").replace("win32", config.OS)
		object_dir				= self.GetAbsPathRelativeTo(self.Expand_Vars(config, project, build_config, build_config.Properties['IntDir']), project.ProjectDir).replace("\\", "/").replace("win32", config.OS) 
		
		if (not os.path.isdir(output_dir)):
			os.makedirs(output_dir)
		if (not os.path.isdir(object_dir)):
			os.makedirs(object_dir)
			
		# Linux needs some extra junk.
		if (config.OS == "linux32"):
			ld_flags += " -pthread -rdynamic -L/usr/lib/i386-linux-gnu/"
	
		pprint.pprint(build_config.Properties);
	
		# Sanitize include paths to remove win32 specific ones (fucking directx).
		if ('IncludePath' in build_config.Properties):
			paths = build_config.Properties['IncludePath'].split(";")
			final = ""
			for path in paths:
				if (path.find(":") == -1):
					if (path != ""):
						final += ";"
					final += path
			build_config.Properties['IncludePath'] = final
	
		# Work out include directories.
		include_paths			= ""
		if ('IncludePath' in build_config.Properties):
			include_paths = self.Expand_Vars(config, project, build_config, build_config.Properties['IncludePath'])
		elif ('AdditionalIncludeDirectories' in build_config.Properties):
			include_paths = self.Expand_Vars(config, project, build_config, build_config.Properties['AdditionalIncludeDirectories'])
			
		# Get dependencies.
		dependencies_paths			= ""
		if ('AdditionalDependencies' in build_config.Properties):
			dependencies_paths = self.Expand_Vars(config, project, build_config, build_config.Properties['AdditionalDependencies'])
						
		# Get preprocessor defines.
		preprocessor_defines 	= self.Expand_Vars(config, project, build_config, build_config.Properties['PreprocessorDefinitions'])
		forced_includes 		= ""
		
		# Grab forced includes.
		if ("ForcedIncludeFiles" in build_config.Properties):
			forced_includes = self.Expand_Vars(config, project, build_config, build_config.Properties['ForcedIncludeFiles'])
	
		# Work out output file name.
		if ("TargetName" in build_config.Properties):
			output_file_name = self.Expand_Vars(config, project, build_config, build_config.Properties['TargetName'])
		
		# Static library?
		if (build_config.Properties['ConfigurationType'] == "StaticLibrary"):
			output_file_name += ".a"
			is_static_lib = True
		
		# Create include path command line.
		for dir in include_paths.split(";"):			
			if (dir != ""):
				c_flags += " -I" + os.path.relpath(self.GetAbsPathRelativeTo(self.Expand_Vars(config, project, build_config, dir), project.ProjectDir), project.BuildDir).replace("\\", "/").replace(" ", "\\ ")
		
		# Created dependencies.
		dependency_libs = ""
		dependencies = ""
		
		# Only linux requires this.
		if (config.OS == "linux32"):		
			dependencies = "-Wl,--start-group  "
		#else:
		#	dependencies += "-Wl,-("

		for dep in dependencies_paths.split(";"):			
			if (dep != ""):				
				path = dep.replace("\\", "/")				
				path = os.path.abspath(path)

				if (path.find(config.OutputDir) <= 0):
					continue
				
				path = path.replace(".lib", ".a")					
				dependencies += os.path.relpath(path, project.BuildDir).replace("win32", config.OS) + " \\\n"
				
				dependency_libs += os.path.relpath(path, project.BuildDir).replace("win32", config.OS) + " \\\n"		
		
		# Only linux requires this.
		if (config.OS == "linux32"):
			dependencies += " -Wl,--end-group "
		#else:
		#	dependencies += " -Wl,-)"

		# Make linker look in executable folder for shared libraries.
		# Probably a better place to put this really.
				
		# Macos needs some more linker flags for objectivec.
		if (config.OS == "macos"):
			ld_flags += " -ObjC"
				
		# Only linux requires this.
		if (config.OS == "linux32"):
			dependencies += " -Wl,-R,'$$ORIGIN' "
		
		for lib in config.Linked_Libraries:
			if (lib != ""):				
				path = lib.replace("\\", "/")		
				path = self.Expand_Vars(config, project, build_config, path)

				if (os.path.isabs(path)):
					path = os.path.abspath(path)	
					
				basename = os.path.splitext(os.path.basename(path))[0]
				dirname = os.path.dirname(path)
				if (basename[:3] == "lib"):
					dependencies += "-l" + basename[3:] + " \\\n"
					ld_flags += " -L" + os.path.relpath(self.GetAbsPathRelativeTo(self.Expand_Vars(config, project, build_config, dirname), project.ProjectDir), project.BuildDir).replace("\\", "/").replace(" ", "\\ ")
				else:
					dependencies += path + " \\\n"					
		
		# Create define command line.
		for define in preprocessor_defines.split(";"):	
			if (define != ""):
				if (define == "WIN32"):			
					# Don't need to define this on anything but windows.
					pass
				else:
					c_flags += " -D" + define
					
		# Created forced incldues command line.
		for include in forced_includes.split(";"):			
			if (include != ""):
				c_flags += " -include " + os.path.relpath(self.GetAbsPathRelativeTo(include, project.ProjectDir), project.BuildDir).replace(" ", "\\ ")
		
		# Create source file string.
		c_source_file_string = "\\\n"
		cpp_source_file_string = "\\\n"
		objc_source_file_string = "\\\n"
		objcpp_source_file_string = "\\\n"
		for file in project.Source_Files:
			#print(file+" - "+project.ProjectDir)
			path = self.GetAbsPathRelativeTo(file, project.ProjectDir)
			path = os.path.relpath(path, source_dir)
			if (os.path.splitext(file)[1] == ".cpp"):
				cpp_source_file_string += "\t" + path.replace(" ", "\\ ").replace(" ", "\\ ") + " \\\n"
			elif (os.path.splitext(file)[1] == ".c"):
				c_source_file_string += "\t" + path.replace(" ", "\\ ").replace(" ", "\\ ") + " \\\n"
			elif (os.path.splitext(file)[1] == ".m"):
				objc_source_file_string += "\t" + path.replace(" ", "\\ ").replace(" ", "\\ ") + " \\\n"
			elif (os.path.splitext(file)[1] == ".mm"):
				objcpp_source_file_string += "\t" + path.replace(" ", "\\ ").replace(" ", "\\ ") + " \\\n"
			
		# Debugging output.
		print("  Type: " + build_config.Properties['ConfigurationType'])
		print("  File: " + output_file_name)
	
		# Total buttfuck makefile generation.
		make_file = ""
		make_file += "OUTPUT_FILE			= " + output_file_name + "\n";
		make_file += "SOURCE_DIR			= " + os.path.relpath(source_dir, project.BuildDir).replace(" ", "\\ ") + "\n";
		make_file += "OBJECT_DIR			= " + os.path.relpath(object_dir, project.BuildDir).replace(" ", "\\ ") + "\n";
		make_file += "OUTPUT_DIR			= " + os.path.relpath(output_dir, project.BuildDir).replace(" ", "\\ ") + "\n";
		make_file += "\n";	
		make_file += "SOURCE_EXT_CPP		= cpp\n";
		make_file += "SOURCE_EXT_C			= c\n";
		make_file += "SOURCE_EXT_OBJC		= m\n";
		make_file += "SOURCE_EXT_OBJCPP		= mm\n";
		make_file += "\n";	
		make_file += "AR					= ar\n";
		make_file += "ARFLAGS				= " + ar_flags + "\n";
		make_file += "\n";	
		make_file += "LDFLAGS				= " + ld_flags + "\n";	
		make_file += "\n";	
		make_file += "CPP_COMPILER			= g++\n"
		make_file += "CPP_COMPILER_FLAGS	= -O3 -g -c " + c_flags + "\n" # -std=c++0x
		make_file += "\n";	
		make_file += "C_COMPILER			= gcc\n";
		make_file += "C_COMPILER_FLAGS		= -O3 -g -c " + c_flags + "\n";
		make_file += "\n";	
		make_file += "OBJC_COMPILER			= gcc\n";
		make_file += "OBJC_COMPILER_FLAGS	= -O3 -g -c " + c_flags + "\n";
		make_file += "\n";	
		make_file += "OBJCPP_COMPILER		= gcc\n";
		make_file += "OBJCPP_COMPILER_FLAGS	= -g -c " + c_flags + "\n";
		make_file += "\n";	
		make_file += "C_SOURCE_FILES		= " + c_source_file_string + "\n";
		make_file += "CPP_SOURCE_FILES		= " + cpp_source_file_string + "\n";
		make_file += "OBJC_SOURCE_FILES		= " + objc_source_file_string + "\n";
		make_file += "OBJCPP_SOURCE_FILES	= " + objcpp_source_file_string + "\n";
		make_file += "\n";	
		make_file += "DEPENDENT_LIBS		= " + dependencies + "\n";	
		make_file += "\n";	
		make_file += "DEPENDENT_LIB_FILES	= " + dependency_libs + "\n";	
		make_file += "\n";	
		make_file += "C_OBJECT_FILES		:= $(patsubst %.$(SOURCE_EXT_C),%.co,$(C_SOURCE_FILES))\n";
		make_file += "CPP_OBJECT_FILES		:= $(patsubst %.$(SOURCE_EXT_CPP),%.cppo,$(CPP_SOURCE_FILES))\n";
		make_file += "OBJC_OBJECT_FILES		:= $(patsubst %.$(SOURCE_EXT_OBJC),%.mo,$(OBJC_SOURCE_FILES))\n";
		make_file += "OBJCPP_OBJECT_FILES	:= $(patsubst %.$(SOURCE_EXT_OBJCPP),%.mmo,$(OBJCPP_SOURCE_FILES))\n";
		make_file += "\n";	
		make_file += "VPATH					 = $(OBJECT_DIR):$(SOURCE_DIR)";	
		make_file += "\n";	
		make_file += ".PHONY: all\n";
		make_file += "\n";	
		make_file += "all: $(OUTPUT_DIR)/$(OUTPUT_FILE)\n";	
		make_file += "\n";	
		make_file += "$(OUTPUT_DIR)/$(OUTPUT_FILE): $(C_OBJECT_FILES) $(CPP_OBJECT_FILES) $(OBJC_OBJECT_FILES) $(OBJCPP_OBJECT_FILES) $(DEPENDENT_LIB_FILES)\n";
		
		if (is_static_lib == True):
			make_file += "	@echo \"Archiving $@...\"\n";
			make_file += "	@$(AR) $(ARFLAGS) $@ $(addprefix $(OBJECT_DIR)/,$(C_OBJECT_FILES)) $(addprefix $(OBJECT_DIR)/,$(CPP_OBJECT_FILES)) $(addprefix $(OBJECT_DIR)/,$(OBJCPP_OBJECT_FILES))  $(addprefix $(OBJECT_DIR)/,$(OBJC_OBJECT_FILES))\n";
		else:
			make_file += "	@echo \"Linking $@...\"\n";
			make_file += "	@$(CC) $(addprefix $(OBJECT_DIR)/,$(C_OBJECT_FILES)) $(addprefix $(OBJECT_DIR)/,$(CPP_OBJECT_FILES)) $(addprefix $(OBJECT_DIR)/,$(OBJCPP_OBJECT_FILES)) $(addprefix $(OBJECT_DIR)/,$(OBJC_OBJECT_FILES)) $(DEPENDENT_LIBS) $(LDFLAGS) -o $@\n";
		
		make_file += "\n";
		make_file += "%.co: %.c\n";
		make_file += "	@echo \"Compiling C $(SOURCE_DIR)/$@...\"\n";
		make_file += "	@mkdir -p $(dir $(OBJECT_DIR)/$@)\n";		
		make_file += "	@$(C_COMPILER) $(C_COMPILER_FLAGS) $(patsubst %.co,%.$(SOURCE_EXT_C),$(SOURCE_DIR)/$@) -o $(OBJECT_DIR)/$@\n";
		make_file += "\n";		
		make_file += "%.cppo: %.cpp\n";
		make_file += "	@echo \"Compiling C++ $(SOURCE_DIR)/$@...\"\n";
		make_file += "	@mkdir -p $(dir $(OBJECT_DIR)/$@)\n";		
		make_file += "	@$(CPP_COMPILER) $(CPP_COMPILER_FLAGS) $(patsubst %.cppo,%.$(SOURCE_EXT_CPP),$(SOURCE_DIR)/$@) -o $(OBJECT_DIR)/$@\n";
		make_file += "\n";	
		make_file += "%.mo: %.m\n";
		make_file += "	@echo \"Compiling ObjC $(SOURCE_DIR)/$@...\"\n";
		make_file += "	@mkdir -p $(dir $(OBJECT_DIR)/$@)\n";		
		make_file += "	@$(OBJC_COMPILER) $(OBJC_COMPILER_FLAGS) $(patsubst %.mo,%.$(SOURCE_EXT_OBJC),$(SOURCE_DIR)/$@) -o $(OBJECT_DIR)/$@\n";
		make_file += "\n";	
		make_file += "%.mmo: %.mm\n";
		make_file += "	@echo \"Compiling ObjC++ $(SOURCE_DIR)/$@...\"\n";
		make_file += "	@mkdir -p $(dir $(OBJECT_DIR)/$@)\n";		
		make_file += "	@$(OBJCPP_COMPILER) $(OBJCPP_COMPILER_FLAGS) $(patsubst %.mmo,%.$(SOURCE_EXT_OBJCPP),$(SOURCE_DIR)/$@) -o $(OBJECT_DIR)/$@\n";
		make_file += "\n";	
		
		return make_file
		
	def Build_Project(self, config, project):
		print(("======== Building "+project.Name+" ====").ljust(35, '='))
	
		project.BuildDir = config.BuildTmpDir + project.Name + "/"
		
		if (not os.path.isdir(project.BuildDir)):
			os.mkdir(project.BuildDir)
	
		build_config = None
	
		for build in project.BuildConfigs:
			if (build.Name == config.SolutionConfig and build.Platform == "Win32"):
				build_config = build
				break
			
		if (build_config == None):
			print("Failed to find build configuration for '" + config.SolutionConfig + "'.")
			os.exit(1)
	
		# Create and dump out makefile.
		make_file_path = project.BuildDir + "makefile"
		make_file = self.Create_Make_File(config, project, build_config)

		with open(make_file_path, 'w') as file:		
			file.write(make_file)
	
		# Execute the makefile which may possibly hopefully work! 
		proc = Process("make", "all", project.BuildDir, False);
		proc.Start();
		proc.Wait();
		
		if (proc.Get_Result() != 0):
			print("Failed to compile makefile.")
			sys.exit(1)

	def Run(self, config):

		print("Using Make for code build.")

		# Parse the solution file into project references.
		solution_config = self.Parse_Solution(config, config.SolutionDir + config.SolutionFile)
				
		# Sort projects by dependencies.
		build_order = self.Sort_By_Build_Order(solution_config.Projects)

		# Build projects.
		for project in build_order:
			if (project.Name != "Data"):			# Don't want to compile data here, this is done in pre-build pass.
				self.Build_Project(config, project)

			