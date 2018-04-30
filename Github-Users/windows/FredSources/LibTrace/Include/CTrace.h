#ifndef _CPP_TRACE_H__
#define _CPP_TRACE_H__

#define ALIGNEMENT 60

#define TO_STRING(a) __TO_STRING(a)
#define __TO_STRING(a) #a

#define ADD_TO_STRING(a,b) __ADD_TO_STRING(a,b)
#define __ADD_TO_STRING(a,b) a b

#ifdef __cplusplus
	extern "C"
	{
#endif
		#ifdef _DEBUG
			void TRACE_DEACTIVATE();
			void TRACE_ACTIVATE();
			void TRACE_START();
			void TRACE_STOP();
			#ifndef UNICODE
				int trace_default(const char * file,int line,const char * fonction,const char * format,...);
			#else
				int trace_default(const wchar_t * file,int line,const wchar_t * fonction,const wchar_t * aFormat,...);
			#endif
		#else
			#define  TRACE_DEACTIVATE() //
			#define  TRACE_ACTIVATE() //
			#define  TRACE_START() //
			#define  TRACE_STOP() //
		#endif
#ifdef __cplusplus
	}
#endif
	#ifdef _DEBUG
		
		#ifndef UNICODE
			#ifdef _STR2WSTR
				#undef _STR2WSTR
			#endif
			#define _STR2WSTR(str) str
			#if (defined(WIN32) || defined (WIN64))
				#define TRACE_DEFAULT(...)            trace_default(__FILE__,__LINE__,__FUNCTION__,__VA_ARGS__)
			#else
				#define TRACE_DEFAULT(...)            trace_default(__FILE__,__LINE__,__PRETTY_FUNCTION__,__VA_ARGS__)
			#endif
		#else
			#ifndef __STR2WSTR
				#define __STR2WSTR(str)    L##str
			#endif
			#ifndef _STR2WSTR
				#define _STR2WSTR(str)     __STR2WSTR(str)
			#endif			
			#if (defined(WIN32) || defined (WIN64))
				#define TRACE_DEFAULT(...)            trace_default(_STR2WSTR(__FILE__),__LINE__,_STR2WSTR(__FUNCTION__),__VA_ARGS__)
			#else
				#define TRACE_DEFAULT(...)            do { \
														   wchar_t *wscBuffer__PRETTY_FUNCTION__;\
														   wchar_t *wscBuffer__FILE__;\
														   wscBuffer__PRETTY_FUNCTION__ = new wchar_t[strlen(__PRETTY_FUNCTION__)]; \
														   wscBuffer__FILE__            = new wchar_t[strlen(__FILE__)]; \
														   if ((wscBuffer__PRETTY_FUNCTION__ != NULL) && (wscBuffer__FILE__ != NULL))\
															{\
																mbstowcs(wscBuffer__PRETTY_FUNCTION__,__PRETTY_FUNCTION__,strlen(__PRETTY_FUNCTION__)); \
																mbstowcs(wscBuffer__FILE__,__FILE__,strlen(__FILE__)); \
														        trace_default(wscBuffer__FILE__,__LINE__,wscBuffer__PRETTY_FUNCTION__,__VA_ARGS__); \
															}\
															else\
															{\
																trace_default(L"__FILE__",__LINE__,L"__FUNCTION__",__VA_ARGS__); \
															}\
													        if (wscBuffer__PRETTY_FUNCTION__ != NULL) delete [] wscBuffer__PRETTY_FUNCTION__;\
													        if (wscBuffer__FILE__ != NULL)            delete [] wscBuffer__FILE__;\
														} while(0)
			#endif
		#endif
	#else
		#define TRACE_DEFAULT(...)
	#endif

#endif /* _CPP_TRACE_H__ */
