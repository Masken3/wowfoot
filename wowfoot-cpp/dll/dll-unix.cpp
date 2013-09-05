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
	if(!mHandle) {
		printf("dlerror \"%s\"\n", dlerror());
	}
	return mHandle != NULL;
}

void* Dll::get(const char* functionName) {
	void* res = dlsym(mHandle, functionName);
	if(!res) {
		printf("dlsym(%s) error: %s\n", functionName, dlerror());
	}
	return res;
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
