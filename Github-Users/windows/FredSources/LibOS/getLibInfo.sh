#!/bin/sh

COLOR_GRIS="\033[01;30m"
COLOR_ROUGE="\033[01;31m"
COLOR_VERT="\033[01;32m"
COLOR_JAUNE="\033[01;33m"
COLOR_BLEU="\033[01;34m"
COLOR_ROSE="\033[01;35m"
COLOR_BLANC="\033[01;37m"
COLOR_END="\033[01;00m"

if  test $# -eq 1
then
	if test -r $1 
	then
		if test -x $1 
		then
			echo ""
			printf  "$COLOR_ROUGE -- Format de $1 --  \n$COLOR_END"
			printf  "$COLOR_ROSE"
			file -L $1 | sed 's/^[ \t]*\(.*\)$/\to \1/g'
			printf  "\n$COLOR_END"		

			printf  "$COLOR_ROUGE -- Stat de $1 --  \n$COLOR_END"
			printf  "$COLOR_BLEU"
			stat $1 | sed 's/^[ \t]*\(.*\)$/\to \1/g'
			printf  "\n$COLOR_END"

			printf  "$COLOR_ROUGE -- Dependance de $1 -- \n$COLOR_END"
			printf  "$COLOR_BLEU"
			ldd $1 | sed 's/^[ \t]*\(.*\)$/\to \1/g'
			printf  "\n$COLOR_END"

			printf  "$COLOR_ROUGE -- Info de $1 --  \n$COLOR_END"
			printf  "$COLOR_BLEU"
			strings $1 | grep INFO_LIBRARY | sed 's/^[ \t]*\(.*\)$/\to \1/g'
			printf  "\n$COLOR_END"

		else
			echo "Le fichier $1 n'est pas executable"
		fi
	else
		echo "Le fichier $1 n'est pas accessible"
	fi
else
	echo "SYNTAXE ERROR : $0 <libXxxxx.so>"
fi

