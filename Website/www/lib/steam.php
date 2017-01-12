<?php

function steam_webapi_request($service, $method, $version, $request_method, $fields)
{	
	$fields_data = http_build_query($fields);
	
	$post_stream_context = stream_context_create(
		array(
			'http'=>array(
				'method' => $request_method,
				'header' => 'Content-type: application/x-www-form-urlencoded\r\n',
				'content' => $fields_data
			),
			'https'=>array(
				'method' => $request_method,
				'header' => 'Content-type: application/x-www-form-urlencoded\r\n',
				'content' => $fields_data
			)
		)
	);
	
	$get_stream_context = stream_context_create(
		array(
			'http'=>array(
				'method' => $request_method
			),
			'https'=>array(
				'method' => $request_method
			)
		)
	);	
	
	$data = "";
	if ($request_method == "GET")
	{
		$data = file_get_contents(
			"https://api.steampowered.com/{$service}/{$method}/{$version}/?" . $fields_data,
			False,
			$get_stream_context);
	}
	else
	{
		$data = file_get_contents(
			"https://api.steampowered.com/{$service}/{$method}/{$version}/",
			False,
			$post_stream_context);		
	}	
	

	$json_data = json_decode($data, true);
	if ($json_data == NULL)
	{
		fail_request("Stream failed to reply with valid json data to method {$service}/{$method}.");	
	}
	
	return $json_data;
}

function create_steam_cheat_report($reported_steamid, $reporter_steamid, $internal_report_id)
{
	global $steam_webapi_publisher_key;
	global $steam_webapi_appid;
	
	$request = steam_webapi_request(
		"ICheatReportingService", 
		"ReportPlayerCheating", 
		"v1",
		"POST", 
		array(
			"key" 					=> $steam_webapi_publisher_key,
			"steamid" 				=> $reported_steamid,
			"appid" 				=> $steam_webapi_appid,
			"steamidreporter" 		=> $reporter_steamid,
			"appdata" 				=> $internal_report_id,
			"heuristic" 			=> False,
			"detection" 			=> False,
			"playerreport" 			=> True,
			"noreportid" 			=> False,
			"gamemode" 				=> 0,
			"suspicionstarttime" 	=> 0,
			"severity" 				=> 0		
		)
	);	
	
	if (!isset($request['response']) ||
		!isset($request['response']['steamid']) ||
		!isset($request['response']['reportid']))
	{
		fail_request("Cheat report to steam failed.");		
	}
		
	return $request['response']['reportid'];
}

function create_steam_game_ban($reported_steamid, $duration, $description, $steam_report_id)
{
	global $steam_webapi_publisher_key;
	global $steam_webapi_appid;
	
	$request = steam_webapi_request(
		"ICheatReportingService", 
		"RequestPlayerGameBan", 
		"v1",
		"POST", 
		array(
			"key" 					=> $steam_webapi_publisher_key,
			"steamid" 				=> $reported_steamid,
			"appid" 				=> $steam_webapi_appid,
			"reportid"		 		=> $steam_report_id,
			"cheatdescription"		=> $description,
			"duration"				=> $duration,
			"delayban"				=> False,
			"flags"					=> 0
		)
	);	
	
	if (!isset($request['response']) ||
		!isset($request['response']['steamid']))
	{
		fail_request("Game ban creation failed.");		
	}
		
	return $request['response']['reportid'];	
}

function register_cheat_user($steam_id)
{
	$rows = database_select(
		"cheat_users",
		array(
			"steam_id" => $steam_id
		)
	);

	$user_id = 0;

	if (count($rows) <= 0)
	{
		$user_id = database_insert_row(
			"cheat_users",
			array(
				"steam_id"				=> $steam_id, 
				"first_seen_time"		=> time(), 
				"last_active_time"		=> time(), 
				"last_active_ip"		=> $_SERVER['REMOTE_ADDR'],
				"reports_recieved"		=> 0,
				"reports_upheld"		=> 0,
				"judgements_made"		=> 0, 
			)
		);
	}
	else
	{
		$user_id = $rows[0]['id'];
		database_update_row(
			"cheat_users",
			$user_id,
			array(
				"last_active_time"		=> time(), 
				"last_active_ip"		=> $_SERVER['REMOTE_ADDR'],			
			)	
		);
	}
	
	return $user_id;
}

function get_cheat_user($steam_id)
{
	$user_id = register_cheat_user($steam_id);
	
	$rows = database_select(
		"cheat_users",
		array(
			"id" => $user_id
		)
	);
	
	if (count($rows) > 0)
	{
		return $rows[0];
	}
	else
	{	
		fail_request("Failed to get matching steam user entry.");	
	}
}


function get_cheat_user_by_id($user_id)
{
	$rows = database_select(
		"cheat_users",
		array(
			"id" => $user_id
		)
	);
	
	if (count($rows) > 0)
	{
		return $rows[0];
	}
	else
	{	
		fail_request("Failed to get matching steam user entry.");	
	}
}

?>