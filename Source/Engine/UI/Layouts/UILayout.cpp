// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/UI/Layouts/UILayoutFactory.h"
#include "Engine/UI/Layouts/UILayout.h"
#include "Engine/UI/Layouts/UILayoutHandle.h"

#include "Engine/UI/UIScene.h"
#include "Engine/UI/UIElement.h"

#include "Engine/Platform/Platform.h"

#include "Engine/Config/ConfigFile.h"

#include "Generic/Helper/StringHelper.h"

#include <typeinfo>

UILayoutElementDefinition::~UILayoutElementDefinition()
{	
	for (std::vector<UILayoutElementDefinition*>::iterator iter = Children.begin(); iter != Children.end(); iter++)
	{
		SAFE_DELETE(*iter);
	}
	Children.clear();
}

UILayout::UILayout()
	: m_root_element(NULL)
{
}

UILayout::~UILayout()
{
	SAFE_DELETE(m_root_element);
}

bool UILayout::Check_Element_For_Errors(UIElement* element) const
{
	std::string val = element->Get_Value();
	if (val != "" && val[0] != '#')
	{
		DBG_LOG("[WARNING] Layout element '%s' contains non-localized value '%s'.", element->Get_Name().c_str(), val.c_str());
	}

	return true;
}

UIElement* UILayout::Instantiate_Element(const UILayoutElementDefinition* def, UIScene* scene) const
{
#define ELEMENT_START(name, classname) \
		{ \
			const UILayoutElementDefinition_##classname* state = dynamic_cast<const UILayoutElementDefinition_##classname*>(def); \
			if (state != 0) \
			{ \
				classname* element = new classname(); \
				element->Set_Scene(scene);
#define ELEMENT_END() \
				element->m_name_hash = StringHelper::Hash(element->m_name.c_str()); \
				for (std::vector<UILayoutElementDefinition*>::const_iterator iter = def->Children.begin(); iter != def->Children.end(); iter++) \
				{ \
					element->Add_Child(Instantiate_Element(*iter, scene)); \
				} \
				if (!Check_Element_For_Errors(element)) \
				{ \
					return NULL; \
				} \
				return element;	\
			} \
		}
#define ELEMENT_PROPERTY(name, fieldname, type, required) \
	if (state->fieldname##_exists == true) \
			{ \
				element->fieldname = state->fieldname; \
			}
#define ELEMENT_PROPERTY_ENUM_START(name, fieldname, type, required) \
			ELEMENT_PROPERTY(name, fieldname, type, required)
#define ELEMENT_PROPERTY_ENUM_END()
#define ELEMENT_ENUM_ENTRY(type, name) 
#define ELEMENT_ALLOW_CHILD(x)
#define ELEMENT_ALLOW_ANY_CHILD()
#define ELEMENT_FORCE_MAX_CHILDREN(x)
#define ELEMENT_MAX_CHILDREN(x)

#include "Engine/UI/Elements/UIElementDefinitions.inc"
	
#undef ELEMENT_ALLOW_CHILD
#undef ELEMENT_ALLOW_ANY_CHILD
#undef ELEMENT_FORCE_MAX_CHILDREN
#undef ELEMENT_MAX_CHILDREN
#undef ELEMENT_ENUM_ENTRY
#undef ELEMENT_PROPERTY_ENUM_START
#undef ELEMENT_PROPERTY_ENUM_END
#undef ELEMENT_PROPERTY
#undef ELEMENT_END
#undef ELEMENT_START

	// dafuq
	DBG_ASSERT_STR(false, "Internal error - Could not determine type of element definition.");
	return NULL;
}

std::vector<UIElement*> UILayout::Instantiate(UIScene* scene) const
{
	std::vector<UIElement*> elements;
	
	for (std::vector<UILayoutElementDefinition*>::iterator iter = m_root_element->Children.begin(); iter != m_root_element->Children.end(); iter++)
	{
		UIElement* element = Instantiate_Element(*iter, scene);
		elements.push_back(element);
	}

	return elements;
}

bool UILayout::Load_Element(ConfigFile* file, ConfigFileNode root_node, UILayoutElementDefinition*& result)
{
	const char* type = file->Get<const char*>("type", root_node, true);
	const char* element_name = "";
	const char* enum_name = "";
	bool enum_required;

	bool force_max_children = false;
	int  max_children = -1;
	bool allow_any_child = true;
	std::vector<std::string> allowed_child_types;

	if (file->Contains("name", root_node, true))
	{
		element_name = file->Get<const char*>("name", root_node, true);
	}

	// =====================================================================================
	//  This ugly block of crazy is responsible for loading element properties
	//  based on the schema defined in UIElementDefinitions.inc
	// =====================================================================================
#define ELEMENT_START(name, classname) \
		if (stricmp(type, #name) == 0) \
		{ \
			UILayoutElementDefinition_##classname* state = new UILayoutElementDefinition_##classname(); 
#define ELEMENT_END() \
			result = state; \
			result->Type = type; \
		} \
		else
#define ELEMENT_PROPERTY(name, fieldname, type, required)	\
	if (file->Contains(#name, root_node, true)) \
		{ \
			state->fieldname = file->Get<type>(#name, root_node, true); \
			state->fieldname##_exists = true; \
		} \
		else \
		{ \
			state->fieldname##_exists = false; \
			if (required) \
			{ \
				DBG_LOG("Element '%s' does not contain required property '%s'.", element_name, #name); \
				SAFE_DELETE(state); \
				return false; \
			} \
		}
#define ELEMENT_PROPERTY_ENUM_START(name, fieldname, type, required)	\
		enum_name = #name; \
		enum_required = required; \
		state->fieldname##_exists = false; \
		if (file->Contains(#name, root_node, true)) \
		{ \
			const char* enum_value = file->Get<const char*>(#name, root_node, true); \
			type::Type& result_ptr = state->fieldname; \
			state->fieldname##_exists = true; 

#define ELEMENT_ENUM_ENTRY(type, name) \
			if (stricmp(enum_value, #name) == 0) \
			{ \
				result_ptr = type::name;\
			}

#define ELEMENT_PROPERTY_ENUM_END() \
		} \
		else \
		{ \
			if (enum_required) \
			{ \
				DBG_LOG("Element does not contain required property '%s'.", enum_name); \
				SAFE_DELETE(state); \
				return false; \
			} \
		}
#define ELEMENT_ALLOW_CHILD(x) \
		allowed_child_types.push_back(#x);  
#define ELEMENT_ALLOW_ANY_CHILD()
		allow_any_child = true;
#define ELEMENT_FORCE_MAX_CHILDREN(x) \
		force_max_children = (x);
#define ELEMENT_MAX_CHILDREN(x) \
		max_children = (x);

#include "Engine/UI/Elements/UIElementDefinitions.inc"
	
#undef ELEMENT_ALLOW_CHILD
#undef ELEMENT_ALLOW_ANY_CHILD
#undef ELEMENT_FORCE_MAX_CHILDREN
#undef ELEMENT_MAX_CHILDREN
#undef ELEMENT_ENUM_ENTRY
#undef ELEMENT_PROPERTY_ENUM_START
#undef ELEMENT_PROPERTY_ENUM_END
#undef ELEMENT_PROPERTY
#undef ELEMENT_END
#undef ELEMENT_START

	// Else block
	{
		DBG_LOG("Invalid element type '%s'.", type);
		return false;
	}

	// Load element children (if there are any).
	if (!Load_Elements(file, result, file->Get<std::vector<ConfigFileNode> >("element", root_node)))
	{
		return false;
	}
	
	// Check we have the correct number of children.
	if (max_children >= 0 && (int)result->Children.size() > max_children)
	{
		DBG_LOG("Element '%s' contains to many children, max children allowed for this element is %i.", type, max_children);
		return false;
	}
	if (force_max_children == true && (int)result->Children.size() != max_children)
	{
		DBG_LOG("Element '%s' contains to invalid number of children, there must be %i children for this element.", type, max_children);
		return false;
	}

	// Check children are of valid types.
	if (allow_any_child == false)
	{
		for (std::vector<UILayoutElementDefinition*>::iterator iter = result->Children.begin(); iter != result->Children.end(); iter++)
		{
			bool found = false;
			UILayoutElementDefinition* def = *iter;

			for (std::vector<std::string>::iterator iter2 = allowed_child_types.begin(); iter2 != allowed_child_types.end(); iter2++)
			{
				if (*iter2 == def->Type)
				{
					found = true;
					break;
				}
			}

			if (found == false)
			{
				DBG_LOG("Element '%s' contains invalid child element '%s'.", type, def->Type.c_str());
				return false;
			}
		}
	}

	return true;
}

bool UILayout::Load_Elements(ConfigFile* file, UILayoutElementDefinition* root, std::vector<ConfigFileNode> children)
{
	for (std::vector<ConfigFileNode>::iterator iter = children.begin(); iter != children.end(); iter++)
	{
		UILayoutElementDefinition* result = NULL;

		if (!Load_Element(file, *iter, result))
		{
			SAFE_DELETE(result);
			return false;
		}

		root->Children.push_back(result);
	}
	return true;
}

UILayout* UILayout::Load(const char* url)
{
	Platform* platform = Platform::Get();

	// Compile time.
	DBG_LOG("Loading layout resource '%s'.", url);
	
	// Load configuration settings.
	ConfigFile config;
	if (!config.Load(url))
	{
		DBG_LOG("Failed to load layout, config file could not be found: '%s'", url);
		return NULL;
	}

	// Create the layout.
	UILayout* layout = new UILayout();
	
	// Load settings.
	layout->m_name = config.Get<std::string>("settings/name");

	// Load layout.
	layout->m_root_element = new UILayoutElementDefinition();
	if (!layout->Load_Elements(&config, layout->m_root_element, config.Get<std::vector<ConfigFileNode> >("layout/element")))
	{
		DBG_LOG("Failed to load layout, element configuration is invalid: '%s'", url);
		SAFE_DELETE(layout);
		return NULL;
	}

	return layout;
}

std::string UILayout::Get_Name()
{
	return m_name;
}