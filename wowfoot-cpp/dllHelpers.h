
#ifdef WIN32
#define VISIBLE __attribute__((visibility("default"))) __declspec(dllexport)
#else
#define VISIBLE __attribute__((visibility("default")))
#endif
