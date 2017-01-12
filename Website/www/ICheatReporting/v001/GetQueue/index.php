<?php

require_once(dirname(__FILE__) . "/../../../lib/webapi.php");

require_steam_ticket();

$steam_id = get_int64_param('steam_id');

database_connect();

# Grab the users internal ID.
$user_id = register_cheat_user($steam_id);

# Grab all uncompleted cases assigned to the user.
$rows = database_select(
	"cheat_judgements",
	array(
		"investigator_user_id" => $user_id,
		"is_complete" => False,
		"is_expired" => False
	)
);

# Return the cases assigned to the user.
$cases = array();

if (count($rows) > 0)
{	
	foreach ($rows as $row)
	{		
		$report_rows = database_select(
			"cheat_reports",
			array(
				"id" => $row['report_id']
			)
		);	
		
		if (count($report_rows) > 0)
		{
			$report = $report_rows[0];
			
			$cases[] = array(
				"judgement_id" => $row['id'],
				"demo_file_path" => $demo_upload_folder_public . $report['demo_guid'],
				"reported_steamid" => $report['reported_steamid'],
				"reported_username" => $report['reported_username'],
				"category" => $report['category'],
				"timeframe" => $report['timeframe']
			);
		}
	}
}

database_disconnect();

# Done!
finish_request(array(
	"cases" => $cases
));

?>