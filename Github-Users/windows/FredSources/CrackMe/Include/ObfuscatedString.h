#ifndef __STRING_ENCODER__H__
#define __STRING_ENCODER__H__

#include <climits>
#include "OS.h"
#include "Key.h"
#include "Indexes.h"
#include "AntiDebug.h"
#include "ObfucatedFunction.h"

#ifndef ALWAYS_INLINE
	#if defined(_MSC_VER)
		#define ALWAYS_INLINE __forceinline
	#else
		#define ALWAYS_INLINE __attribute__((always_inline))
	#endif
#endif

#define STATIC_DECODE(STR) (ns_EncodingString::decode(STR,sizeof(STR)))

namespace ns_EncodingString
{
	using namespace ns_Key;
	using namespace ns_Indexes;

	//----------------------------------------------------
	// decode
	//----------------------------------------------------

	void decode(ULONG_PTR adresseVar, size_t memorySize);

	//----------------------------------------------------
	// String
	//----------------------------------------------------
	template<unsigned char... string>
	struct String {};

	//----------------------------------------------------
	// Convert char Array to String
	//----------------------------------------------------
	template <typename Str, unsigned int ... indices>
	decltype(auto) build_string(ns_Indexes::Indexes<indices...>) {
		return ns_EncodingString::String<Str().chars[indices]...>();
	}

	//----------------------------------------------------
	// Concat char and String
	//----------------------------------------------------
	template< unsigned char first, typename String>
	struct ConcatString {};

	template< unsigned char first, unsigned char... rest>
	struct ConcatString< first, String<rest...> >
	{
		typedef String<first, rest...> Type;
	};

	//----------------------------------------------------
	// GetStringSize
	//----------------------------------------------------
	template<typename String>
	struct getStringSize {};

	template<char first, char... string >
	struct getStringSize< String<first, string... > >
	{
		enum { value = getStringSize<String<string... >>::value + 1 };
	};

	template< >
	struct getStringSize< String<> >
	{
		enum { value = 0 };
	};

	//----------------------------------------------------
	// GetElem String
	//----------------------------------------------------
	template<int index, typename String>
	struct getElemString {};

	template <int index, unsigned char firstElem, unsigned char... restElem >
	struct getElemString <index, String<firstElem, restElem...>  >
	{

		enum { value = getElemString<index - 1, String<restElem...> >::value };
	};

	template<unsigned char firstElem, unsigned char... restElem>
	struct getElemString <0, String<firstElem, restElem...>  >
	{
		enum { value = firstElem };
	};

	//----------------------------------------------------
	// Encode char
	//----------------------------------------------------
	template<unsigned char orig, int XorIndex, typename Key >
	struct Encode {};

	template<unsigned char orig, int XorIndex, unsigned char... key >
	struct Encode< orig, XorIndex, Key< key... > >
	{
		enum
		{
			value = static_cast<unsigned char> (orig ^ (ns_Key::getElemKey<(XorIndex%sizeof...(key)), Key<key...> >::value))
		};
	};

	//----------------------------------------------------
	// Encode String
	//----------------------------------------------------
	template<int count, typename Key, typename String>
	struct EncodeString {};

	template<int count, typename Key, unsigned char first, unsigned char... rest>
	struct EncodeString<count, Key, String<first, rest... > >
	{
		typedef typename ConcatString<Encode<first, count, Key>::value, typename EncodeString<count + 1, Key, String<rest...> >::Type >::Type Type;
	};

	template<int count, typename Key >
	struct EncodeString< count,  Key, String<> >
	{
		typedef String<> Type;
	};

	//----------------------------------------------------
	// Encode Key
	//----------------------------------------------------
	template<int count, typename localKey, typename Key>
	struct EncodeKey {};

	template<int count, typename localKey, unsigned char first, unsigned char... rest>
	struct EncodeKey<count, localKey, Key<first, rest... > >
	{
		typedef typename ConcatKey<Encode<first, count, localKey>::value, typename EncodeKey<count + 1, localKey, Key<rest...> >::Type >::Type Type;
	};

	template<int count, typename localKey>
	struct EncodeKey< count, localKey,  Key<>  >
	{
		typedef Key<> Type;
	};

    //----------------------------------------------------
	// sumOffset
	//----------------------------------------------------
	template <typename Key>
	struct sumOffset {};

	template <unsigned char first, unsigned char...  rest>
	struct sumOffset < Key<first, rest...> >
	{
		static constexpr  unsigned long value = (((sumOffset < Key<rest...> >::value)<<8) + first )%(ULONG_MAX) ;
	};
	template <unsigned char first >
	struct sumOffset< Key <first > >
	{
		static constexpr  unsigned long value = first ;
	};
	//----------------------------------------------------
	// Cdecode :
	//----------------------------------------------------

	template<unsigned int size,typename Key>
	struct Cdecode {};

	typedef size_t(*T_decrypt)(char*, size_t,char [4]);

	template<unsigned int stringSize, unsigned char... key>
	struct Cdecode <stringSize,Key<key...>>
	{
		static ALWAYS_INLINE size_t decrypt(char buffer_[stringSize], size_t index, char secret[4])
		{
			unsigned char localkey[sizeof...(key)] = { key... };
			if ((buffer_ != NULL) && (secret != NULL))
			{
				if (index < stringSize)
				{
					buffer_[index] = buffer_[index] ^ (localkey[index % sizeof...(key)] ^ secret[index % 4]);
				}
			}
			return stringSize;
		}
	};
	//----------------------------------------------------
	// Conversion S_Value :
	//----------------------------------------------------
	struct S_Value_entete
	{
		T_decrypt function;
		unsigned long offset;
		unsigned long mask;
		unsigned char marqueur[4];
		unsigned char secret[4];
	};
	template<typename Elem,unsigned int size>
	struct S_Value
	{
		S_Value_entete entete;
		Elem const  string[size+sizeof(Elem)];
	} ;

	//----------------------------------------------------
	// Conversion String to C values :
	//----------------------------------------------------

	template<typename Elem, typename Key, typename Indexes, typename String>
	struct CStringArray {};

	template<typename Elem, unsigned char... key,  unsigned int... indexes, unsigned char... string>
	struct CStringArray< Elem, Key<key...>, Indexes<indexes...>, String<string...> >
	{
		typedef typename ns_Key::MetaRandomKey<4, __COUNTER__, Seed<key...> >::Type LocalKey;
		typedef typename ns_Key::MetaRandomKey<sizeof(unsigned long), __COUNTER__, Seed<key...> >::Type LocalOffset;
		typedef typename ns_Key::MetaRandomKey<sizeof(unsigned long), __COUNTER__, Seed<key...> >::Type LocalMask;
		static struct S_Value<Elem, sizeof...(string)> const value ;
	};

	template<typename Elem, unsigned char... key, unsigned int... indexes, unsigned char... string>
#if (defined(WIN32) || defined (WIN64))
	MEMORY_PROTECT struct S_Value<Elem,sizeof...(string)> const CStringArray< Elem, Key<key...>, Indexes<indexes...>, String<string...> >::value =
#else
	constexpr struct S_Value<Elem,sizeof...(string)> const CStringArray< Elem, Key<key...>, Indexes<indexes...>, String<string...> >::value MEMORY_PROTECT =
#endif
	{
		{
			OBFUSCATED_FUNCTION_OFFSET((Cdecode<sizeof...(string),typename EncodeKey<0, LocalKey, Key<key...>>::Type>::decrypt),(sumOffset<LocalOffset>::value)),
			sumOffset<LocalOffset>::value ^ sumOffset<LocalMask>::value,
			sumOffset<LocalMask>::value,
			{
				'<' ^ getElemKey<0, LocalKey >::value,
				'@' ^ getElemKey<3, LocalKey >::value,
				'!' ^ getElemKey<1, LocalKey >::value,
				'>' ^ getElemKey<2, LocalKey >::value
			},
			{
				getElemKey<0, LocalKey >::value,
				getElemKey<1, LocalKey >::value,
				getElemKey<2, LocalKey >::value,
				getElemKey<3, LocalKey >::value
			}
		},
		{	string...,'\0'	}
	};
	//----------------------------------------------------
	// Conversion String to C values Encoded :
	//----------------------------------------------------
	template<typename Elem, typename Key, typename Indexes, typename String>
	struct CStringArrayEnc {};

	template<typename Elem, unsigned char... key,  unsigned int... indexes, unsigned char... string>
	struct CStringArrayEnc<Elem, Key<key...>,  Indexes<indexes...>, String<string...> >
	{
		typedef CStringArray<Elem, Key<key...>,  Indexes<indexes...>, typename EncodeString<0, Key<key...>, String<string...>>::Type > Type;
	};

}

#ifdef KEY_SEED_RANDOM
	#undef   KEY_SEED_RANDOM
#endif
#define  KEY_SEED_RANDOM(file,line) (__TIME__ ":" file  ":" STRINGIFIER(line*(__COUNTER__+1)) )



#define OBFUSCATED_STRING(str) []\
{\
        struct Str { const char * chars = str; };\
		struct StrSeed { const char * chars = KEY_SEED_RANDOM(__FILE__,__LINE__); } ;\
		return (reinterpret_cast<const char*>(ns_EncodingString::CStringArrayEnc< \
									unsigned char, \
									ns_Key::MetaRandomKey<\
															sizeof(str)-1,\
															__COUNTER__, \
															decltype(ns_Key::build_seed<StrSeed>(ns_Indexes::Make_Indexes<sizeof(KEY_SEED_RANDOM(__FILE__,__LINE__)) - 1>::type()))\
														 >::Type, \
									ns_Indexes::Make_Indexes<sizeof(str)-1>::type, \
									decltype(ns_EncodingString::build_string<Str>(ns_Indexes::Make_Indexes<sizeof(str)-1>::type()))\
									>::Type::value.string)); \
}()


#define STATIC_OBFUSCATED_STRING(str) []\
{\
	DWORD old_prot;\
	static const char * strenc = OBFUSCATED_STRING(str);\
	OS_MemoryProtect((LPVOID)strenc,sizeof(str), OS_PAGE_READWRITE , &old_prot);\
	(ns_EncodingString::decode((ULONG_PTR)(strenc),sizeof(str)-1));\
	return strenc;\
}()

#endif

