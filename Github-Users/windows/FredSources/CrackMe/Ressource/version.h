#ifndef __VERSION_H__
#define __VERSION_H__

	/*****************************************************/
	/* Definition de la version de la librairie          */
	/*****************************************************/

	#define VERSION      1,0,0,0
	#define FILENAME     "CrackMe"
	#define DESCRIPTION  "CrackMe"

	/*****************************************************/
	/* FIN de definition de la version de la librairie   */
	/*****************************************************/
	#if (defined(WIN32) || defined(WIN64))
		#define EXTENTION ".exe"
		#ifdef __GNUC__
			#define __CONVERT_VERSION(MAJOR,MINOR,REV,COMPIL) #MAJOR "."  #MINOR "."  #REV "."  #COMPIL
		#else
			#define CONVERT_VERSION(a) __TO_STRING(a)
		#endif
	#else
		#define EXTENTION ".x"
		#define CONVERT_VERSION(a) __CONVERT_VERSION(a)
		#define __CONVERT_VERSION(MAJOR,MINOR,REV,COMPIL) #MAJOR "."  #MINOR "."  #REV "."  #COMPIL
	#endif

	#define TO_STRING(a) __TO_STRING(a)
	#define __TO_STRING(a) #a

	#define ADD_TO_STRING(a,b) __ADD_TO_STRING(a,b)
	#define __ADD_TO_STRING(a,b) a b

	#define FILEVER          VERSION
	#define PRODUCTVER       FILEVER
	#define STRFILEVER       TO_STRING(FILEVER)
	#define STRPRODUCTVER    TO_STRING(PRODUCTVER)

	#ifdef _DEBUG
		#if (defined(WIN32) || defined(WIN64))
			#define TRUE_FILENAME     ADD_TO_STRING(FILENAME,"d")
		#else
			#define TRUE_FILENAME     FILENAME
		#endif
		#define TRUE_DESCRIPTION  ADD_TO_STRING(DESCRIPTION," (Mode Debug)")
	#else
		#define TRUE_FILENAME     FILENAME
		#define TRUE_DESCRIPTION  DESCRIPTION
	#endif

#endif



