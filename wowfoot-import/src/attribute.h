#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#ifdef __GNUC__
#define ATTRIBUTE(a) __attribute((a))
#define NORETURN(func) void func __attribute((noreturn))
#else
#define ATTRIBUTE(a)
#define NORETURN(func) __declspec(noreturn) void func
#define abort() exit(1)
#endif

#endif	//ATTRIBUTE_H
