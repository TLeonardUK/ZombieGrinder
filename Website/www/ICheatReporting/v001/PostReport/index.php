<?php

require_once(dirname(__FILE__) . "/../../../lib/webapi.php");

require_steam_ticket();

$reporter 			= get_int64_param('steam_id');
$reported 			= get_int64_param('reported_user');
$reported_username 	= get_string_param('reported_username');
$timeframe 			= get_int_param('timeframe');
$description 		= get_string_param('description');
$category 			= get_int_param('category');
$demo				= get_file_param('demo');

$guid 				= store_file_param('demo', $demo_upload_folder_local);

database_connect();

# Insert our internal report information.
$report_id = database_insert_row(
	"cheat_reports",
	array(
		"report_time"			=> time(), 
		"report_ip"				=> $_SERVER['REMOTE_ADDR'],
		"reported_steamid"		=> $reported, 
		"reporter_steamid"		=> $reporter, 
		"description"			=> $description, 
		"timeframe"				=> $timeframe,
		"category"				=> $category,	 
		"demo_guid"				=> $guid,
		"steam_reportid"		=> -1,
		"reported_username"		=> $reported_username,
	)
);

# Create the report on the steam servers.
$steam_report_id = create_steam_cheat_report($reported, $reporter, $report_id);

# Update the database with the steam-id.
database_update_row(
	"cheat_reports",
	$report_id,
	array(
		"steam_reportid" => $steam_report_id,
	)	
);

// Update the reportee's report count.
$reported_user = get_cheat_user($reported);

database_update_row(
	"cheat_users",
	$reported_user['id'],
	array(
		"reports_recieved" => ($reported_user['reports_recieved'] + 1),
	)	
);

database_disconnect();

# Done!
finish_request(array(
	"report_id" => $report_id
));

?>