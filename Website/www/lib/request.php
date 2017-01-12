<?php
 
function finish_request($json = array())
{
	header('Content-Type: application/json');
	die(json_encode(array( 
		"success" => True, 
		"result" => $json 
	)));
}

function fail_request($error = "Unspecified error.")
{
	header('Content-Type: application/json');
	die(json_encode(array( 
		"success" => False, 
		"result" => array( 
			"message" => $error 
		) 
	)));
}
 
?>