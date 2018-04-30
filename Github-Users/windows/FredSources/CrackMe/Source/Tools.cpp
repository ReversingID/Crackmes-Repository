
#include <string.h>
#include <stdlib.h>
#include "Tools.h"
#include "CTrace.h"

size_t my_fgets(FILE * fichier,char ** buffer)
{
	char c;
	size_t taille = 1;
	size_t position = 0;

	TRACE_DEFAULT(_STR2WSTR(""));
	if (*buffer == NULL)
	{
		*buffer = (char *)malloc(REALLOC_STEP);
	}
	else
	{
		*buffer = (char *)realloc(*buffer, REALLOC_STEP);
	}
	if (*buffer != NULL)
	{
		do
		{
			if (position >= taille)
			{
				taille = taille + REALLOC_STEP;
				*buffer = (char *)realloc(*buffer, taille + REALLOC_STEP);
			}
			if (*buffer != NULL)
			{
				c = fgetc(fichier);
				if (c != EOF)
				{
					if (c != '\n')
					{
						(*buffer)[position] = c;
						position++;
					}
					else
					{
						break;
					}
				}
			}
		} while ((c != EOF) && (*buffer != NULL));
		if (*buffer != NULL)
		{
			(*buffer)[position] = '\0';
			if ((c == EOF) && (position == 0))
			{
				position = -1;
			}
		}
	}

	if (*buffer == NULL)
	{
		fprintf(stderr, "Allocation Failed !!! ");
		position = 0;
	}

	fflush(stdin);
	TRACE_DEFAULT(_STR2WSTR("%s"), *buffer);
	return position;
}
