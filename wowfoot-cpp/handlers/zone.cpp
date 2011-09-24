#include <dllInterface.h>
#include <string.h>
#include <stdlib.h>

extern "C"
void getResponse(const char* urlPart, DllResponseData* drd) {
	drd->size = strlen(urlPart)+1;
	drd->text = malloc(drd->size);
	strcpy((char*)drd->text, urlPart);
	//strcat((char*)drd->text, urlPart);
	drd->code = 200;
}

// must not attempt to delete[] or free() \a drd.
extern "C"
void cleanup(DllResponseData* drd) {
	free(drd->text);
}
