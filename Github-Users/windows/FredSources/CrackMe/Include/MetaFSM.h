//
//  MetaFSM.h
//  ADVobfuscator
//
// Copyright (c) 2010-2014, Sebastien Andrivet
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Get latest version on https://github.com/andrivet/ADVobfuscator

#ifndef MetaFSM_h
#define MetaFSM_h

#include <iostream>
#include <tuple>
#include <type_traits>
#include <boost/msm/back/state_machine.hpp>
#include <boost/msm/front/state_machine_def.hpp>
// functors
#include <boost/msm/front/functor_row.hpp>
#include <boost/msm/front/euml/common.hpp>

#include "Key.h"
#include "Indexes.h"
#include "Unroller.h"
#include "ObfucatedFunction.h"

// Code common to our FSM (finite state machines)

namespace msm = boost::msm;
namespace mpl = boost::mpl;
using namespace boost::msm::front;

namespace ns_metaFSM {
    using namespace ns_Indexes;

    // Same as void but can be instantiated
    struct Void {};

    // Event template to call a function F with a list of parameters.
    // Note: F is passed as value.
    template<typename R, typename F, typename... Args>
    struct event
    {
        // Constructor
        constexpr event(F f, Args&... args): f_{f}, data_{args...} {}
        
        // Call target function
        R call() const
        {
            // Generate a list of indexes to extract arguments from tuple
            using I = typename Make_Indexes<sizeof...(Args)>::type;
            return call_(I{});
        }
        
    private:
        // When F is returning a value
        template<typename U = R, unsigned int... I>
        typename std::enable_if<!std::is_same<U, Void>::value, U>::type
        call_(Indexes<I...>) const { return DEOBFUSCATED_FUNCTION(f_)(std::get<I>(data_)...); }
        
        // When F does not return a value (void)
        template<typename U = R, unsigned int... I>
        typename std::enable_if<std::is_same<U, Void>::value, Void>::type
        call_(Indexes<I...>) const { DEOBFUSCATED_FUNCTION(f_)(std::get<I>(data_)...); return Void{}; }
        
    private:
        F f_;
        std::tuple<Args&...> data_;
    };


    // When function F is returning a value
    // FSM: Finite State Machine
    // F: Function (target)
    // Args: Arguments of target
    template<template<typename, typename> class FSM, typename F, typename... Args>
    //template < typename R_ = typename std::result_of<typename F::type(Args...)>::type>
    //inline  typename std::enable_if<!std::is_void<R_>::value,R_>::type
    inline decltype(auto) ObfuscatedCallRet( F f, Args&&... args)
    {
    	using T = typename std::result_of<typename F::type(Args...)>::type ;
    	typedef typename std::conditional<std::is_void<T>::value, Void, T>::type R;
        using E = event< R, F, Args&...>;
        using M = msm::back::state_machine<FSM<E, R>>;
        using Run = typename FSM<E, R>::template Run<F, Args...>;

        M machine;
        Run::run(machine, f, std::forward<Args>(args)...);

        return machine.result_;
    }

    // When function F is not returning a value
    // FSM: Finite State Machine
    // F: Function (target)
    // Args: Arguments of target
    template<template<typename, typename = Void> class FSM, typename F, typename... Args>
    //template < typename R_ = typename std::result_of<typename F::type(Args...)>::type>
    //inline  typename std::enable_if<std::is_void<R_>::value,void>::type
    inline decltype(auto) ObfuscatedCallVoid(F f, Args&&... args)
    {
    	using T = typename std::result_of<typename F::type(Args...)>::type ;
    	typedef typename std::conditional<std::is_void<T>::value, Void, T>::type R;
        using E = event<R, F, Args&...>;
        using M = msm::back::state_machine<FSM<E, R>>;
        using Run = typename FSM<E, R>::template Run<F, Args...>;

        M machine;
        Run::run(machine, f, std::forward<Args>(args)...);
    }

}

#endif
