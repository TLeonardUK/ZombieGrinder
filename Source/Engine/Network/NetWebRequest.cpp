// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Network/NetWebRequest.h"

#include "Generic/Helper/StringHelper.h"

NetWebRequest::NetWebRequest()
	: m_thread(NULL)
	, m_shutting_down(false)
	, m_active(false)
	, m_was_success(false)
	, m_progress(0.0f)
{
}

NetWebRequest::~NetWebRequest()
{
	if (m_thread)
	{
		m_shutting_down = true;
		m_thread->Join();
		SAFE_DELETE(m_thread);
	}
}

void NetWebRequest::Add_URI_Parameter(std::string name, std::string value)
{
	NetWebRequestParameter param;
	param.name = name;
	param.is_file = false;
	param.buffer.Reserve(value.size(), false);
	memcpy(param.buffer.Buffer(), value.c_str(), value.size());
	m_uri_params.push_back(param);
}

void NetWebRequest::Add_URI_Parameter(std::string name, DataBuffer buffer)
{
	NetWebRequestParameter param;
	param.name = name;
	param.buffer = buffer;
	param.is_file = false;
	m_uri_params.push_back(param);
}

void NetWebRequest::Add_URI_Parameter(std::string name, int value)
{
	Add_URI_Parameter(name, StringHelper::To_String(value));
}

void NetWebRequest::Add_Body_Parameter(std::string name, std::string value)
{
	NetWebRequestParameter param;
	param.name = name;
	param.is_file = false;
	param.buffer.Reserve(value.size(), false);
	memcpy(param.buffer.Buffer(), value.c_str(), value.size());
	m_body_params.push_back(param);
}

void NetWebRequest::Add_Body_Parameter(std::string name, DataBuffer buffer)
{
	NetWebRequestParameter param;
	param.name = name;
	param.buffer = buffer;
	param.is_file = true;
	m_body_params.push_back(param);
}

void NetWebRequest::Add_Body_Parameter(std::string name, int value)
{
	Add_Body_Parameter(name, StringHelper::To_String(value));
}

bool NetWebRequest::Post(std::string host, int port, bool use_ssl, std::string uri)
{
	DBG_ASSERT(m_thread == NULL);

	m_type = NetWebRequestType::Post;
	m_host = host;
	m_port = port;
	m_use_ssl = use_ssl;
	m_uri = uri;
	m_was_success = false;
	m_active = true;
	m_thread = Thread::Create("WebRequest", &NetWebRequest::Static_Entry_Point, this);
	m_thread->Start();

	return true;
}

bool NetWebRequest::Get(std::string host, int port, bool use_ssl, std::string uri)
{
	DBG_ASSERT(m_thread == NULL);

	m_type = NetWebRequestType::Get;
	m_host = host;
	m_port = port;
	m_use_ssl = use_ssl;
	m_uri = uri;
	m_was_success = false;
	m_active = true;
	m_thread = Thread::Create("WebRequest", &NetWebRequest::Static_Entry_Point, this);
	m_thread->Start();

	return true;
}

void NetWebRequest::Cancel()
{
	m_was_success = false;
	m_shutting_down = true;
}

bool NetWebRequest::Poll()
{
	if (m_thread)
	{
		if (!m_active)
		{
			SAFE_DELETE(m_thread);
			return true;
		}
	}
	return false;
}

bool NetWebRequest::Was_Success()
{
	return m_was_success;
}

DataBuffer NetWebRequest::Get_Result()
{
	return m_result;
}

std::string NetWebRequest::Url_Encode(std::string input)
{
	int buffer_size = input.size() * 2;
	while (true)
	{
		char* buffer = new char[buffer_size];
		int result = mg_url_encode(input.c_str(), buffer, buffer_size);
		if (result >= 0)
		{
			std::string result = buffer;
			SAFE_DELETE(buffer);
			return result;
		}
		else
		{
			SAFE_DELETE(buffer);
			buffer_size *= 2;
		}
	}
}

void NetWebRequest::Entry_Point(Thread* self)
{
#define POST_BOUNDRY "----ZomGriFormBoundaryePkpFF7tjBAqx29L"

	// Build out encoded uri.
	std::string uri = m_uri;

	for (std::vector<NetWebRequestParameter>::iterator iter = m_uri_params.begin(); iter != m_uri_params.end(); iter++)
	{
		NetWebRequestParameter& param = *iter;

		std::string encoded_name = Url_Encode(param.name);
		std::string encoded_value = Url_Encode(param.buffer.Buffer());

		if (iter == m_uri_params.begin())
		{
			uri += "?";
		}
		else
		{
			uri += "&";
		}

		uri += encoded_name + "=" + encoded_value;	
	}

	std::string post_data;

	for (std::vector<NetWebRequestParameter>::iterator iter = m_body_params.begin(); iter != m_body_params.end(); iter++)
	{
		NetWebRequestParameter& param = *iter;
		post_data += "--" POST_BOUNDRY "\r\n";
		if (param.is_file)
		{
			post_data += StringHelper::Format("Content-Disposition: attachment; name=\"%s\"; filename=\"%s\"\r\n", param.name.c_str(), param.name.c_str());
		}
		else
		{
			post_data += StringHelper::Format("Content-Disposition: form-data; name=\"%s\"\r\n", param.name.c_str());
		}
		post_data += "Content-Type: application/octet-stream\r\n";
		post_data += "\r\n";
		post_data.append(param.buffer.Buffer(), param.buffer.Buffer() + param.buffer.Size());
		post_data += "\r\n";
	}

	post_data += "--" POST_BOUNDRY "--\r\n";

	// Build out request.
	std::string request_data;
	
	if (m_type == NetWebRequestType::Post)
	{
		request_data += StringHelper::Format("POST %s HTTP/1.1\r\n", uri.c_str());
	}
	else 
	{
		request_data += StringHelper::Format("GET %s HTTP/1.1\r\n", uri.c_str());
	}
	request_data += StringHelper::Format("Host: %s\r\n", m_host.c_str());
	request_data += "Content-Type: multipart/form-data; boundary=" POST_BOUNDRY "\r\n";
	request_data += StringHelper::Format("Content-Length: %d\r\n", post_data.size());
	request_data += "\r\n";
	request_data += post_data;

	// Make connection.
	char error_buffer[4096];
	memset(error_buffer, 0, sizeof(error_buffer));

	mg_connection* connection = mg_connect_client(
		m_host.c_str(), 
		m_port, 
		m_use_ssl, 
		error_buffer,
		sizeof(error_buffer)
	);

	if (!connection)
	{
		DBG_LOG("Failed to open web request with error: %s", error_buffer);
		m_was_success = false; 
		m_active = false; 
		return;
	}

	mg_set_connection_timeout(connection, 15 * 1000);

	// Write the request.
	int chunk_offset = 0;
	while (true)
	{
		int chunk_size = Min(16 * 1024, request_data.size() - chunk_offset);
		if (chunk_size <= 0)
		{
			break;
		}

		int len = mg_write(connection, request_data.c_str() + chunk_offset, chunk_size);
		if (len == 0)
		{
			DBG_LOG("Socket was closed prior to request finished sending.");
			mg_close_connection(connection);
			m_was_success = false;
			m_active = false;
			return;

		}
		else if (len < 0)
		{
			DBG_LOG("Failed to write web request with write-error %i.", len);
			mg_close_connection(connection);
			m_was_success = false;
			m_active = false;
			return;

		}
		else
		{
			chunk_offset += len;
		}

	//	DBG_LOG("Upload Progress: %i/%i (%.2f)", chunk_offset, request_data.size(), ((float)chunk_offset / (float)request_data.size()) * 100.0f);
	}

	// Wait for response.
	int err = mg_get_response(connection, error_buffer, sizeof(error_buffer), -1);
	if (err < 0)
	{
		DBG_LOG("Failed to wait for web response: %s", error_buffer);
		mg_close_connection(connection);
		m_was_success = false;
		m_active = false;
		return;
	}

	// Poll until it finishes.
	DataBuffer buffer;

	int offset = 0;
	char read_buffer[1024];
	while (true)
	{
		int len = mg_read(connection, read_buffer, sizeof(read_buffer));

		if (mg_get_content_len(connection) > 0)
		{
			m_progress = (float)offset / (float)mg_get_content_len(connection);
		}

		// At end.
		if (len == 0)
		{
			break;
		}

		// Read error.
		else if (len < 0)
		{
			DBG_LOG("Failed to read web request response with read-error %i.", len);
			mg_close_connection(connection);
			m_was_success = false;
			m_active = false;
			return;

		}

		// Data!
		else
		{
			buffer.Reserve(buffer.Size() + len + 1, true);
			memcpy(buffer.Buffer() + offset, read_buffer, len);
			offset += len;

			// Null terminate.
			buffer.Buffer()[offset] = '\0';
		}
	}
	
	// Parse result.
	m_result = buffer;

	// Done
	mg_close_connection(connection);
	m_was_success = true;
	m_active = false;
	return;

#undef POST_BOUNDRY
}

void NetWebRequest::Crack_URL(std::string url, std::string& protocol, std::string& host, int& port, std::string& uri)
{
	protocol = "http";
	port = 80;
	uri = "/";

	// Remove https.
	size_t offset = url.find("://");
	if (offset != std::string::npos)
	{
		protocol = url.substr(0, offset);
		url = url.substr(offset + 3);
	}

	// Look for first forward slash to seperate uri from host.
	size_t slash_offset = url.find("/");

	std::string host_segment = url;

	if (slash_offset != std::string::npos)
	{
		host_segment = url.substr(0, slash_offset);
		uri = url.substr(slash_offset);
	}

	// Split host by port
	size_t port_offset = host_segment.find(":");
	if (port_offset != std::string::npos)
	{
		host = host_segment.substr(0, port_offset);
		port = atoi(host_segment.substr(port_offset + 1).c_str());
	}
	else
	{
		host = host_segment;
	}
}

float NetWebRequest::Get_Progress()
{
	return m_progress;
}