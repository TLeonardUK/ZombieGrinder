<?php

require_once(dirname(__FILE__) . "/../../../lib/webapi.php");

require_steam_ticket();

$steam_id = get_int64_param('steam_id');
$judgement_id = get_int_param('judgement_id');
$category = get_int_param('category');
$result = get_int_param('result');

database_connect();

# Grab the users internal ID.
$user_id = register_cheat_user($steam_id);

# Grab all uncompleted cases assigned to the user.
$rows = database_select(
	"cheat_judgements",
	array(
		"id" => $judgement_id
	)
);

# Valid judgement?
if (count($rows) == 0)
{	
	fail_request("Invalid judgement ID.");
}

# Ensure its for us to judge.
$judgement = $rows[0];
if ($judgement['investigator_user_id'] != $user_id)
{
	fail_request("Judgement not assigned to you.");
}

# Ensure we haven't judged it already.
if ($judgement['is_complete'])
{
	fail_request("Judgement already complete.");
}

# Ensure its not expired.
if ($judgement['is_expired'])
{
	fail_request("Judgement has expired.");
}

# Judge!
database_update_row(
	"cheat_judgements",
	$judgement_id,
	array(
		"is_complete" => True,
		"judgement_time" => time(),
		"category" => $category,
		"result" => $result
	)	
);

database_disconnect();

# Done!
finish_request(array());

?>