// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_CONFIGFILE_
#define _ENGINE_CONFIGFILE_

#include "Generic/Types/IntVector3.h"
#include "Generic/Types/Vector3.h"
#include "Generic/Types/Vector2.h"
#include "Generic/Types/Rect2D.h"
#include "Generic/Types/Color.h"

#include "Generic/Helper/StringHelper.h"

#include "Generic/ThirdParty/RapidXML/rapidxml.hpp"
#include "Generic/ThirdParty/RapidXML/rapidxml_iterators.hpp"
#include "Generic/ThirdParty/RapidXML/rapidxml_utils.hpp"
#include "Generic/ThirdParty/RapidXML/rapidxml_print.hpp"

#include <cstring>
 
typedef rapidxml::xml_node<>* ConfigFileNode;

struct ConfigFileBuffer
{
	MEMORY_ALLOCATOR(ConfigFileBuffer, "Config");

private:
	friend class ConfigFile;

	rapidxml::xml_document<>*	m_xml_document;
	char*						m_source_buffer;
	int							m_source_buffer_len;

};

class ConfigFile
{
	MEMORY_ALLOCATOR(ConfigFile, "Config");

private:
	rapidxml::xml_document<>*	m_xml_document;
	char*						m_source_buffer;
	int							m_source_buffer_len;

	std::string					m_root_element;

private:

	// Memory management.
	bool Resize_Buffer(int size);

public:
	
	// Constructor!
	ConfigFile();
	ConfigFile(const ConfigFile& other);
	~ConfigFile();

	ConfigFileBuffer Get_Buffer();
	void Set_Buffer(ConfigFileBuffer buffer);

	void Set_Root_Element(std::string root)
	{
		m_root_element = root;
	}

	// Save & load options.
	bool Save(const char* path);
	bool Save(std::string& result);
	bool Load(const char* path);
	bool Load(const char* buffer, int size);
	
	// Used by derived classes to unpack and repack data members for saving/loading.
	virtual void Pack  (ConfigFile& file);
	virtual void Unpack(ConfigFile& file);
	
	// ==============================================================
	// Node functions
	// ==============================================================
	ConfigFileNode Get_Node(const char* name, bool create = false, bool force_create_last = false, ConfigFileNode base_node = NULL, bool error_out = true)
	{
		rapidxml::xml_node<>* element = base_node != NULL ? base_node : m_xml_document->first_node(m_root_element.c_str());
		if (element == NULL)
		{
			if (create == true)
			{
				element = m_xml_document->allocate_node(rapidxml::node_element, m_xml_document->allocate_string(m_root_element.c_str()), NULL);
				m_xml_document->append_node(element);
			}
			else
			{
				if (error_out == true)
				{
					DBG_ASSERT_STR(false, "Expecting config node '%s'.", m_root_element.c_str());
				}
				else
				{
					return NULL;
				}
			}
		}

		const int name_len = strlen(name);
		std::string node_name = "";

		for (int i = 0; i < name_len; i++)
		{
			char chr = name[i];
			if (chr == '\\' || chr == '/')
			{
				rapidxml::xml_node<>* new_element = element->first_node(node_name.c_str(), 0, false);
				if (new_element == NULL)
				{
					if (create == true)
					{
						new_element = m_xml_document->allocate_node(rapidxml::node_element, m_xml_document->allocate_string(node_name.c_str()), NULL);
						element->append_node(new_element);
					}
					else
					{
						if (error_out == true)
						{
							DBG_ASSERT_STR(false, "Expecting config node '%s'.", node_name.c_str());
						}
						else
						{
							return NULL;
						}
					}
				}

				element = new_element;
				node_name = "";
			}
			else
			{
				node_name += chr;
			}
		}

		if (node_name != "")
		{
			rapidxml::xml_node<>* new_element = NULL;

			if (force_create_last == true)
			{
				new_element = m_xml_document->allocate_node(rapidxml::node_element, m_xml_document->allocate_string(node_name.c_str()), NULL);
				element->append_node(new_element);
			}
			else
			{
				new_element = element->first_node(node_name.c_str(), 0, false);
				if (new_element == NULL)
				{
					if (create == true)
					{
						new_element = m_xml_document->allocate_node(rapidxml::node_element, m_xml_document->allocate_string(node_name.c_str()), NULL);
						element->append_node(new_element);
					}
					else
					{
						if (error_out == true)
						{
							DBG_ASSERT_STR(false, "Expecting config node '%s'.", node_name.c_str());
						}
						else
						{
							return NULL;
						}
					}
				}
			}

			element = new_element;
		}

		return element;
	}
	
	ConfigFileNode New_Node(const char* name, ConfigFileNode node = NULL)
	{
		return Get_Node(name, true, true, node);
	}
	
	template<typename T>
	ConfigFileNode New_Node(const char* name, T value, ConfigFileNode node = NULL)
	{
		ConfigFileNode new_node = Get_Node(name, true, true, node);
		Set(name, value, new_node, false);
		return new_node;
	}
	
	bool Contains(const char* name, ConfigFileNode node = NULL, bool as_attribute = false)
	{
		if (as_attribute == true)
		{
			rapidxml::xml_attribute<>* attribute = node->first_attribute(name, 0, false);
			return (attribute != NULL);
		}
		else
		{
			ConfigFileNode n = Get_Node(name, false, false, node, false);
			return (n != NULL);
		}
	}

	void Clear_Nodes(const char* name)
	{
		rapidxml::xml_node<>* element = m_xml_document->first_node(m_root_element.c_str());

		const int name_len = strlen(name);
		std::string node_name = "";

		for (int i = 0; i < name_len; i++)
		{
			char chr = name[i];
			if (chr == '\\' || chr == '/')
			{
				rapidxml::xml_node<>* new_element = element->first_node(node_name.c_str(), 0, false);
				if (new_element == NULL)
				{
					return;
				}
				element = new_element;
				node_name = "";
			}
			else
			{
				node_name += chr;
			}
		}

		if (node_name != "")
		{
			rapidxml::xml_node<>* new_element = NULL;

			new_element = element->first_node(node_name.c_str(), 0, false);
			while (new_element != NULL)
			{			
				rapidxml::xml_node<>* remove_element = new_element;

				new_element = new_element->next_sibling(node_name.c_str(), 0, false);
			
				element->remove_node(remove_element);
			}
		}
	}

	// ==============================================================
	// Get functions
	// ==============================================================
	template<typename T>
	T Get(const char* name, ConfigFileNode node = NULL, bool as_attribute = false)
	{
		T val;
		Get_Primitive(name, node, as_attribute, &val);
		return val;
	}
	
	void Get_Primitive(const char* name, ConfigFileNode node, bool as_attribute, std::string* val)
	{
		*val = Get<const char*>(name, node, as_attribute);
	}

	void Get_Primitive(const char* name, ConfigFileNode node, bool as_attribute, Color* val)
	{
		const char* source = Get<const char*>(name, node, as_attribute);
		Color result;
		if (!Color::Parse(source, result))
		{
			DBG_ASSERT_STR(false, "Config node '%s' was in invalid format.", name);
		}
		*val = result;
	}

	void Get_Primitive(const char* name, ConfigFileNode node, bool as_attribute, Rect2D* val)
	{
		const char* source = Get<const char*>(name, node, as_attribute);
		Rect2D result;
		if (!Rect2D::Parse(source, result))
		{
			DBG_ASSERT_STR(false, "Config node '%s' was in invalid format.", name);
		}
		*val = result;
	}

	void Get_Primitive(const char* name, ConfigFileNode node, bool as_attribute, Vector2* val)
	{
		const char* source = Get<const char*>(name, node, as_attribute);
		Vector2 result;
		if (!Vector2::Parse(source, result))
		{
			DBG_ASSERT_STR(false, "Config node '%s' was in invalid format.", name);
		}
		*val = result;
	}
	
	void Get_Primitive(const char* name, ConfigFileNode node, bool as_attribute, const char** val)
	{
		if (as_attribute == true)
		{
			DBG_ASSERT(node != NULL);
			
			rapidxml::xml_attribute<>* attribute = node->first_attribute(name, 0, false);			
			DBG_ASSERT_STR(attribute != NULL, "Expecting config node attribute '%s'.", name);

			*val = attribute->value();
		}
		else
		{
//			DBG_ASSERT(node == NULL);

			ConfigFileNode n = Get_Node(name, false, false, node);
			*val = n->value();
		}
	}
	
	void Get_Primitive(const char* name, ConfigFileNode node, bool as_attribute, int* val)
	{
		const char* source = Get<const char*>(name, node, as_attribute);
		*val = atoi(source);
	}
	
	void Get_Primitive(const char* name, ConfigFileNode node, bool as_attribute, unsigned int* val)
	{
		const char* source = Get<const char*>(name, node, as_attribute);
		unsigned int s = 0;
		if (sscanf(source, "%u", &s) == EOF)
		{
			s = 0;
		}
		*val = s;
	}

	void Get_Primitive(const char* name, ConfigFileNode node, bool as_attribute, bool* val)
	{
		const char* source = Get<const char*>(name, node, as_attribute);
		*val = (stricmp(source, "0") == 0 || stricmp(source, "false") == 0) ? false : true;
	}

	void Get_Primitive(const char* name, ConfigFileNode node, bool as_attribute, float* val)
	{
		const char* source = Get<const char*>(name, node, as_attribute);
		*val = (float)atof(source);
	}

	void Get_Primitive(const char* name, ConfigFileNode node, bool as_attribute, std::vector<const char*>* val)
	{
		std::vector<const char*> result;

		const rapidxml::xml_node<>* element = m_xml_document->first_node(m_root_element.c_str());
		DBG_ASSERT(element != NULL);

		const int name_len = strlen(name);
		std::string node_name = "";

		for (int i = 0; i < name_len; i++)
		{
			char chr = name[i];
			if (chr == '\\' || chr == '/')
			{
				element = element->first_node(node_name.c_str(), 0, false);
				DBG_ASSERT_STR(element != NULL, "Expecting config node '%s'.", node_name.c_str());

				node_name = "";
			}
			else
			{
				node_name += chr;
			}
		}

		// Iterate over results.
		DBG_ASSERT(node_name != "");

		element = element->first_node(node_name.c_str(), 0, false);
		while (element != NULL)
		{
			result.push_back(element->value());
			element = element->next_sibling(node_name.c_str(), 0, false);
		}
	
		*val = result;		
	}
	
	void Get_Primitive(const char* name, ConfigFileNode node, bool as_attribute, std::vector<ConfigFileNode>* val)
	{
		std::vector<ConfigFileNode> result;

		const rapidxml::xml_node<>* element = node != NULL ? node : m_xml_document->first_node(m_root_element.c_str());
		DBG_ASSERT(element != NULL);

		const int name_len = strlen(name);
		std::string node_name = "";

		for (int i = 0; i < name_len; i++)
		{
			char chr = name[i];
			if (chr == '\\' || chr == '/')
			{
				element = element->first_node(node_name.c_str(), 0, false);
				DBG_ASSERT_STR(element != NULL, "Expecting config node '%s'.", node_name.c_str());

				node_name = "";
			}
			else
			{
				node_name += chr;
			}
		}

		// Iterate over results.
		DBG_ASSERT(node_name != "");

		element = element->first_node(node_name.c_str(), 0, false);
		while (element != NULL)
		{
			result.push_back((ConfigFileNode)element);
			element = element->next_sibling(node_name.c_str(), 0, false);
		}
	
		*val = result;		
	}
	
	std::vector<ConfigFileNode> Get_Children(ConfigFileNode node)
	{
		std::vector<ConfigFileNode> result;

		rapidxml::xml_node<>* element = node != NULL ? node : m_xml_document->first_node(m_root_element.c_str());
		DBG_ASSERT(element != NULL);

		element = element->first_node(NULL, 0, false);
		while (element != NULL)
		{
			result.push_back((ConfigFileNode)element);
			element = element->next_sibling(NULL, 0, false);
		}
	
		return result;		
	}

	std::vector<const char*> Get_Attribute_Names(ConfigFileNode node)
	{
		DBG_ASSERT(node != NULL);
	
		std::vector<const char*> results;
	
		rapidxml::xml_attribute<>* attribute = node->first_attribute(NULL, 0, false);
		while (attribute != NULL)
		{
			results.push_back(attribute->name());
			attribute = attribute->next_attribute(NULL, 0, false);
		}

		return results;
	}
	
	const char* Get_Name(ConfigFileNode node)
	{
		DBG_ASSERT(node != NULL);
	
		return node->name();
	}

	// ==============================================================
	// Set functions
	// ==============================================================
	template<typename T>
	void Set(const char* name, T value, ConfigFileNode node = NULL, bool as_attribute = false)
	{
		Set_Primitive(name, value, node, as_attribute);		
	}

	/*
	template<typename T>
	void Set(const char* name, T value, ConfigFileNode node = NULL, bool as_attribute = false)
	{
		Set(name, value.To_String().c_str(), node, as_attribute);		
	}
	*/

	void Set_Primitive(const char* name, Color value, ConfigFileNode node = NULL, bool as_attribute = false)
	{
		Set(name, value.To_String().c_str(), node, as_attribute);		
	}
	
	void Set_Primitive(const char* name, Rect2D value, ConfigFileNode node = NULL, bool as_attribute = false)
	{
		Set(name, value.To_String().c_str(), node, as_attribute);		
	}
	
	void Set_Primitive(const char* name, Vector2 value, ConfigFileNode node = NULL, bool as_attribute = false)
	{
		Set(name, value.To_String().c_str(), node, as_attribute);		
	}

	void Set_Primitive(const char* name, const char* value, ConfigFileNode node, bool as_attribute)
	{
		if (node == NULL)
		{
			node = Get_Node(name, true);
		}
		/*else
		{
			DBG_ASSERT(as_attribute == true);
		}*/

		if (as_attribute == true)
		{
			rapidxml::xml_attribute<>* attr = node->first_attribute(name, 0, false);
			if (attr == NULL)
			{
				attr = m_xml_document->allocate_attribute(m_xml_document->allocate_string(name), m_xml_document->allocate_string(value));
				node->append_attribute(attr);
			}
			attr->value(m_xml_document->allocate_string(value));
		}
		else
		{
			node->value(m_xml_document->allocate_string(value));
		}
	}
	
	void Set_Primitive(const char* name, std::string value, ConfigFileNode node, bool as_attribute)
	{
		Set(name, value.c_str(), node, as_attribute);	
	}

	void Set_Primitive(const char* name, int value, ConfigFileNode node, bool as_attribute)
	{
		Set(name, StringHelper::To_String(value).c_str(), node, as_attribute);
	}

	void Set_Primitive(const char* name, unsigned int value, ConfigFileNode node, bool as_attribute)
	{
		Set(name, StringHelper::To_String(value).c_str(), node, as_attribute);
	}
	
	void Set_Primitive(const char* name, bool value, ConfigFileNode node, bool as_attribute)
	{
		Set(name, value == true ? "true" : "false", node, as_attribute);
	}

	void Set_Primitive(const char* name, float value, ConfigFileNode node, bool as_attribute)
	{
		Set(name, StringHelper::To_String(value).c_str(), node, as_attribute);
	}

};

#endif

