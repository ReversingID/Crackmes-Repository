#ifndef __ObfuscatedAddress_H__
#define __ObfuscatedAddress_H__
#include "Indexes.h"

#ifndef ALWAYS_INLINE
	#if defined(_MSC_VER)
		#define ALWAYS_INLINE __forceinline
	#else
		#define ALWAYS_INLINE __attribute__((always_inline))
	#endif
#endif

#ifndef NEVER_INLINE
	#if defined(_MSC_VER)
 		#define NEVER_INLINE  __declspec(noinline)
	#else
 		#define NEVER_INLINE  __attribute__ ((noinline))
	#endif
#endif

/* Helper */
#define OBFUSCATED_FUNCTION_OFFSET(function,offset)        (ns_obfuscatedFunction::ObfuscatedFunction<offset>(function))
#define DEOBFUSCATED_FUNCTION_OFFSET(function,offset)      (ns_obfuscatedFunction::DeObfuscatedFunction(function,offset))
#define OBFUSCATED_FUNCTION(function)                      ([]\
		{\
			struct TOStruct { uintptr_t funct = reinterpret_cast<uintptr_t>(function); };\
			return ns_obfuscatedFunction::MakeObfuscatedFunction<GET_RANDOM_VALUE(uintptr_t),TOStruct>(function);\
		}())
#define DEOBFUSCATED_FUNCTION(InstanceObfuscatedFunction)  (InstanceObfuscatedFunction.original())

#if defined(__GNUC__)
	// GCC O3 is doing very strange things that are sometimes wrong or that remove the obfuscation. So use O1.
	#ifndef _DEBUG
		#pragma GCC push_options
		#pragma GCC optimize("O1")
	#endif
#endif

namespace ns_obfuscatedFunction {

/* Obfuscation without memory */
template <uintptr_t offset,typename F >
constexpr ALWAYS_INLINE F ObfuscatedFunction(F function)
{
	return reinterpret_cast<F>(reinterpret_cast<uintptr_t>(function) + offset);
}

template <typename F >
F  DeObfuscatedFunction(F function,uintptr_t offset)
{
	return reinterpret_cast<F>(reinterpret_cast<uintptr_t>(function) - offset);
}

/* Obfuscation with memory */

template<uintptr_t offset,typename FunctionStruct,typename F>
struct S_ObfuscatedFunction
{
  public :

	typedef F  type;
	static F const value  ;
	//constexpr static F const value = OBFUSCATED_FUNCTION_OFFSET(reinterpret_cast<F>(FunctionStruct().funct),offset) ;
	//enum :F { value  = reinterpret_cast<uintptr_t>(OBFUSCATED_FUNCTION_OFFSET(reinterpret_cast<F>(FunctionStruct().funct),offset)) } ;

	constexpr S_ObfuscatedFunction()  { }

	F original() const  {
		F func = S_ObfuscatedFunction<offset,FunctionStruct,F>::value;
		volatile uintptr_t off = offset;
		return DEOBFUSCATED_FUNCTION_OFFSET(func,off);
	}

};

template<uintptr_t offset,typename FunctionStruct,typename F>
#if (defined(WIN32) || defined (WIN64))
	F const S_ObfuscatedFunction<offset, FunctionStruct, F>::value  =
#else
	F const S_ObfuscatedFunction<offset, FunctionStruct, F>::value  =
#endif
OBFUSCATED_FUNCTION_OFFSET(reinterpret_cast<F>(FunctionStruct().funct),offset);

template<uintptr_t offset,typename FunctionStruct,typename F>
static constexpr S_ObfuscatedFunction<offset,FunctionStruct,F> MakeObfuscatedFunction(F f)
{
	return S_ObfuscatedFunction<offset,FunctionStruct,F>();
}


} /*end namespace*/

#if defined(__GNUC__)
	#ifndef _DEBUG
		#pragma GCC pop_options
	#endif
#endif


#endif
