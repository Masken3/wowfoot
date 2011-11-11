/*
 Copyright (c) 2002
 Author: Konstantin Boukreev
 E-mail: konstantin@mail.primorye.ru
 Created: 16.01.2002 12:36:26
 Version: 1.0.0

 Permission to use, copy, modify, distribute and sell this software
 and its documentation for any purpose is hereby granted without fee,
 provided that the above copyright notice appear in all copies and
 that both that copyright notice and this permission notice appear
 in supporting documentation.  Konstantin Boukreev makes no representations
 about the suitability of this software for any purpose.
 It is provided "as is" without express or implied warranty.

 the exception class holding the call stack's information

 exception2 : std::exception

 try {
 }
 catch(exception2& ex) {
	std::cout << ex.trace_call();
 }

*/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "exception2.h"
#include "sym_engine.h"

call_stack::call_stack (unsigned skip)
: m_s((skip == unsigned(-1)) ? "" : call_stack::get(skip))
{
}

std::string call_stack::get(unsigned skip)
{
	std::stringstream ss(std::ios::out);
	sym_engine::stack_trace(ss, skip);
	return ss.str();
}

#if 0
void call_stack::stream(std::ostream& o, unsigned skip)
{
	sym_engine::stack_trace(o, skip);
}
#endif
