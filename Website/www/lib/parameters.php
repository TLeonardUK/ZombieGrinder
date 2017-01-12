<?php

function get_int_param($name)
{
	if (!isset($_REQUEST[$name]) || !is_numeric($_REQUEST[$name]))
	{
		fail_request("Expected integer parameter '{$name}'.");
	}
	return intval($_REQUEST[$name]);	
}

function get_int64_param($name)
{
	if (!isset($_REQUEST[$name]) || !is_numeric($_REQUEST[$name]))
	{
		fail_request("Expected integer parameter '{$name}'.");
	}
	return $_REQUEST[$name]; // We return int64's as strings, because fucking php.	
}

function get_string_param($name)
{
	if (!isset($_REQUEST[$name]) || !is_string($_REQUEST[$name]))
	{
		fail_request("Expected string parameter '{$name}'.");
	}
	return strval($_REQUEST[$name]);
}

function get_file_param($name)
{
	if (!isset($_FILES[$name]))
	{
		fail_request("Expected file parameter '{$name}'.");
	}
	return $_FILES[$name];
}

function store_file_param($name, $result_folder)
{
	if (!isset($_FILES[$name]))
	{
		fail_request("Expected file parameter '{$name}'.");
	}
	
	if ($_FILES[$name]['error'] != UPLOAD_ERR_OK)
	{
		fail_request("File uploaded failed with error {$_FILES[$name]['error']}.");
	}
	
	$file_guid = create_guid();
	
	if (move_uploaded_file($_FILES[$name]['tmp_name'], $result_folder . '/' . $file_guid) !== TRUE)	
	{
		fail_request("Internal error configuring uploaded file.");
	}		

	return $file_guid;	
}

?>