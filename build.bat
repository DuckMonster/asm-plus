@echo off
echo -- Building compiler --
msbuild ap-compile.vcxproj -verbosity:minimal -nologo -property:Configuration=%1 -property:Platform=x64

