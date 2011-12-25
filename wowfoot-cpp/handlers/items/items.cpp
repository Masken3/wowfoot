#define __STDC_FORMAT_MACROS
#include "items.h"
#include <stdio.h>
#include <inttypes.h>

void itemsChtml::getResponse2(const char* urlPart, DllResponseData* drd, ostream& os) {
	gItemClasses.load();
	gItemSubClasses.load();
#if 0
	for(size_t i=0; i<gItemClasses.size(); i++) {
		printf("class %"PRIuPTR": %s\n", i, gItemClasses[i].name);
	}
	for(size_t i=0; i<gItemSubClasses.size(); i++) {
		for(size_t j=0; j<gItemSubClasses[i].size(); j++) {
			const ItemSubClass& isc(gItemSubClasses[i][j]);
			printf("subClass %"PRIuPTR"/%"PRIuPTR": %i %s %s\n",
				i, j, isc.hands, isc.name, isc.plural);
		}
	}
#endif

	drd->code = run(os);
}
