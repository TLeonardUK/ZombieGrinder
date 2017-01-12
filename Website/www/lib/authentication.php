<?php

function require_steam_ticket()
{
	$user_steamid = get_int64_param('steam_id');
	$auth_ticket  = get_string_param('auth_ticket');
	if (!validate_steam_ticket($user_steamid, $auth_ticket))
	{
		fail_request("Could not authenticate steam ticket.");
	}
}

function require_local_connection()
{
	global $ignore_local_connection_auth;
	
	if ($ignore_local_connection_auth)
	{
		return;		
	}
	
	if (isset($_SERVER['REMOTE_ADDR']))
	{
		fail_request("Reserved for internal use.");			
	}
}

function validate_steam_ticket($user_steamid, $auth_ticket)
{
	global $steam_webapi_publisher_key;
	global $steam_webapi_appid;
	
	$request = steam_webapi_request('ISteamUserAuth', 'AuthenticateUserTicket', 'V0001', 'GET', array(
		"key" => $steam_webapi_publisher_key,
		"appid" => $steam_webapi_appid,
		"ticket" => $auth_ticket,
	));
	
	if ($request === false)
	{	
		return false;
	}

	if ($request['response']['params']['result'] == 'OK' &&
		$request['response']['params']['steamid'] == $user_steamid)
	{
		return true;		
	}
	
	return false;
}

?>