@echo off

@SET PATH=".\Tools;%PATH%"
SET PROGNAME=%1

%PROGNAME% > tmpfile
SET /p VALUE= < tmpfile
DEL tmpfile
Tools\sed.exe -b -i_old.exe "s,\x00|TOTO-TATA-TITI-TUTU-TETE-TYTY|\x00,%VALUE%,g" %PROGNAME%
