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

#ifndef _stdexcept2_b20f8ff9_4d70_4262_a60a_a615f3fbc8b3
#define _stdexcept2_b20f8ff9_4d70_4262_a60a_a615f3fbc8b3

#if defined(_MSC_VER) && _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <sstream>
#include <string>
#include <stdexcept>

// the mixin class for exception
struct call_stack
{
	call_stack (unsigned skip = 3);
	static std::string get(unsigned skip = 2);
	//static void stream(std::ostream&, unsigned skip = 2);
	const char * stack_trace() const { return m_s.c_str(); }
private:
	std::string m_s;
};

#if defined(_MSC_VER)
#pragma inline_depth(0) // must disable inlining due to using skip value
#endif
class exception2 : public std::logic_error, public call_stack
{
public:
	exception2(const std::string& s, unsigned skip = 4 /* -1 for disable call stack*/)
		: std::logic_error(s), call_stack(skip)
	{}
	virtual ~exception2() throw() {}
};

#if defined(_MSC_VER)
#pragma inline_depth() // restore back the default value
#endif

#endif //_stdexcept2_b20f8ff9_4d70_4262_a60a_a615f3fbc8b3
