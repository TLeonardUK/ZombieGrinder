<?php

// Steam settings.
$steam_webapi_publisher_key 				= "";
$steam_webapi_appid 					= 263920;

// Database settings.
$database_host 						= "";
$database_name 						= "";
$database_user 						= "";
$database_pass 						= "";

// Upload directories.
$demo_upload_folder_local 				= "/home/www/api.twindrills.com/public_html/demos/";
$demo_upload_folder_public 				= "http://api.zombiegrinder.com/demos/";

// Global disable method for webapi for maintenance etc.
$webapi_disabled 					= False;

// WARNING: Very dangerous, allows webapi calls that should only be called from localhost, to be called from anywhere.
$ignore_local_connection_auth				= False;

// Case resolution settings.
$judgements_per_report 					= 14;   				// How many judgements we request per report.
$judgements_required_for_resolve 			= 9;    				// How many judgements we need to make a resolution.
$judgement_expire_age  					= 24 * 60 * 60;			// How long (in seconds) without resolution before a judgement expires.
$judgement_report_ignore_period				= 12 * 60 * 60;			// If we have multiple reports within this period then only the first report resolved will
																	// count against the user as the reports are probably from the same instance of bad behaviour.
$judgement_majority_threshold				= 0.65;					// At least 65% of our "jury" need to agree to consider it a safe judgement.
$infraction_severity_previous_weighting 		= 0.35;					// Weighting given to previous infractions when calculating current infractions severity. Read calculate_case_severity for details.			
$infraction_expire_time					= 60 * 24 * 60 * 60;	// How old infractions have to be before they are no longer weighted into new infraction severities.
				
$investigator_active_time				= 2 * 24 * 60 * 60;		// How recent a user had to be online to be considered an investigator.
$investigator_max_last_severity				= 15.0;					// User has a last report severity above this then they are blacklisted from being an investigator.
$investigator_upheld_report_threshold			= 3;					// Users with more upheld reports than this are blacklisted from being investigators.
$investigator_minority_ratio_threshold			= 2.0;					// Minority ratio is calculated as as (judgements_in_minority/judgements_in_majority). If value is higher than this, the user is blacklisted.
$investigator_minority_ratio_min_judgements		= 8;					// Minimum number of judgements before the minority ratio blacklist kicks in.
$investigator_max_caseload				= 2;					// Maximum caseload a user can have at one time.
$investigator_min_age					= 7 * 24 * 60 * 60;		// How old the account must be before we can consider them an investigator.

$apply_bans_manually					= True;					// Bans will be stored in the cheat_requested_bans table, but will not be automatically submitted to steam.
$manually_approved_investigators			= True;					// If true only manually approved investigators will recieve cases.			
			
// Some enums.
define("CHEAT_JUDGEMENT_RESULT_GUILTY", 		0);
define("CHEAT_JUDGEMENT_RESULT_INCONCLUSIVE", 		1);
define("CHEAT_JUDGEMENT_RESULT_INNOCENT", 		2);

define("CHEAT_JUDGEMENT_CATEGORY_EXPLOIT", 		1);
define("CHEAT_JUDGEMENT_CATEGORY_CHEAT", 		2);
define("CHEAT_JUDGEMENT_CATEGORY_ABUSE", 		4);
define("CHEAT_JUDGEMENT_CATEGORY_GRIEF", 		8);
	
?>