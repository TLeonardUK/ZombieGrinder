<?php

require_once(dirname(__FILE__) . "/../../../lib/webapi.php");

require_steam_ticket();

$steam_id = get_int64_param('steam_id');

database_connect();

# Grab all judgements relating to this user.
$rows = database_select(
	"cheat_outcomes",
	array(
		"reported_steamid" => $steam_id,
		"conclusion" => CHEAT_JUDGEMENT_RESULT_GUILTY,
		"ban_submitted" => True,
	)
);

$bans = array();

if (count($rows) > 0)
{	
	foreach ($rows as $row)
	{		
		$end_time = $row['ban_start'] + $row['ban_duration'];
		if ($end_time < time())
		{
			$ban = array(
				"end_time"		=> ($row['ban_duration'] <= 0 ? "" : date("jS F Y, H:i e", $end_time)),
				"report_time"	=> date("jS F Y, H:i e", $row['report_time']),
				"category"		=> $row['category'],
				"report_id"		=> $row['report_id']
			);
			$bans[] = $ban;
		}
	}
}

database_disconnect();

# Done!
finish_request(array(
	"bans" => $bans
));

?>