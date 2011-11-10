#include "dll.h"
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef NULL
#define NULL 0
#endif

Dll::Dll() {
	mHandle = NULL;
}

Dll::~Dll() {
	close();
}

bool Dll::open(const char* fileName) {
	mHandle = dlopen(fileName, RTLD_LOCAL | RTLD_NOW);
	return mHandle != NULL;
}

void* Dll::get(const char* functionName) {
	return dlsym(mHandle, functionName);
}

void Dll::close() {
	if(mHandle != NULL) {
		int res = dlclose(mHandle);
		if(res != 0) {
			printf("dlclose error: %i\n", res);
			exit(res);
		}
		mHandle = NULL;
	}
}
