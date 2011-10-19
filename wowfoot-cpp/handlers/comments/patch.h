#ifndef PATCH_H
#define PATCH_H

#include <inttypes.h>

struct PatchVersion {
	const char* name;
	int64_t time;
};

extern const PatchVersion gPatchVersions[];
extern const int gnPatchVersions;

#endif	//PATCH_H
