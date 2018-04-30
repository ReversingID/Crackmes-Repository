#ifndef __KEY__H__
#define __KEY__H__

#include "Indexes.h"
#include <climits>
#include <stdint.h>
//#define NULL_KEY
//#ifndef _DEBUG
//	#undef NULL_KEY
//#endif

#ifndef STRINGIFIER
	#define STRINGIFIER_(x) #x
	#define STRINGIFIER(x) STRINGIFIER_(x)
#endif

namespace ns_Key
{
	using namespace ns_Indexes;

	//----------------------------------------------------
	// Seed
	//----------------------------------------------------
	template<unsigned char... seed>
	struct Seed {};

	//----------------------------------------------------
	// sumSeed
	//----------------------------------------------------
	template <typename Seed>
	struct sumSeed {};

	template <unsigned char first, unsigned char...  rest>
	struct sumSeed < Seed<first, rest...> >
	{
		#ifndef NULL_KEY
			static constexpr  unsigned int value = ((sumSeed< Seed<rest...> >::value<<8) + (first^(sumSeed< Seed<rest...> >::value&0xFF))  )%(UINT_MAX) ;
		#else
			static constexpr  unsigned int value = 0;
		#endif
	};
	template <unsigned char first >
	struct sumSeed< Seed <first > >
	{
		#ifndef NULL_KEY
			static constexpr  unsigned int value = first ;
		#else
			static constexpr  unsigned int value = 0;
		#endif
	};

	//----------------------------------------------------
	// Make Seed from Str
	//----------------------------------------------------
	template <typename Str, unsigned int ... indices>
	decltype(auto) build_seed(ns_Indexes::Indexes<indices...>)
	{
		return Seed<Str().chars[indices]...>();
	}

	//----------------------------------------------------
	// Key;
	//----------------------------------------------------
	template <unsigned char... key>
	struct Key { };

	//----------------------------------------------------
	// CKeyArray;
	//----------------------------------------------------
	template<typename Key>
	struct CKeyArray;

	template<unsigned char... key>
	struct CKeyArray<Key<key...> > {
		enum { size = sizeof...(key) };
		static char const  value[size + 4];
	};
	template<unsigned char... key>
	char const CKeyArray<Key<key...> >::value[] = { key... , '\0' };


	//----------------------------------------------------
	// getElemKey
	//----------------------------------------------------
	template<int index, typename Key>
	struct getElemKey;

	template <int index, unsigned char firstElem, unsigned char... restElem >
	struct getElemKey <index, Key<firstElem, restElem...>  >
	{
		enum { value = getElemKey<index - 1, Key<restElem...> >::value };
	};

	template<unsigned char firstElem, unsigned char... restElem>
	struct getElemKey <0, Key<firstElem, restElem...>  >
	{
		enum { value = firstElem };
	};

	//----------------------------------------------------
	// RANDOM
	//----------------------------------------------------
	// 1988, Stephen Park and Keith Miller
	// "Random Number Generators: Good Ones Are Hard To Find", considered as "minimal standard"
	// Park-Miller 31 bit pseudo-random number generator, implemented with G. Carta's optimisation:
	// with 32-bit math and without division

	template<unsigned int N,typename Seed>
	struct MetaRandomGenerator
	{
	private:
		static constexpr unsigned a = 16807;        // 7^5
		static constexpr unsigned m = 2147483647;   // 2^31 - 1

		static constexpr unsigned s = MetaRandomGenerator<N - 1,Seed>::value;
		static constexpr unsigned lo = a * (s & 0xFFFF);                // Multiply lower 16 bits by 16807
		static constexpr unsigned hi = a * (s >> 16);                   // Multiply higher 16 bits by 16807
		static constexpr unsigned lo2 = lo + ((hi & 0x7FFF) << 16);     // Combine lower 15 bits of hi with lo's upper bits
		static constexpr unsigned hi2 = hi >> 15;                       // Discard lower 15 bits of hi
		static constexpr unsigned lo3 = lo2 + hi;

	public:
		static constexpr unsigned max = m;
		static constexpr unsigned value = lo3 > m ? lo3 - m : lo3;
	};

	//----------------------------------------------------
	// MetaRandomGenerator<0u>
	//----------------------------------------------------
	template< typename Seed>
	struct MetaRandomGenerator<0u, Seed >
	{
		static constexpr unsigned int value = sumSeed< Seed >::value;
	};

	template<unsigned int N,typename Seed>
	struct MetaRandom
	{
		static constexpr unsigned int value = MetaRandomGenerator<N + 1, Seed>::value;
	};

	//----------------------------------------------------
	// Concat char and Key
	//----------------------------------------------------
	template<unsigned char first, typename Key>
	struct ConcatKey;

	template<unsigned char first, unsigned char... rest>
	struct ConcatKey< first, Key<rest...> >
	{
		typedef Key<first, rest...> Type;
	};

	//----------------------------------------------------
	// Create random Encoding key:
	//----------------------------------------------------
	template<size_t taille, unsigned int N,typename Seed>
	struct MetaRandomKey
	{
		// Limit template recursion : 500
		#define LIMIT_RECURSION  500
		typedef  typename ConcatKey< static_cast<unsigned char>(MetaRandom<N, Seed>::value & 0xFF), typename MetaRandomKey<taille - 1,((N+1)*(__COUNTER__+1)*taille)%LIMIT_RECURSION , Seed>::Type >::Type  Type;
	};
	template<unsigned int N, typename Seed>
	struct MetaRandomKey<0u, N,Seed>
	{
		typedef  Key<> Type;
	};


    //----------------------------------------------------
	// getRandomValue
	//----------------------------------------------------
	template <typename T,typename Key>
	struct getRandomValueFromKey {};

	template <typename T,unsigned char first, unsigned char...  rest>
	struct getRandomValueFromKey < T, Key<first, rest...> >
	{
		static constexpr T value = (((getRandomValueFromKey <T,Key<rest...> >::value)<<8) + first ) ;
	};
	template <typename T,unsigned char first >
	struct getRandomValueFromKey< T,Key <first > >
	{
		static constexpr T value = first ;
	};


template <typename T,typename Seed >
	struct getRandomValue
	{
		static constexpr T value = getRandomValueFromKey<T,
														 typename ns_Key::MetaRandomKey<sizeof(T),
																						__COUNTER__,
																						Seed
																						>::Type
														>::value ;
	};
}

#define GET_RANDOM_VALUE(T)  (ns_Key::getRandomValue<T,\
													ns_Key::Seed<__TIME__[0],__TIME__[1],__TIME__[2],__TIME__[3],__TIME__[4],__TIME__[5],__TIME__[6],__TIME__[7],\
																 __LINE__&0xFF,__COUNTER__&0xFF,(__LINE__*__COUNTER__)&0xFF,(__LINE__/__COUNTER__)&0xFF,\
																 __FILE__[0],__FILE__[sizeof(__FILE__)-1],__FILE__[(sizeof(__FILE__)-1)/2],\
																 __FILE__[((sizeof(__FILE__)-1)/2)+((sizeof(__FILE__)-1)/4)],__FILE__[((sizeof(__FILE__)-1)/2)-((sizeof(__FILE__)-1)/4)]>\
													>::value)

#endif
