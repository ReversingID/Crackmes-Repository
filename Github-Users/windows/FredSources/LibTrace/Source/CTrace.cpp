#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <CTrace.h>
#include <string.h>
#include <stdlib.h>
#include <sys/timeb.h>
#include <time.h>
#ifdef UNICODE
	#include <wchar.h>
#endif

#include <version.h>
#if (defined(WIN32) || defined (WIN64) || defined (__MINGW32__) || defined (__MINGW64__))
	#include <windows.h>
#else
    #include <sys/types.h>
    #include <sys/wait.h>
	#include <signal.h>
    #include <sys/stat.h>
    #include <fcntl.h>
	#include <unistd.h>
	#include <errno.h>
#endif

#include <mutex>
#include <functional>
#include <memory>
#include <utility>

using std::call_once;
using std::once_flag;

//#define DEBUG_STDERR
#define AUTO_ACTIVATION true

#if ((AUTO_ACTIVATION != true) && (AUTO_ACTIVATION != false))
	#error "CTrace :  AUTO_ACTIVATION [true|false]"
#endif

#ifdef _DEBUG
	#if ((defined(LIN32) || defined (LIN64)) &&  !defined (__MINGW32__) &&  !defined (__MINGW64__) )
		#define __XTERM_COMMAND_SIZE__ 1024
	#endif
	template <class T>
	class Singleton_Static
	{
	public:
		template <typename... Args>
		static T& getInstance(Args&&... args) 
		{
			call_once( get_once_flag(), [](Args&&... args) { instance_ = new T(std::forward<Args>(args)...); },	std::forward<Args>(args)...);
			return *instance_;
		}

	protected:
		explicit Singleton_Static<T>() {}
		~Singleton_Static<T>() { if (instance_ != nullptr) { delete instance_; } }

	private:
		static T *instance_;
		static once_flag& get_once_flag() {
			static once_flag once_;
			return once_;
		}

		Singleton_Static(const Singleton_Static&) = delete;
		const Singleton_Static& operator=(const Singleton_Static&) = delete;
	};
	template<class T> T* Singleton_Static<T>::instance_ = nullptr;

    class CTrace : public Singleton_Static<CTrace>
    {
		friend class Singleton_Static<CTrace>;
	    private :
			bool running;
			bool status = false;
			#if ((defined(LIN32) || defined (LIN64)) &&  !defined (__MINGW32__) &&  !defined (__MINGW64__) )
				#ifndef DEBUG_STDERR 
					char * m_filename;
					char m_command[__XTERM_COMMAND_SIZE__];
					int pid ;

				#endif
			#endif

	    private :
		    CTrace(char * _filename = (char *)"TRACE_LOG")
		    {
		    	start(_filename);
		    	atexit(TRACE_STOP);
		    }
		public:
		    void start(char * _filename)
		    {
		    	if (status == true)
		    	{
		    		return;
		    	}
				status = true;
				running = AUTO_ACTIVATION;
				#if ((defined(LIN32) || defined (LIN64)) &&  !defined (__MINGW32__) &&  !defined (__MINGW64__) )
					#ifndef DEBUG_STDERR
						int len = 0;
						char * HOME =getenv("HOME");

						if ((HOME != NULL) && (_filename != NULL))
						{
							len = strlen(_filename) + strlen(HOME) + 2;
							m_filename = new char [len];
							if (m_filename != NULL)
							{
								memset(m_filename,0,len);
								sprintf(m_filename,"%s/%s",HOME,_filename);
							}
						}
						else
						{
							m_filename = NULL;
						}

						if (m_filename != NULL)
						{
							int ret = mkfifo(m_filename, S_IRWXU | S_IRWXG | S_IRWXO);
							if ((ret == -1 && errno == EEXIST) || (ret == 0))
							{
								memset(m_command,0, __XTERM_COMMAND_SIZE__);
								snprintf(m_command,__XTERM_COMMAND_SIZE__,"tail -f %s",m_filename);
								pid = fork();
								if (pid == 0)
								{
									//execlp("xterm","-l","-e",m_command,NULL);
									execlp("xterm","xterm",
												"-bg", "rgb:b2/b2/b2",
												"-fg", "rgb:08/08/08",
												"-rightbar","-sb",
												"-fa", "'fixed'",
												"-fs", "8",
												"-geometry","240x35",
												"-l",
												"-e",m_command,
												NULL);
									perror("Problème : cette partie du code ne doit jamais être exécutée");
								}
							}
							else
							{
								delete [] m_filename;
								m_filename = NULL ;
							}
						}
					#endif
				#endif
		    }

		    void stop()
		    {
		    	if (status == false)
		    	{
		    		return;
		    	}
				#if ((defined(LIN32) || defined (LIN64)) &&  !defined (__MINGW32__) &&  !defined (__MINGW64__) )
					#ifndef DEBUG_STDERR
						if (m_filename != NULL)
						{
							kill (pid, SIGKILL);
							waitpid(pid,NULL,WUNTRACED|WCONTINUED);
							unlink (m_filename);
							delete [] m_filename;
							m_filename = NULL;
						}
						status = false;
						running = false;
					#endif
				#endif
		    }

		    virtual ~CTrace()
		    {
		    	 stop();
		    }
			void Activate()
			{
				running = true;
			}
			void Deactivate()
			{
				running = false;
			}
			#ifndef UNICODE
		        void Write(char * cMessage)
		        {
					if (running == false) { return; }
					#if (defined(WIN32) || defined (WIN64) || defined (__MINGW32__) || defined (__MINGW64__))
						#ifndef  DEBUG_STDERR
							if (IsDebuggerPresent())
							{
								OutputDebugStringA(cMessage);
							}
						#else
							fprintf(stderr, "%s", cMessage);
						#endif
					#else
						#ifndef DEBUG_STDERR
							if (m_filename != NULL)
							{
									FILE* fd = fopen(m_filename,"w");
									if (fd != NULL)
									{
									   fprintf(fd,"%s",cMessage);
									   fclose(fd);
									}

							}
						#else
							fprintf(stderr,"%s",cMessage);
						#endif
					#endif
		        }
		    #else
		        void Write(wchar_t * cMessage)
		        {
					if (running == false) { return; }
					#if (defined(WIN32) || defined (WIN64) || defined (__MINGW32__) || defined (__MINGW64__))
						#ifndef  DEBUG_STDERR
							if (IsDebuggerPresent())
							{
								OutputDebugStringW(cMessage);
							}
						#else
							fwprintf(stderr, L"%ls", cMessage);
						#endif
					#else
						#ifndef DEBUG_STDERR
						if (m_filename != NULL)
						{
								FILE* fd = fopen(m_filename,"w");
								if (fd != NULL)
								{
								   fwprintf(fd,L"%ls",cMessage);
								   fclose(fd);
								}
						}
					#else
						fwprintf(stderr,L"%ls",cMessage);
					#endif
					#endif
		        }
			#endif
    };
#endif

#ifdef _DEBUG
void TRACE_ACTIVATE() {
		return CTrace::getInstance().Activate();
}

void TRACE_DEACTIVATE() {
	return CTrace::getInstance().Deactivate();
}

void TRACE_START(char * filename) {
	return CTrace::getInstance().start(filename);
}

void TRACE_STOP() {
	return CTrace::getInstance().stop();
}

#endif
#ifdef _DEBUG
	#if ((defined(LIN32) || defined (LIN64)) &&  !defined (__MINGW32__) &&  !defined (__MINGW64__) )
		#ifndef UNICODE
			// Permet le calcul de la taille max de la chaine
			int _vscprintf(const char * aFormat,va_list theArgList)
			{
				int iReste;
				char *p, *np;
				va_list theArgListCopy;
				int iSize = -1;

				iSize = strlen(aFormat)*2;
				if ((p = (char *)malloc(iSize*sizeof(char))) != NULL)
				{
				   iReste = -1 ;
				   do
				   {
					   va_copy(theArgListCopy,theArgList);
					   iReste = vsnprintf(p, iSize, aFormat, theArgListCopy);
					   va_end(theArgListCopy);
					   if (!(iReste > -1 && iReste < iSize))
					   {
						   if (iReste > -1)
							   iSize = iReste+1;
						   else
							   iSize *= 2;
						   if ((np =(char *)realloc(p, iSize*sizeof(char))) == NULL)
						   {
							   iSize = -1;
							   break;
						   }
						   else
						   {
							   p = np;
							   np = NULL;
						   }
					   }
				   } while (!(iReste > -1 && iReste < iSize));
				   if (p!=NULL)
				   {
					   free(p);
					   p=NULL;
				   }
				}
				else
				{
					iSize = -1;
				}
				return iSize ;
			}
		#else /* UNICODE */
			// Permet le calcul de la taille max de la chaine
			int _vscwprintf(const wchar_t * aFormat,va_list theArgList)
			{
				int iReste;
				wchar_t *p, *np;
				va_list theArgListCopy;
				int iSize = -1;

				iSize = wcslen(aFormat)*2;
				if ((p = (wchar_t *)malloc(iSize*sizeof(wchar_t))) != NULL)
				{
				   iReste = -1 ;
				   do
				   {
					   va_copy(theArgListCopy,theArgList);
					   iReste = vswprintf(p, iSize, aFormat, theArgListCopy);
					   va_end(theArgListCopy);
					   if (!(iReste > -1 && iReste < iSize))
					   {
						   if (iReste > -1)
							   iSize = iReste+1;
						   else
							   iSize *= 2;
						   if ((np =(wchar_t *)realloc(p, iSize*sizeof(wchar_t))) == NULL)
						   {
							   iSize = -1;
							   break;
						   }
						   else
						   {
							   p = np;
							   np = NULL;
						   }
					   }
				   } while (!(iReste > -1 && iReste < iSize));
				   if (p!=NULL)
				   {
					   free(p);
					   p=NULL;
				   }
				}
				else
				{
					iSize = -1;
				}
				return iSize ;
			}
		#endif

	#endif /* WIN32 */

	#if ((defined(WIN32) || defined (WIN64)) &&  (!defined (__MINGW32__) &&  !defined (__MINGW64__)))
		#pragma warning (disable : 4996)
	#endif /* WIN32 */

	#ifndef UNICODE
		int trace_default(const char * file,int line,const char * fonction,const char * aFormat,...)
		{
			int iRet = -1;
			char * cMessage;
			char format[] = "%s :: %s:%d :: %s ";
			size_t len_int ;
			size_t len_format;
			size_t iSize ;
			size_t len_total;

			char cChaineTime[256];
			timeb timebuffer; 
			struct tm * TimeStruct;

			va_list theArgList;
			va_start(theArgList,aFormat);

			ftime(&timebuffer);
			TimeStruct = localtime(&timebuffer.time);
			sprintf(cChaineTime,"%02d/%02d/%04d %02d:%02d:%02d.%03d",TimeStruct->tm_mday,(TimeStruct->tm_mon)+1,(TimeStruct->tm_year)+1900,TimeStruct->tm_hour,TimeStruct->tm_min,TimeStruct->tm_sec,timebuffer.millitm);

			len_format = _vscprintf( aFormat, theArgList );
			len_int    = (int)(log10((float)line)+1);
			len_total  = strlen(file) + strlen(fonction) + strlen(cChaineTime) + len_int + len_format + strlen(format) + ALIGNEMENT + 10 /* marge */  ;

			cMessage = new char [len_total] ;
			if (cMessage != NULL)
			{
				memset(cMessage, 0, len_total);
				sprintf(cMessage,format,cChaineTime,file,line,fonction);
				iSize = strlen(cMessage);
				cMessage[iSize] = ' ' ;
				cMessage[iSize+1] = '\0' ;

				while (strlen(cMessage)<=ALIGNEMENT)
				{
					iSize = strlen(cMessage);
					cMessage[iSize] = ' ' ;
					cMessage[iSize+1] = '\0' ;
				}

				iSize = strlen(cMessage);
				cMessage[iSize] = '=' ;
				cMessage[iSize+1] = '>' ;
				cMessage[iSize+2] = ' ' ;
				cMessage[iSize+3] = '\0' ;

				vsprintf(cMessage+(strlen(cMessage)), aFormat, theArgList);
				iSize = strlen(cMessage);
				cMessage[iSize] = '\n' ;
				cMessage[iSize+1] = '\0' ;

				// Affichage du messsage de trace
				CTrace::getInstance().Write(cMessage);
				delete [] cMessage;
				iRet = 0;
			}
			return iRet;
		}
	#else /* UNICODE */

 		int trace_default(const wchar_t * file,int line,const wchar_t * fonction,const wchar_t * aFormat,...)
		{
			int iRet = -1;
			wchar_t * cMessage;
			wchar_t format[] = L"%ls :: %ls:%d :: %ls ";
			size_t len_int ;
			size_t len_format;
			size_t iSize ;
			size_t len_total;

			wchar_t cChaineTime[256];
			timeb timebuffer; 
			struct tm * TimeStruct;

			va_list theArgList;
			va_start(theArgList,aFormat);

			ftime(&timebuffer);
			TimeStruct = localtime(&timebuffer.time);
			memset(cChaineTime, 0, 256);
			swprintf(cChaineTime,256,L"%02d/%02d/%04d %02d:%02d:%02d.%03d",TimeStruct->tm_mday,(TimeStruct->tm_mon)+1,(TimeStruct->tm_year)+1900,TimeStruct->tm_hour,TimeStruct->tm_min,TimeStruct->tm_sec,timebuffer.millitm);

			len_format = _vscwprintf( aFormat, theArgList );
			len_int    = (int)(log10((float)line)+1);
			len_total  = wcslen(file) + wcslen(fonction) + wcslen(cChaineTime) + len_int + len_format + wcslen(format) + ALIGNEMENT + 10 /* marge */  ;

			cMessage = new wchar_t [ len_total] ;
			if (cMessage != NULL)
			{
				memset(cMessage, 0, len_total);
				swprintf(cMessage,len_total,format,cChaineTime,file,line,fonction);
				iSize = wcslen(cMessage);
				cMessage[iSize] = wchar_t(' ') ;
				cMessage[iSize+1] = wchar_t('\0') ;

				while (wcslen(cMessage)<=ALIGNEMENT)
				{
					iSize = wcslen(cMessage);
					cMessage[iSize] = wchar_t(' ') ;
					cMessage[iSize+1] = wchar_t('\0') ;
				}

				iSize = wcslen(cMessage);
				cMessage[iSize] = '=' ;
				cMessage[iSize+1] = '>' ;
				cMessage[iSize+2] = ' ' ;
				cMessage[iSize+3] = '\0' ;

				vswprintf(cMessage+(wcslen(cMessage)),len_total-wcslen(cMessage), aFormat, theArgList);
				iSize = wcslen(cMessage);
				cMessage[iSize] = '\n' ;
				cMessage[iSize+1] = '\0' ;

				// Affichage du messsage de trace
				CTrace::getInstance().Write(cMessage);

				delete [] cMessage;
				iRet = 0;
			}
			return iRet;
		}

	#endif /* UNICODE */

	#if ((defined(WIN32) || defined (WIN64)) &&  (!defined (__MINGW32__) &&  !defined (__MINGW64__)))
		#pragma warning (default : 4996)
	#endif /* WIN32 */
#endif

