#include "items.h"

void itemsChtml::getResponse2(const char* urlPart, DllResponseData* drd, ostream& os) {
	drd->code = run(os);
}
