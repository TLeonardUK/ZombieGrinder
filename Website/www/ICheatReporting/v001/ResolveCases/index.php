<?php

require_once(dirname(__FILE__) . "/../../../lib/webapi.php");

require_local_connection();

database_connect();

# Step one - Get all returned judgements, if we have enough to 
# resolve a report, then do so.
resolve_cases();

# Step two - Give any outstanding judgements to people to investigate.
distribute_cases();

# Step three - Apply any bans requested (approved only in manual mode).
apply_bans();

database_disconnect();

finish_request(array());

function apply_bans()
{	
	global $apply_bans_manually;

	$bans = database_select(
		"cheat_requested_bans",
		array(
			"submitted" => False
		)
	);
	
	foreach ($bans as $ban)
	{
		if ($apply_bans_manually == True && $ban['approved'] == False)
		{
			continue;
		}			
		
		create_steam_game_ban($ban['steam_id'], $ban['duration'], $ban['description'], $ban['steam_reportid']);	
		
		database_update_row(
			"cheat_requested_bans",
			$ban['id'],
			array(
				"submitted" => False
			)					
		);
		
		database_update_row(
			"cheat_outcomes",
			$ban['outcome_id'],
			array(
				"ban_submitted" => True,
				"ban_id" => $ban['id'],
				"ban_start" => time()
			)			
		);
	}	
}

function resolve_cases()
{
	global $judgements_required_for_resolve;
	
	$unresolved_reports = database_select(
		"cheat_reports",
		array(
			"outcome_id" >= -1
		)
	);
	foreach ($unresolved_reports as $report)
	{
		$judgements = database_select(
			"cheat_judgements",
			array(
				"report_id"  => $report['id'],
				"is_expired" => False
			)
		);
		
		$resolved = 0;
		
		foreach ($judgements as $judgement)
		{
			if ($judgement['is_complete'])
			{
				$resolved += 1;
			}		
		}
		
		// Enough resolved judgements to make a resolution?
		if ($resolved >= $judgements_required_for_resolve)
		{
			resolve_case($judgements, $report);		
		}	
		
		// Otherwise we expire any pending judgements that are old enough
		// so they can be redistributed to other users.
		else
		{
			database_update_expr(
				"cheat_judgements",
				"assignment_time < ?",
				array(				
					time() - $judgement_expire_age
				),
				array(
					"is_expired" => True,
				)
			);
		}
	}
}

function resolve_case($judgements, $report)
{
	global $judgement_majority_threshold;
	global $judgement_report_ignore_period;
	
	// Just mark report as closed if report was created within the same time period of a pre-existing
	// outcome. As the reports probably came form the same instance of poor behaviour.
	$overlapping_outcomes = databse_select_expr(
		"cheat_outcomes",
		"report_id == ? AND report_time > ? AND report_time < ?",
		array(
			$report['id'],
			$report['report_time'] - $judgement_report_ignore_period,
			$report['report_time'] + $judgement_report_ignore_period
		)
	);
	
	$result = array(
		"report_id" => $report['id'],
		"report_time" => $report['report_time'],
		"conclusion" => 0,
		"category" => 0,
		"ban_duration" => 0,
		"ban_start" => time(),
		"severity" => 0.0,
		"reported_steamid" => $report['reported_steamid'],
		"reported_userid" => -1,
		"ignored_due_to_report_time" => False
	);
	
	if (count($overlapping_outcomes) > 0)
	{
		$result['ignored_due_to_report_time'] = True;
	}
	
	// Work out the conclusion.
	$investigator_stats = array();
	calculate_case_conclusion($judgements, &$result['conclusion'], &$result['category'], $investigator_stats);
	
	// Work out user info.
	$reported_user = get_cheat_user($report['reported_steamid']);
	$result['reported_userid'] = $reported_user['id'];
		
	$ban_requested = False;
	$ban_fields = array();
		
	// Ban the user and increment his ban stats if required.
	if ($result['ignored_due_to_report_time'] == False && $result['conclusion'] == $CheatJudgementResult_Guilty)
	{
		$previous_report_avg_sum   = 0.0;
		$previous_report_avg_count = 0.0;
		calculate_user_report_avg_severity($reported_user['id'], &$previous_report_avg_sum, &$previous_report_avg_count);
		
		$result['severity'] 	= calculate_case_severity($result['category'], $previous_report_avg_sum / $previous_report_avg_count, $previous_report_avg_count);				
		$result['ban_duration'] = calculate_ban_duration($result['severity']);				
		
		$guilt_string = "";
		if (($guilt_string & CHEAT_JUDGEMENT_CATEGORY_EXPLOIT) != 0)
		{
			if ($guilt_string != "")
				$guilt_string .= ", ";

			$guilt_string .= "Exploting";
		}
		if (($guilt_string & CHEAT_JUDGEMENT_CATEGORY_CHEAT) != 0)
		{
			if ($guilt_string != "")
				$guilt_string .= ", ";

			$guilt_string .= "Cheating";
		}
		if (($guilt_string & CHEAT_JUDGEMENT_CATEGORY_ABUSE) != 0)
		{
			if ($guilt_string != "")
				$guilt_string .= ", ";

			$guilt_string .= "Abusive Behaviour";
		}
		if (($guilt_string & CHEAT_JUDGEMENT_CATEGORY_GRIEF) != 0)
		{
			if ($guilt_string != "")
				$guilt_string .= ", ";

			$guilt_string .= "Griefing";
		}
		
		$ban_requested = True;
		$ban_fields = array(
			"steam_id" 			=> $report['reported_steamid'],  
			"steam_reportid"	=> $report['steam_reportid'],
			"duration" 			=> $result['ban_duration'],
			"description" 		=> $report['steam_reportid'],
			"submitted" 		=> "Judged by " . count($judgements) . " investigators to be guilty of '" . $guilt_string . "', based on demo replay. Previous Infractions = " . $previous_report_avg_count . ", Average Infraction Severity = " . $average_severity . ", Internal Report ID = " . $report['id'] . "."
		);
		
		database_update_row(
			"cheat_users",
			$reported_user['id'],
			array(
				"reports_upheld" => ($reported_user['reports_upheld'] + 1),
				"last_upheld_report_severity" => $result['severity']
			)	
		);
	}
	
	// Increment investigator stats.
	foreach ($stats as $investigator_stats)
	{			
		if ($stats['in_majority'])
		{
			database_update_row(
				"cheat_users",
				$stats['user']['id'],
				array(
					"judgements_made" => ($stats['user']['judgements_made'] + 1),
					"judgements_in_majority" => ($stats['user']['judgements_in_majority'] + 1),
				)	
			);
		}		
		else if ($stats['in_minority'])
		{
			database_update_row(
				"cheat_users",
				$stats['user']['id'],
				array(
					"judgements_made" => ($stats['user']['judgements_made'] + 1),
					"judgements_in_minority" => ($stats['user']['judgements_in_minority'] + 1),
				)	
			);
		}
		
		// TODO: Give rewards?
	}
	
	// Create outcome row.
	$outcome_id = database_insert_row(
		"cheat_outcomes",
		$result
	);
	
	if ($ban_requested)
	{
		$ban_fields['outcome_id'] = $outcome_id;
		
		database_insert(
			"cheat_requested_bans",
			$ban_fields
		);
	}
	
	// Mark the report as closed.
	database_update_row(
		"cheat_judgements",
		$report['id'],
		array(
			"is_resolved" => True,
			"outcome_id" => $outcome_id
		)
	);
}

function calculate_case_conclusion($judgements, &$out_conclusion, &$out_category, &$out_investigator_stats)
{
	global $judgement_majority_threshold;
	
	$guilty_verdicts 		= 0;
	$inconclusive_verdicts 	= 0;
	$innocent_verdicts 		= 0;
	$total_verdicts			= count($judgement);
	
	foreach ($judgements as $judgement)
	{
		if ($judgements['result'] == CHEAT_JUDGEMENT_RESULT_GUILTY)
		{
			$guilty_verdicts += 1;
		}
		if ($judgements['result'] == CHEAT_JUDGEMENT_RESULT_INCONCLUSIVE)
		{
			$inconclusive_verdicts += 1;
		}
		if ($judgements['result'] == CHEAT_JUDGEMENT_RESULT_INNOCENT)
		{
			$innocent_verdicts += 1;
		}
	}
	
	$out_conclusion = CHEAT_JUDGEMENT_RESULT_INCONCLUSIVE;
	$inconclusive = False;
	
	// Any overwhelming majorities?
	if ($guilty_verdicts >= ($total_verdicts * $judgement_majority_threshold))
	{
		$out_conclusion = CHEAT_JUDGEMENT_RESULT_GUILTY;
	}
	if ($innocent_verdicts >= ($total_verdicts * $judgement_majority_threshold))
	{
		$out_conclusion = CHEAT_JUDGEMENT_RESULT_INNOCENT;
	}
	if ($inconclusive_verdicts >= ($total_verdicts * $judgement_majority_threshold))
	{
		$out_conclusion = CHEAT_JUDGEMENT_RESULT_INCONCLUSIVE;
	}
	else
	{
		// If no majorities we have to return an inconclusive result.
		$out_conclusion = CHEAT_JUDGEMENT_RESULT_INCONCLUSIVE;
		$inconclusive = True;
	}
	
	// If we have a guilty conclusion, then we need to figure out what the majority believe the user is guilty of.
	if ($out_conclusion == CHEAT_JUDGEMENT_RESULT_GUILTY)
	{
		$exploit_verdicts 	= 0;
		$cheat_verdicts 	= 0;
		$abuse_verdicts 	= 0;
		$grief_verdicts 	= 0;
			
		foreach ($judgements as $judgement)
		{
			if (($judgements['category'] & CHEAT_JUDGEMENT_CATEGORY_EXPLOIT) != 0)
			{
				$exploit_verdicts += 1;
			}
			if (($judgements['category'] & CHEAT_JUDGEMENT_CATEGORY_CHEAT) != 0)
			{
				$cheat_verdicts += 1;
			}
			if (($judgements['category'] & CHEAT_JUDGEMENT_CATEGORY_ABUSE) != 0)
			{
				$abuse_verdicts += 1;
			}
			if (($judgements['category'] & CHEAT_JUDGEMENT_CATEGORY_GRIEF) != 0)
			{
				$grief_verdicts += 1;
			}
		}
		
		// Majority agreements.
		$out_category = 0;
		
		if ($exploit_verdicts >= ($total_verdicts * $judgement_majority_threshold))
		{
			$out_category |= CHEAT_JUDGEMENT_CATEGORY_EXPLOIT;
		}
		if ($cheat_verdicts >= ($total_verdicts * $judgement_majority_threshold))
		{
			$out_category |= CHEAT_JUDGEMENT_CATEGORY_CHEAT;
		}
		if ($abuse_verdicts >= ($total_verdicts * $judgement_majority_threshold))
		{
			$out_category |= CHEAT_JUDGEMENT_CATEGORY_ABUSE;
		}
		if ($grief_verdicts >= ($total_verdicts * $judgement_majority_threshold))
		{
			$out_category |= CHEAT_JUDGEMENT_CATEGORY_GRIEF;
		}
		
		// If nobody can agree on an infraction the user is performing. Then we have to return an inconclusive result.	
		if ($out_category == 0)
		{
			$out_conclusion = CHEAT_JUDGEMENT_RESULT_INCONCLUSIVE;
			$inconclusive = True;
		}
	}

	// Work out the investigator stats as well.	
	$out_investigator_stats = array();
	foreach ($judgements as $judgement)
	{
		$out_investigator_stats[] = array(
			"user" => get_cheat_user_by_id($judgement['investigator_user_id']),
			"in_majority" => ($inconclusive == False && $judgement['result'] == $out_conclusion),
			"in_minority" => ($inconclusive == False && $judgement['result'] != $out_conclusion)
		);
	}
}

function distribute_cases()
{	
	$caseload = generate_caseload();
	$investigators = get_available_investigators();
	
	$investigator_cycle_id = 0;
	
	if (count($investigators) > 0)
	{
		foreach ($reports as $report)
		{			
			$reported_user = get_cheat_user($report['reported_steamid']);
			$reported_user_id = $reported_user['id'];
			
			$bSuitable = false;
			$investigator_user_id = 0;
			
			for ($i = 0; $i < count($investigators); $i++)
			{
				$bSuitable = true;
		
				$investigator_user_id = $investigators[$investigator_cycle_id % count($investigators)];
				$investigator_cycle_id++;
		
				// If investigator is the same as the reported user, go to the next investigator.
				if ($investigator_user_id == $reported_user_id)
				{
					$bSuitable = false;
				}
				
				// Check if investigator has already voted on this judgement.
				$existing_judgements = database_select(
					"cheat_judgements",
					array(
						"investigator_user_id" => $investigator_user_id,
						"report_id" => $report['id'],
					)
				); 
				if (count($existing_judgements) > 0)
				{
					$bSuitable = false;
				}
				
				if ($bSuitable)
				{
					break;
				}
			}
			
			if (!$bSuitable)
			{
				// No suitable investigators.
				continue;
			}
			
			$judgement_id = database_insert(
				"cheat_judgements",
				array(
					"investigator_user_id" => $investigator_user_id,
					"report_id" => $report['id'],
					"category" => $report['category'],
					"assignment_time" => time(),
					"judgement_time" => 0,
					"is_complete" => False,
					"is_expired" => False				
				)			
			);
		}
	}	
}

function generate_caseload()
{	
	$unresolved_reports = database_select(
		"cheat_reports",
		array(
			"outcome_id" >= -1
		)
	);

	$result = array();
	
	foreach ($unresolved_reports as $report)
	{
		$judgements = database_select(
			"cheat_judgements",
			array(
				"report_id"  => $report['id'],
				"is_expired" => False
			)
		);
		if (count($judgements) < $judgements_per_report)
		{
			for ($i = 0; $i < $judgements_per_report - count($judgements); $i++)
			{
				$result[] = $report;
			}
		}
	}
	
	return $result;
}

function calculate_user_report_avg_severity($user_id, &$avg_sum, &$avg_count)
{
	global $infraction_expire_time;
	
	$rows = database_select_expr(
		"cheat_outcomes",
		"id == ? AND report_time > ? AND conclusion == ?",
		array(
			$user_id,
			time() - $infraction_expire_time,
			CHEAT_JUDGEMENT_RESULT_GUILTY
		)
	);

	$avg_count = 0;	
	$avg_sum = 0.0;
	
	foreach ($rows as $row)
	{
		$avg_sum += $row['severity'];		
	}	
}

function compare_investigators($a, $b) 
{
	# Investigators are sorted with 2 orders:
	#   1. number of judgements, the higher the further up the list.
	#	1. (judgements_in_minority / judgements_in_majority), lower the value, higher in the list.
	
    if ($a['judgements_made'] == $b['judgements_made']) 
	{		
		$minority_threshold_a = $a['judgements_in_minority'] / $a['judgements_in_majority'];
		$minority_threshold_b = $b['judgements_in_minority'] / $b['judgements_in_majority'];
		
		if ($minority_threshold_a == 0 && $minority_threshold_b == 0)
		{
			return 0;			
		}
		
		return ($minority_threshold_b < $minority_threshold_b ? -1 : 1);				
    }
	else
	{
		return ($a['judgements_made'] < $b['judgements_made']) ? 1 : -1;		
	}
}

function get_available_investigators()
{
	global $investigator_active_time;
	global $investigator_max_last_severity;
	global $investigator_upheld_report_threshold;
	global $investigator_minority_ratio_threshold;
	global $investigator_minority_ratio_min_judgements;
	global $investigator_max_caseload;
	global $investigator_min_age;
	
	global $manually_approved_investigators;
	
	$investigators = array();

	# Get all potential investigators.
	$rows = database_select_expr(
		"cheat_users",
		"last_active_time > ? AND last_upheld_report_severity < ? AND reports_upheld < ? AND (judgements_made < ? || (judgements_in_minority/judgements_in_majority) < ?) AND first_seen_time < ?",
		array(
			time() - $investigator_active_time,
			$investigator_max_last_severity,
			$investigator_upheld_report_threshold,
			$investigator_minority_ratio_min_judgements,
			$investigator_minority_ratio_threshold,	
			time() - $investigator_min_age
		)
	);
	
	# Prune all investigators who have caseloads above the max.
	foreach ($rows as $row)
	{
		$judgement_rows = database_select(
			"cheat_judgements",
			array(
				"investigator_user_id" => $row['id'],
				"is_expired" => False,
				"is_complete" => False,
			)
		);
		
		if ($manually_approved_investigators == False || $row['approved_investigator'] == True)
		{
			if (count($judgement_rows) < $investigator_max_caseload)
			{
				$investigators[] = $row;
			}			
		}
	}
	
	# Sort the investigators by prefence, see callback for details.
	uasort($investigators, 'compare_investigators');
	
	# We basically start a raffle here, each entry is a ticket, the higher an investigator is in the previous array
	# the more tickets they get.
	$result = array();	

	for ($i = 0; $i < count($investigators); $i++)
	{
		for ($j = 0; $j < (count($investigators) - $i); $i++)
		{
			$result[] = $investigators[$i]['id'];
		}
	}
	
	# We randomise the pool.
	shuffle($result);
	
	return $result;	
}

function calculate_ban_duration($severity)
{		
	# Based on the resulting severity, the following punishments are applied:
	#	<= 15% - 6 hour ban
	#	<= 30% - 1 day ban
	#	<= 45% - 3 day ban
	#	<= 60% - 7 day ban
	#	<= 80% - 14 day ban
	#	>  80% - permanent ban
	
	if ($severity <= 15.0)
	{
		return 6 * 60 * 60;
	}		
	else if ($severity <= 30.0)
	{
		return 1 * 24 * 60 * 60;		
	}		
	else if ($severity <= 45.0)
	{
		return 3 * 24 * 60 * 60;				
	}		
	else if ($severity <= 60.0)
	{
		return 7 * 24 * 60 * 60;				
	}		
	else if ($severity <= 80.0)
	{
		return 14 * 24 * 60 * 60;				
	}		
	else
	{
		return 10 * 365 * 24 * 60 * 60;
	}
}

function calculate_case_severity($category, $previous_report_average, $previous_report_average_count)
{
	global $infraction_severity_previous_weighting;
	
	# Case severity is a scale between 0 and 100.
	# Lowest end recieves a few hours ban, up to a permanent ban.
	
	# Severity is calculated indevidually for each judgement then averaged. 
	
	# Severity is calculated based on the categories they were judged to be infringing:
	# 	Exploiting: +20%
	# 	Cheating:   +35%
	#	Abusive:	+10%
	#	Griefing:	+10%
	
	# Scale adds up to below 100 to ensure no perma bans on first infraction.
	# For each previous infraction the user has had (within last 3 months) the 
	# severity is multiplied by a multiplier generated as following:
	#	((average_prev_infractions / 100) * 0.5) * prev_infraction_count
	
	# todo these examples dont account for the multiplier from the previous case.
	
	# So if a user is reported as cheating this will happen:
	# First case
	#   35 = 7 day ban
	# Second case
	#   Prev average: 35
	#   35 + ((35*0.5)=17) = 52 = 7 day ban
	# Third case
	#   Prev Average = (35+52)*0.5 = 43.5
	#   35 + ((43.5*0.5)*2=42) = 77 = 14 day ban
	# Forth case
	#   Prev Average = (35+52+77)/3 = 54
	#   35 + ((54*0.5)*3=81) = 116 = Permanent
	
	# Abusive: 
	# First Case
	#	10 - 12 hour ban
	# Second case
	#	10 + 5 = 12 hour ban
	# Third case
	#   10 + 10 = 3 day bans
	# Forth Case
	#	10 + 15 = 3 day ban
	# Fifth Case
	#   10 + 20 = 3 day ban
	
	# Based on the resulting severity, the following punishments are applied:
	#	<= 15% - 12 hour ban
	#	<= 30% - 3 day ban
	#	<= 60% - 7 day ban
	#	<= 80% - 14 day ban
	#	>  80% - permanent ban

	$severity = 0.0;
	
	if (($category & CHEAT_JUDGEMENT_CATEGORY_EXPLOIT) != 0)
	{
		$severity += 20.0;
	}
	if (($category & CHEAT_JUDGEMENT_CATEGORY_CHEAT) != 0)
	{
		$severity += 35.0;
	}
	if (($category & CHEAT_JUDGEMENT_CATEGORY_ABUSE) != 0)
	{
		$severity += 10.0;
	}
	if (($category & CHEAT_JUDGEMENT_CATEGORY_GRIEF) != 0)
	{
		$severity += 10.0;
	}
	
	$severity += (($previous_report_average / 100.0) * $infraction_severity_previous_weighting) * $previous_report_average_count;	
	
	return $severity;
}
 
?>