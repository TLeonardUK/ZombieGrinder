// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Builder/Compilers/Scripts/ScriptCompiler.h"
#include "Builder/Builder/AssetBuilder.h"
#include "Builder/Builder/AssetBuilderOptions.h"

#include "Generic/Types/PooledMemoryHeap.h"

#include "Engine/IO/StreamFactory.h"
#include "Engine/IO/PatchedBinaryStream.h"
#include "Engine/Platform/Platform.h"

#include "XScript/Compiler/CCompiler.h"
#include "XScript/Compiler/CTranslationUnit.h"
#include "XScript/Helpers/CPathHelper.h"

#include "XScript/Translator/CTranslator.h"
#include "XScript/Translator/VMBinary/CVMBinaryTranslator.h"
#include "XScript/VirtualMachine/CVMBinary.h"
#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/Parser/Nodes/CASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CClassASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CClassBodyASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CClassMemberASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CClassPropertyASTNode.h"
#include "XScript/Parser/Nodes/Statements/CVariableStatementASTNode.h"

#include "XScript/Parser/Types/CArrayDataType.h"
#include "XScript/Parser/Types/CBoolDataType.h"
#include "XScript/Parser/Types/CFloatDataType.h"
#include "XScript/Parser/Types/CIntDataType.h"
#include "XScript/Parser/Types/CObjectDataType.h"
#include "XScript/Parser/Types/CStringDataType.h"
#include "XScript/Parser/Types/CVoidDataType.h"

ScriptCompiler::ScriptCompiler()
{
	m_build_mutex = Mutex::Create();
}

ScriptCompiler::~ScriptCompiler()
{
	SAFE_DELETE(m_build_mutex);
}

int ScriptCompiler::Get_Version()
{
	return 74;
}

std::vector<AssetToCompile> ScriptCompiler::Gather_Assets(AssetBuilder* builder)
{
	std::vector<AssetToCompile> result;

	ConfigFile* file = builder->Get_Package_Config_File();

	if (!file->Contains("scripts"))
	{
		return result;
	}

	std::vector<ConfigFileNode> atlases	= file->Get<std::vector<ConfigFileNode> >("scripts/script");

	// Load scripts.
	for (std::vector<ConfigFileNode>::iterator atlas_iter = atlases.begin(); atlas_iter != atlases.end(); atlas_iter++)
	{
		ConfigFileNode& node = *atlas_iter;

		AssetToCompile asset;
		asset.Source_Path = file->Get<const char*>("file", node, true);
		asset.Priority = "";

		if (file->Contains("priority", node, true))
		{
			asset.Priority = file->Get<const char*>("priority", node, true);
		}

		// We load dependencies based on all the included files the last time we compiled (or none if this is the first time).

		result.push_back(asset);
	}

	return result;
}

bool ScriptCompiler::Build_Asset(AssetBuilder* builder, AssetToCompile compile)
{
	// TODO: Blah, atm our String class results in constant lock contention. Faster to let one script
	// compile at a time for now. Plz fix.
	MutexLock lock(m_build_mutex);

//	platform_get_malloc_heap()->Push_Group(); // TODO: Lazy as fuck solution to compiling mem leaks :S.

	{
		std::string resolved_sound_path = builder->Resolve_Asset_Path(compile.Source_Path);
		std::string compiled_sound_path = builder->Get_Compiled_Asset_File(compile.Source_Path);

		std::string script_dir = Platform::Get()->Get_Absolute_Path(builder->Get_Package_XML_Dir() + "/" + builder->Get_Package_Config_File()->Get<std::string>("package/script_directory"));
		std::string root_dir = builder->Get_Root_Dir();
		std::string root_package_dir = builder->Get_Package_Root_Dir();

		// Write out compiled version.
		std::string source = "";
		if (!StreamFactory::Load_Text(resolved_sound_path.c_str(), &source))	
		{
			DBG_LOG("Could not open: %s", resolved_sound_path.c_str());
			return false;
		}

		// Compile the script.
		CCompiler script_compiler;
		int fatal_errors = 0;

	//	DBG_LOG("Compiling %s ...", compile.Source_Path.c_str());
		CTranslationUnit* translation_unit = script_compiler.Compile(resolved_sound_path, script_dir, std::vector<CDefine>());

		std::vector<CCompileError> errors = translation_unit->GetErrors();
		for (std::vector<CCompileError>::iterator iter = errors.begin(); iter != errors.end(); iter++)
		{
			CCompileError& error = *iter;
			if (error.IsFatal())
			{
				fatal_errors++;
			}
			DBG_LOG("\t%s", error.ToString().c_str());
			printf("%s\n", error.ToString().c_str()); // Done so VS picks up on the errors.
		}

		if (fatal_errors > 0)
		{
			DBG_LOG("Failed with %i fatal errors.", fatal_errors);
			return false;
		}

		// Store extra dependencies so we recompile if any included files are changed.
		std::vector<std::string> deps;
		std::vector<String> using_files = translation_unit->GetUsingFileList();

		std::string base_dir = CPathHelper::GetAbsolutePath(root_dir + "/").c_str();

		for (std::vector<String>::iterator iter = using_files.begin(); iter != using_files.end(); iter++)
		{
			// File needs to be pre-resolve. eg Data\Scripts\Derp not Data\Base\Scripts\Derp.
			std::string path = (*iter).c_str();
			int data_idx = path.find("Data/");
			if (data_idx != std::string::npos)
			{
				int end_idx = path.find("/", data_idx + 6);
				if (end_idx != std::string::npos)
				{
					path = "Data" + path.substr(end_idx);
				}
				else
				{
					std::string rel_path = CPathHelper::GetRelativePath(path, base_dir).c_str();
					std::string use_path = (root_package_dir + rel_path).c_str();
					path = use_path;
				}
			}
			deps.push_back(path);
		}
		builder->Write_Extra_Dependencies(compile, deps);
	
		// Dump out native interfaces.
#ifndef MASTER_BUILD
		if (true)//*AssetBuilderOptions::build_cpp_script_interfaces)
		{
			String dir_file_path = (builder->Get_Compile_Dir() + "/.interfaces");
			Platform::Get()->Create_Directory(dir_file_path.c_str(), false);

			CASTNode* node = translation_unit->GetASTRoot();
			for (std::vector<CASTNode*>::iterator iter = node->Children.begin(); iter != node->Children.end(); iter++)
			{
				CClassASTNode* class_node = dynamic_cast<CClassASTNode*>(*iter);
				if (class_node->IsNative && (class_node->IsGeneric == false || class_node->GenericInstanceOf != NULL))
				{
					String h_file_path = (builder->Get_Compile_Dir() + "/.interfaces/CRuntime_" + class_node->MangledIdentifier + ".h");
					String cpp_file_path = (builder->Get_Compile_Dir() + "/.interfaces/CRuntime_" + class_node->MangledIdentifier + ".cpp");
					Stream* h_file = StreamFactory::Open(h_file_path.c_str(), StreamMode::Write);
					Stream* cpp_file = StreamFactory::Open(cpp_file_path.c_str(), StreamMode::Write);

					Dump_Native_Interface(class_node, h_file, cpp_file);

					SAFE_DELETE(h_file);
					SAFE_DELETE(cpp_file);
				}
			}
		}
#endif

		// Get compiled script.
		CVMBinary* vm_binary = dynamic_cast<CVMBinaryTranslator*>(script_compiler.GetTranslator())->GetVMBinary();

		// Write out compiled version.
		PackageFile* file = builder->Get_Package_File();
		PatchedBinaryStream* stream = new PatchedBinaryStream(compiled_sound_path.c_str(), StreamMode::Write);

		vm_binary->Serialize(stream);

		stream->Close();

		SAFE_DELETE(stream);
		SAFE_DELETE(vm_binary);
		SAFE_DELETE(translation_unit);
	}
	
	//platform_get_malloc_heap()->Pop_Group();

	return true;
}

void ScriptCompiler::Add_Chunks(AssetBuilder* builder, AssetToCompile asset)
{
	//std::string package_location = asset.Source_Path;
	//std::string source_location = builder->Resolve_Asset_Path(package_location);
	//std::string compiled_sound_path = builder->Get_Compiled_Asset_File(package_location);
	//builder->Get_Package_File()->Add_Chunk(compiled_sound_path.c_str(), package_location.c_str(), PackageFileChunkType::Script, false, asset.Priority.c_str());
}

void ScriptCompiler::Finalize(AssetBuilder* builder, std::vector<AssetToCompile> assets, std::vector<AssetToCompile> rebuilt)
{
	if (assets.size() == 0)
	{
		return;
	}

	CVMBinary binary;
	
	// WARNING: Be very careful changing this filename, some things in EngineVirtualMachine rely on it.
	std::string mega_script = StringHelper::Format("Data/Scripts/__%s_Linked.xs", builder->Get_Package_Name().c_str());
	std::string compiled_sound_path = builder->Get_Compiled_Asset_File(mega_script);

	if (!Platform::Get()->Is_File(compiled_sound_path.c_str()) || rebuilt.size() > 0)
	{
		if (assets.size() == 1)
		{
			for (std::vector<AssetToCompile>::iterator iter = assets.begin(); iter != assets.end(); iter++)
			{
				AssetToCompile root = *iter;

				DBG_LOG("Saving '%s' ...", root.Source_Path.c_str());

				std::string compiled_root_path = builder->Get_Compiled_Asset_File(root.Source_Path);
				Platform::Get()->Copy_File(compiled_root_path.c_str(), compiled_sound_path.c_str());
			}
		}
		else
		{
			DBG_LOG("Linking scripts into '%s'.", compiled_sound_path.c_str());

			std::vector<CVMBinary*> binaries;

			// Link all scripts together into a mega-script :3 
			// Skips the linking step at runtime to save a good chunk of loading time.
			for (std::vector<AssetToCompile>::iterator iter = assets.begin(); iter != assets.end(); iter++)
			{
				AssetToCompile root = *iter;
		
				std::string compiled_root_path = builder->Get_Compiled_Asset_File(root.Source_Path);

				DBG_LOG("Linking '%s' ...", root.Source_Path.c_str());

				Stream* input_stream = StreamFactory::Open(compiled_root_path.c_str(), StreamMode::Read);
				PatchedBinaryStream patched_stream(input_stream, StreamMode::Read);

				CVMBinary* other_binary = new CVMBinary();
				other_binary->Deserialize(&patched_stream);	
				binary.Merge(other_binary);
				binaries.push_back(other_binary);

				SAFE_DELETE(input_stream);
			}
	
			PatchedBinaryStream* stream = new PatchedBinaryStream(compiled_sound_path.c_str(), StreamMode::Write);
			binary.Serialize_Merged(stream);
			SAFE_DELETE(stream);

			for (std::vector<CVMBinary*>::iterator iter = binaries.begin(); iter != binaries.end(); iter++)
			{
				SAFE_DELETE(*iter);
			}
			binaries.clear();

			DBG_LOG("Link complete.");
		}
	}

	builder->Get_Package_File()->Add_Chunk(compiled_sound_path.c_str(), mega_script.c_str(), PackageFileChunkType::Script, false, "", rebuilt.size() > 0);
}

void ScriptCompiler::Dump_Native_Interface(CClassASTNode* class_node, Stream* header_file, Stream* source_file)
{
	std::string class_name = StringHelper::Format("CRuntime_%s", class_node->Identifier.c_str());

	header_file->WriteLine("// ===================================================================");
	header_file->WriteLine("//	Copyright (C) 2013 Tim Leonard");
	header_file->WriteLine("// ===================================================================");
	header_file->WriteLine(StringHelper::Format("#ifndef _GAME_RUNTIME_%s_", StringHelper::Uppercase(class_node->Identifier.c_str()).c_str()).c_str());
	header_file->WriteLine(StringHelper::Format("#define _GAME_RUNTIME_%s_", StringHelper::Uppercase(class_node->Identifier.c_str()).c_str()).c_str());
	header_file->WriteLine("");
	header_file->WriteLine("#include \"XScript/VirtualMachine/CVirtualMachine.h\"");
	header_file->WriteLine("#include \"XScript/VirtualMachine/CVMValue.h\"");
	header_file->WriteLine("");
	header_file->WriteLine(StringHelper::Format("class %s", class_name.c_str()).c_str());
	header_file->WriteLine("{");
	header_file->WriteLine("public:");

	source_file->WriteLine("// ===================================================================");
	source_file->WriteLine("//	Copyright (C) 2013 Tim Leonard");
	source_file->WriteLine("// ===================================================================");
	source_file->WriteLine(StringHelper::Format("#include \"Game/Scripts/Runtime/CRuntime_%s.h\"", class_node->Identifier.c_str()).c_str());
	source_file->WriteLine("#include \"XScript/VirtualMachine/CVirtualMachine.h\"");
	source_file->WriteLine("#include \"XScript/VirtualMachine/CVMContext.h\"");
	source_file->WriteLine("#include \"XScript/VirtualMachine/CVMBinary.h\"");
	source_file->WriteLine("");

	for (std::vector<CASTNode*>::iterator iter = class_node->Body->Children.begin(); iter != class_node->Body->Children.end(); iter++)
	{		
		CClassMemberASTNode* member = dynamic_cast<CClassMemberASTNode*>(*iter);
		CClassPropertyASTNode* property = dynamic_cast<CClassPropertyASTNode*>(*iter);

		if (member != NULL)
		{
			Dump_Native_Member(class_name, class_node->MangledIdentifier.c_str(), member, header_file, source_file, false);
		}
		else if (property != NULL)
		{
			for (std::vector<CASTNode*>::iterator iter = property->Children.begin(); iter != property->Children.end(); iter++)
			{		
				CClassMemberASTNode* sub_member = dynamic_cast<CClassMemberASTNode*>(*iter);
				if (sub_member != NULL)
				{
					Dump_Native_Member(class_name, class_node->MangledIdentifier.c_str(), sub_member, header_file, source_file, false);
				}
			}
		}
	}	
	
	source_file->WriteLine(StringHelper::Format("void %s::Bind(CVirtualMachine* vm)", class_name.c_str()).c_str());
	source_file->WriteLine("{");
	
	for (std::vector<CASTNode*>::iterator iter = class_node->Body->Children.begin(); iter != class_node->Body->Children.end(); iter++)
	{
		CClassMemberASTNode* member = dynamic_cast<CClassMemberASTNode*>(*iter);
		CClassPropertyASTNode* property = dynamic_cast<CClassPropertyASTNode*>(*iter);

		if (member != NULL)
		{
			Dump_Native_Member(class_name, class_node->MangledIdentifier.c_str(), member, header_file, source_file, true);
		}
		else if (property != NULL)
		{
			for (std::vector<CASTNode*>::iterator iter = property->Children.begin(); iter != property->Children.end(); iter++)
			{		
				CClassMemberASTNode* sub_member = dynamic_cast<CClassMemberASTNode*>(*iter);
				if (sub_member != NULL)
				{
					Dump_Native_Member(class_name, class_node->MangledIdentifier.c_str(), sub_member, header_file, source_file, true);
				}
			}
		}
	}	

	source_file->WriteLine("}");

	header_file->WriteLine("");
	header_file->WriteLine("	static void Bind(CVirtualMachine* machine);");
	header_file->WriteLine("};");
	header_file->WriteLine("");
	header_file->WriteLine("#endif");
}

std::string ScriptCompiler::Native_Data_Type(CDataType* type)
{
	if (dynamic_cast<CStringDataType*>(type) != NULL)
	{
		return "CVMString";
	}
	else if (dynamic_cast<CArrayDataType*>(type) != NULL || 
			 dynamic_cast<CObjectDataType*>(type) != NULL)
	{
		return "CVMObjectHandle";
	}
	else if (dynamic_cast<CIntDataType*>(type) != NULL ||
			 dynamic_cast<CBoolDataType*>(type) != NULL)
	{
		return "int";
	}
	else if (dynamic_cast<CFloatDataType*>(type) != NULL)
	{
		return "float";
	}
	else if (dynamic_cast<CVoidDataType*>(type) != NULL)
	{
		return "void";
	}
	else
	{
		DBG_ASSERT_STR(false, "Failed to translate native data type.");
	}

	return "";
}

void ScriptCompiler::Dump_Native_Member(std::string class_name, std::string native_class_name, CClassMemberASTNode* member, Stream* header_file, Stream* source_file, bool bind)
{
	if (member->IsNative == false ||
		member->MemberType != MemberType::Method)
	{
		return;
	}

	if (bind == true)
	{
		source_file->WriteString("\tvm->Bind_");
		if (member->IsStatic)
			source_file->WriteString("Function");
		else
			source_file->WriteString("Method");
		source_file->WriteString("<");
		source_file->WriteString(Native_Data_Type(member->ReturnType).c_str());
		for (std::vector<CVariableStatementASTNode*>::iterator iter = member->Arguments.begin(); iter != member->Arguments.end(); iter++)
		{
			CVariableStatementASTNode* var = *iter;
			source_file->WriteString(("," + Native_Data_Type(var->Type)).c_str());
		}
		source_file->WriteString(">(\"");
		source_file->WriteString(native_class_name.c_str());
		source_file->WriteString("\", \"");
		source_file->WriteString(member->MangledIdentifier.c_str());
		source_file->WriteString("\", &");
		source_file->WriteString(member->MangledIdentifier.c_str());
		source_file->WriteString(");");
		source_file->WriteLine("");
	}
	else
	{
		std::string result = Native_Data_Type(member->ReturnType) + " ";
		
		header_file->WriteString("\tstatic ");
		header_file->WriteString(Native_Data_Type(member->ReturnType).c_str());
		header_file->WriteString(" ");

		source_file->WriteString(Native_Data_Type(member->ReturnType).c_str());
		source_file->WriteString(" ");
		source_file->WriteString(class_name.c_str());
		source_file->WriteString("::");

		header_file->WriteString(member->MangledIdentifier.c_str());
		source_file->WriteString(member->MangledIdentifier.c_str());

		std::string parameters = "(CVirtualMachine* vm";
	
		if (member->IsStatic == false)
		{
			parameters += ", CVMValue self";
		}

		for (std::vector<CVariableStatementASTNode*>::iterator iter = member->Arguments.begin(); iter != member->Arguments.end(); iter++)
		{
			CVariableStatementASTNode* var = *iter;
			parameters += ", " + Native_Data_Type(var->Type) + " " + var->Identifier.c_str();
		}

		parameters += ")";

		header_file->WriteString(parameters.c_str());
		source_file->WriteString(parameters.c_str());

		header_file->WriteLine(";");

		source_file->WriteLine("");
		source_file->WriteLine("{");
		
		if (dynamic_cast<CStringDataType*>(member->ReturnType) != NULL)
		{
			source_file->WriteLine("\treturn \"\"; // TODO");
		}
		else if (dynamic_cast<CArrayDataType*>(member->ReturnType) != NULL || 
				 dynamic_cast<CObjectDataType*>(member->ReturnType) != NULL)
		{
			source_file->WriteLine("\treturn NULL; // TODO");
		}
		else if (dynamic_cast<CIntDataType*>(member->ReturnType) != NULL ||
				 dynamic_cast<CBoolDataType*>(member->ReturnType) != NULL)
		{
			source_file->WriteLine("\treturn 0; // TODO");
		}
		else if (dynamic_cast<CFloatDataType*>(member->ReturnType) != NULL)
		{
			source_file->WriteLine("\treturn 0.0f; // TODO");
		}
		else if (dynamic_cast<CVoidDataType*>(member->ReturnType) != NULL)
		{
			source_file->WriteLine("\treturn; // TODO");
		}
		else
		{
			DBG_ASSERT_STR(false, "Failed to translate native data type.");
		}

		source_file->WriteLine("}");
		source_file->WriteLine("");
	}
}