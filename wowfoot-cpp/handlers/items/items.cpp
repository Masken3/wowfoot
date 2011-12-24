#define __STDC_FORMAT_MACROS
#include "items.h"
//#include "dbcItemSubClass.h"
#include "dbcItemClass.h"
#include <stdio.h>
#include <inttypes.h>

void itemsChtml::getResponse2(const char* urlPart, DllResponseData* drd, ostream& os) {
	gItemClasses.load();
	for(size_t i=0; i<gItemClasses.size(); i++) {
		printf("class %"PRIuPTR": %s\n", i, gItemClasses[i].name);
	}
#if 0
	gItemSubClasses.load();
	for(ItemSubClasses::citr itr = gItemSubClasses.begin(); itr != gItemSubClasses.end(); itr++) {
		const ItemSubClass& isc(itr->second);
		printf("subClass %i/%i: %i %s %s\n",
			isc.itemClass, isc.subClass, isc.hands, isc.name, isc.plural);
	}
#endif

	drd->code = run(os);
}
