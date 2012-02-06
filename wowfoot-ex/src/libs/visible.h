#ifndef VISIBLE_H
#define VISIBLE_H

#if defined(__GNUC__) && !defined(VISIBLE)
#ifdef WIN32
#define VISIBLE __attribute__((visibility("default"))) __declspec(dllexport)
#else
#define VISIBLE __attribute__((visibility("default")))
#endif	//WIN32
#else
#define VISIBLE
#endif	//GNUC

#endif	//VISIBLE_H
