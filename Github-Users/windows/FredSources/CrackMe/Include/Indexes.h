#ifndef __INDEXES__H__
#define __INDEXES__H__

#include <stddef.h>

namespace ns_Indexes
{

	template<unsigned int... I>
	struct Indexes { using type = Indexes<I..., sizeof...(I)>; };

	template<size_t N>
	struct Make_Indexes { using type = typename Make_Indexes<N - 1>::type::type; };

	template<>
	struct Make_Indexes<0u> { using type = Indexes<>; };

}

#endif

