import os
import os.path
import uuid

def main(dir):
	filter_file = open("Data.vcxproj.filters", 'w')
	project_file = open("Data.vcxproj", 'w')

	filters = []
	write_files = []
	
	for root, dirs, files in os.walk(dir):
		for file in files:		
			full_path = root[len(dir):] + "\\" + file			
			write_files.append(full_path)
			
	for file in write_files:
		filter = os.path.dirname(file)			
		while (True):
			if (filter not in filters):
				filters.append(filter)
			if ("\\" not in filter):
				break
			filter = os.path.dirname(filter)
			
	filter_file.write("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n")
	filter_file.write("<Project ToolsVersion=\"4.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">\n")
	filter_file.write("  <ItemGroup>\n")
	
	for filter in filters:
		filter_file.write("		<Filter Include=\"" + filter + "\">\n");
		filter_file.write(" 		<UniqueIdentifier>{" + str(uuid.uuid1()) + "}</UniqueIdentifier>\n");
		filter_file.write("		</Filter>\n");
		
	filter_file.write("  </ItemGroup>\n")
	filter_file.write("  <ItemGroup>\n")
	
	for filter in write_files:
		filter_file.write("		<None Include=\"" + dir + filter + "\">\n");
		filter_file.write(" 		<Filter>"+os.path.dirname(filter)+"</Filter>\n");
		filter_file.write("		</None>\n");
					
	filter_file.write("  </ItemGroup>\n")
	filter_file.write("</Project>\n")
	
	template = ""
	with open("Template.vcxproj", "r") as temp_file:
		template = temp_file.read()
			
	includes = ""
	for filter in write_files:
		includes += "	<None Include=\"" + dir + filter + "\" />\n"

	project_file.write(template.replace("{{INCLUDES}}", includes))
			
	filter_file.close()
	project_file.close()
			
		
if __name__ == "__main__":
    main("..\\..\\Data\\")