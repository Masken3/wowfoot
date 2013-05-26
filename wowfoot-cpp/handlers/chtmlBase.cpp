#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <string>
#include <stdio.h>
#include "dllInterface.h"
#include "chtmlBase.h"

using namespace std;

// must not attempt to delete[] or free() the drd itself.
extern "C"
void cleanup(DllResponseData* drd) {
	printf("cleanup. code %i. size: %" PRIu64 "\n", drd->code, drd->size);
	string* s = (string*)drd->user;
	delete s;
}

ChtmlContext::~ChtmlContext() {
}
