@echo off
cd ..\..\Binary\Win32

if not exist ..\..\Output\Builder\%1\win32\Builder.exe (
	echo Waiting for builder to be generated at: ..\..\Output\Builder\%1\win32\Builder.exe
)
:waitForBuilder
if not exist ..\..\Output\Builder\%1\win32\Builder.exe (
	goto waitForBuilder
)

..\..\Output\Builder\%1\win32\Builder.exe Build Base