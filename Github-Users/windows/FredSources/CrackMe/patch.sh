#!/bin/sh

PROGNAME=$1
sleep 2
VALUE=`$PROGNAME | head -n 1 ` 
sed -b -i_old.x "s,\x00|TOTO-TATA-TITI-TUTU-TETE-TYTY|\x00,$VALUE,g" $PROGNAME
