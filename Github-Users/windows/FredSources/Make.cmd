@set MODE=release
@set ARCH=x64
@set ACTION=build

@if "%1" == "clean"  set ACTION=clean
@if "%1" == "all"    set ACTION=rebuild
@if "%3" == "clean"  set ACTION=clean
@if "%3" == "all"    set ACTION=rebuild
@if "%5" == "clean"  set ACTION=clean
@if "%5" == "all"    set ACTION=rebuild

@if "%1" == "MODE" (
	if "%2" == "Release" set MODE=release
	if "%2" == "Debug"   set MODE=debug
)

@if "%3" == "MODE"  (
	if "%4" == "Release" set MODE=release
	if "%4" =="Debug"   set MODE=debug
)

@if "%1" == "ARCH"  (
	if "%2" == "x86_64" set ARCH=x64
	if "%2" == "x86"    set ARCH=Win32
	if "%2" == "x86_64" set ENVVC=amd64
	if "%2" == "x86"    set ENVVC=x86
)

@if "%3" == "ARCH"  (
	if "%4" == "x86_64" set ARCH=x64
	if "%4" == "x86"    set ARCH=Win32
	if "%4" == "x86_64" set ENVVC=amd64
	if "%4" == "x86"    set ENVVC=x86	
)

@echo ARCH=%ARCH%
@echo MODE=%MODE%
@echo ACTION=%ACTION%

@echo *************************************************
@echo * Set build env                                 *
@echo *************************************************
@if NOT "%MAKE_ENV_LOADED%" == "1" (
call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" %ENVVC%
SET MAKE_ENV_LOADED=1
)

@echo *************************************************
@echo * Build solution                                *
@echo *************************************************
msbuild CrackMe.sln /p:configuration=%MODE% /p:platform=%ARCH% /t:%ACTION%
pause
