<?php

require_once(dirname(__FILE__) . "/helpers.php");
require_once(dirname(__FILE__) . "/settings.php");
require_once(dirname(__FILE__) . "/authentication.php");
require_once(dirname(__FILE__) . "/database.php");
require_once(dirname(__FILE__) . "/parameters.php");
require_once(dirname(__FILE__) . "/request.php");
require_once(dirname(__FILE__) . "/steam.php");

if ($webapi_disabled)
{
	fail_request("WebAPI is currently undergoing maintenance.");	
}

?>