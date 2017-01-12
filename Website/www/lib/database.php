<?php

function database_connect()
{	
	global $database_host;
	global $database_name;
	global $database_user;
	global $database_pass;

	global $database_link;
	
	$database_link = new mysqli($database_host, $database_user, $database_pass, $database_name);
	if (!$database_link || $database_link->connect_error)
	{
		fail_request("Database unavailable.");		
	}	
}

function database_type_id($value)
{
	if (is_string($value))
	{
		return "s";
	}
	else if (is_float($value))
	{
		return "d";
	}
	else if (is_int($value) || is_bool($value))
	{
		return "i";
	}
	else
	{
		return "b";
	}
}

function database_bind_result($stmt, &$out) 
{
	$data = mysqli_stmt_result_metadata($stmt);
	$fields = array();
	$out = array();

	$count = 0;

	while($field = mysqli_fetch_field($data)) {
		$fields[$count] = &$out[$field->name];
		$count++;
	}	
	call_user_func_array(array(&$stmt, 'bind_result'), $fields);	
}

function database_insert_row($table, $fields)
{
	global $database_link;
	
	$field_names = "";
	$field_value_placeholders = "";
	$param_types = "";
	$param_values = array();
	
	foreach ($fields as $key => $value)
	{
		if ($field_names != "")
		{			
			$field_names .= ",";
		}
		$field_names .= $key;
		
		if ($field_value_placeholders != "")
		{			
			$field_value_placeholders .= ",";
		}
		$field_value_placeholders .= "?";
		
		$param_types .= database_type_id($value);
		$param_values[] = &$fields[$key];
	}
	
	$stmt = $database_link->prepare("INSERT INTO {$table} ({$field_names}) VALUES ({$field_value_placeholders})");
	if ($stmt === FALSE)
	{
		fail_request("SQL error in database_insert_row '" . $database_link->error . "'.");		
	}
	
	$params = array_merge(array(&$param_types), $param_values);
	call_user_func_array(array(&$stmt, 'bind_param'), $params);	
	
	$stmt->execute();
	$stmt->close();
	
	return $database_link->insert_id;
}

function database_update_row($table, $row_id, $fields)
{
	global $database_link;
	
	$field_names = "";
	$field_value_placeholders = "";
	$param_types = "";
	$param_values = array();
	
	foreach ($fields as $key => $value)
	{
		if ($field_names != "")
		{			
			$field_names .= ",";
		}
		$field_names .= $key . "=?";
		
		$param_types .= database_type_id($value);
		$param_values[] = &$fields[$key];
	}
	
	$param_types .= "i";
	$param_values[] = &$row_id;
	
	$stmt = $database_link->prepare("UPDATE {$table} SET {$field_names} WHERE id=?");
	if ($stmt === FALSE)
	{
		fail_request("SQL error in database_update_row '" . $database_link->error . "'.");		
	}
	
	$params = array_merge(array(&$param_types), $param_values);
	call_user_func_array(array(&$stmt, 'bind_param'), $params);		
	
	$stmt->execute();
	$rows_affected = $stmt->affected_rows;	
	$stmt->close();
	
	return $rows_affected;
}

function database_select($table, $fields)
{
	global $database_link;
	
	$field_names = "";
	$field_value_placeholders = "";
	$param_types = "";
	$param_values = array();
	
	foreach ($fields as $key => $value)
	{
		if ($field_names != "")
		{			
			$field_names .= " AND ";
		}
		$field_names .= $key . "=?";
		
		$param_types .= database_type_id($value);
		$param_values[] = &$fields[$key];
	}
	
	$stmt = $database_link->prepare("SELECT * FROM {$table} WHERE {$field_names}");
	if ($stmt === FALSE)
	{
		fail_request("SQL error in database_update_row '" . $database_link->error . "'.");		
	}
	
	$params = array_merge(array(&$param_types), $param_values);
	call_user_func_array(array(&$stmt, 'bind_param'), $params);		

	$output_row = array();
	database_bind_result($stmt, $output_row);
	
	$result = array();
	
	if ($stmt->execute())
	{
		while ($stmt->fetch())
		{
			$result[] = $output_row;
		}
	}
	$stmt->close();
	
	return $result;
}

function database_update_expr($table, $expr, $params, $fields)
{
	global $database_link;
	
	$field_names = "";
	$field_value_placeholders = "";
	$param_types = "";
	$param_values = array();
	
	foreach ($fields as $key => $value)
	{
		if ($field_names != "")
		{			
			$field_names .= ",";
		}
		$field_names .= $key . "=?";
		
		$param_types .= database_type_id($value);
		$param_values[] = &$fields[$key];
	}
	
	foreach ($params as $key => $value)
	{		
		$param_types .= database_type_id($value);
		$param_values[] = &$params[$key];
	}
	
	$stmt = $database_link->prepare("UPDATE {$table} SET {$field_names} WHERE {$expr}");
	if ($stmt === FALSE)
	{
		fail_request("SQL error in database_update_row '" . $database_link->error . "'.");		
	}
	
	$params = array_merge(array(&$param_types), $param_values);
	call_user_func_array(array(&$stmt, 'bind_param'), $params);		
	
	$stmt->execute();
	$rows_affected = $stmt->affected_rows;	
	$stmt->close();
	
	return $rows_affected;	
}

function database_select_expr($table, $expr, $params)
{
	global $database_link;
	
	$param_types = "";
	$param_values = array();
	
	foreach ($params as $key => $value)
	{		
		$param_types .= database_type_id($value);
		$param_values[] = &$params[$key];
	}
	
	$stmt = $database_link->prepare("SELECT * FROM {$table} WHERE {$expr}");
	if ($stmt === FALSE)
	{
		fail_request("SQL error in database_select_expr '" . $database_link->error . "'.");		
	}
	
	$params = array_merge(array(&$param_types), $param_values);
	call_user_func_array(array(&$stmt, 'bind_param'), $params);		

	$output_row = array();
	database_bind_result($stmt, $output_row);
	
	$result = array();
	
	if ($stmt->execute())
	{
		while ($stmt->fetch())
		{
			$result[] = $output_row;
		}
	}
	$stmt->close();
	
	return $result;	
}

function database_disconnect()
{
	global $database_link;
	
	if ($database_link)
	{
		$database_link->close();
	}
}
?>