//
//  ObfuscatedCall.h
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

#ifndef ObfuscatedCall_h
#define ObfuscatedCall_h

#if (defined(WIN32) || defined(WIN64))
#include <windows.h>
#else
#include <unistd.h>
#endif
#include "CTrace.h"
#include "MetaFSM.h"
#include "Key.h"
#include "Indexes.h"
#include "Unroller.h"
#include "ObfucatedFunction.h"
#include "OS.h"

#define OBFUSCATED
#ifndef _DEBUG
	#define OBFUSCATED
#endif


#define NB_STEP_MAX 1024
#define NB_STEP_MIN 512


#if ((NB_STEP_MIN >= NB_STEP_MAX) || (NB_STEP_MIN <= 0 ) || (NB_STEP_MAX<= 0) )
	#error Configuration obfuscation MIN MAX
#endif

// Obfuscate function call with a finite state machine (FSM).
// This is only a (simplified) example to show the principle
// In this example, the target is called at the end of the FSM so it can be located.
// In production, it would be better to put it in the middle of the FSM with some computing triggering it.

namespace ns_obfuscatedCall {
	using namespace ns_Key;
	using namespace ns_Indexes;
    using namespace ns_metaFSM;
    using namespace ns_unroller;

    // Finite State Machine
    // E: Event associated with target
    // R: Type of return value
    template<typename E, typename R = Void>
    struct Machine : public msm::front::state_machine_def<Machine<E, R>>
    {
        // --- Events
        struct event1 {};
        struct event2 {};
        struct event3 {};
        struct event4 {};
        struct event5 {};
        struct event6 {};
        
        // --- States
        struct State1 : public msm::front::state<>{};
        struct State2 : public msm::front::state<>{};
        struct State3 : public msm::front::state<>{};
        struct State4 : public msm::front::state<>{};
        struct State5 : public msm::front::state<>{};
        struct State6 : public msm::front::state<>{};
        
        // --- Transitions
        struct CallTarget
        {
            template<typename EVT, typename FSM, typename SRC, typename TGT>
            void operator()(EVT const& evt, FSM& fsm, SRC& src, TGT& tgt)
            {
            	TRACE_DEFAULT(_STR2WSTR("CallTarget reached"));
                fsm.result_ = evt.call();
            }
        };
        
        struct CallNone1
        {
            template<typename EVT, typename FSM, typename SRC, typename TGT>
            void operator()(EVT const& evt, FSM& fsm, SRC& src, TGT& tgt)
            {
            	//TRACE_DEFAULT(_STR2WSTR("CallNone1"));
				//Sleep(0);
            }
        };

        struct CallNone2
        {
            template<typename EVT, typename FSM, typename SRC, typename TGT>
            void operator()(EVT const& evt, FSM& fsm, SRC& src, TGT& tgt)
            {
            	//TRACE_DEFAULT(_STR2WSTR("CallNone2"));
				//Sleep(0);
            }
        };

        struct CallNone3
        {
            template<typename EVT, typename FSM, typename SRC, typename TGT>
            void operator()(EVT const& evt, FSM& fsm, SRC& src, TGT& tgt)
            {
            	//TRACE_DEFAULT(_STR2WSTR("CallNone3"));
				//Sleep(0);
            }
        };

        // --- Initial state of the FSM. Must be defined
        using initial_state = State1;
        
        // --- Transition table
        struct transition_table : mpl::vector<
        //    Start     Event         Next      Action               Guard
        //  +---------+-------------+---------+---------------------+----------------------+
        Row < State1  , event5      , State2  ,   CallNone3                                >,
        Row < State1  , event1      , State3                                               >,
        //  +---------+-------------+---------+---------------------+----------------------+
        Row < State2  , event2      , State4,     CallNone1                                >,
        //  +---------+-------------+---------+---------------------+----------------------+
        Row < State3  , none        , State3                                               >,
        //  +---------+-------------+---------+---------------------+----------------------+
        Row < State4  , event4      , State1,     CallNone2                                >,
        Row < State4  , event3      , State5                                               >,
        //  +---------+-------------+---------+---------------------+----------------------+
        Row < State5  , E           , State6,     CallTarget                               >,
        //  +---------+-------------+---------+---------------------+----------------------+
        Row < State6  , event6      , State1                                               >
        //  +---------+-------------+---------+---------------------+----------------------+
        > {};
        
        using StateMachine = msm::back::state_machine<Machine<E, R>>;
        
        template<typename F, typename... Args>
        struct Run
        {
            static inline void run(StateMachine& machine, F f, Args&&... args)
            {
                // This is just an example of what is possible. In actual production code it would be better to call event E in the middle of this loop and to make transitions more complex.
                machine.start();

                Unroller<(GET_RANDOM_VALUE(unsigned int)%(NB_STEP_MAX-NB_STEP_MIN))+(NB_STEP_MIN/2)+1>{}([&]()
                {
                    machine.process_event(event5{});
                    machine.process_event(event2{});
                    machine.process_event(event4{});
                });

                machine.process_event(event5{});
                machine.process_event(event2{});
                machine.process_event(event3{});

                // This will call our target. In actual production code it would be better to call event E in the middle of the FSM processing.
                machine.process_event(E{f, args...});

                machine.process_event(event6{});

                Unroller<(GET_RANDOM_VALUE(unsigned int)%(NB_STEP_MAX-NB_STEP_MIN))+(NB_STEP_MIN/2)+1>{}([&]()
                {
                    machine.process_event(event5{});
                    machine.process_event(event2{});
                    machine.process_event(event4{});
                });

            }
        };
        
        // Result of the target
        R result_;
    };

}

// Warning: ##__VA_ARGS__ is not portable (only __VA_ARGS__ is). However, ##__VA_ARGS__ is far better (handles cases when it is empty) and supported by most compilers

#define OBFUSCATED_CALL(f, ...)     (ns_obfuscatedCall::ObfuscatedCallVoid<ns_obfuscatedCall::Machine> (OBFUSCATED_FUNCTION(f), ##__VA_ARGS__))
#define OBFUSCATED_CALL_RET(f, ...) (ns_obfuscatedCall::ObfuscatedCallRet<ns_obfuscatedCall::Machine>  (OBFUSCATED_FUNCTION(f), ##__VA_ARGS__))

#endif
