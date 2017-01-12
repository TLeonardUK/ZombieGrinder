function Redirect(target)
{
	window.location = target;
}
function RedirectConfirm(target, message)
{
	if (confirm(message))
	{
		window.location = target;
	}
}