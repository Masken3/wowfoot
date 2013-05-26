#define __STDC_FORMAT_MACROS
#include "items.h"
#include <stdio.h>
#include <inttypes.h>
#include <string.h>

void itemsChtml::httpArgument(const char* key, const char* value) {
	if(*value == 0)
		return;
	printf("p %s: %s\n", key, value);
	if(strcmp(key, "itemClass") == 0) {
		mItemClass = toInt(value);
	}
	if(strcmp(key, "subClass") == 0) {
		mSubClass = toInt(value);
	}
}

void itemsChtml::getResponse2(const char* urlPart, DllResponseData* drd, ostream& os) {
	gItemClasses.load();
	gItemSubClasses.load();
#if 0
	for(size_t i=0; i<gItemClasses.size(); i++) {
		printf("class %" PRIuPTR ": %s\n", i, gItemClasses[i].name);
	}
	for(size_t i=0; i<gItemSubClasses.size(); i++) {
		for(size_t j=0; j<gItemSubClasses[i].size(); j++) {
			const ItemSubClass& isc(gItemSubClasses[i][j]);
			printf("subClass %" PRIuPTR "/%" PRIuPTR ": %i %s %s\n",
				i, j, isc.hands, isc.name, isc.plural);
		}
	}
#endif
	mItemClass = -1;
	mSubClass = -1;
	getArguments(drd);

	if(mItemClass >= 0) {
		gItems.load();
		if(mSubClass < 0) {
			mPair = new ItrPairImpl<Item, Items::ClassItr>(gItems.findClass(mItemClass));
		} else {
			mPair = new ItrPairImpl<Item, Items::ClassSubclassItr>(gItems.findClassSubclass(mItemClass, mSubClass));
		}
	} else if(mSubClass >= 0) {
		throw Exception("subClass without itemClass");
	}

#if 0
	if(mPair) {
		int count = 0;
		while(mPair->hasNext()) {
			const Item& i(mPair->next());
			printf("Item %i (%s)\n", i.entry, i.name.c_str());
			count++;
		}
		printf("count: %i\n", count);
		Items::citr itr = gItems.begin();
		while(count < 10) {
			const Item& i(itr->second);
			printf("Item %i class %i (%s)\n", i.entry, i.class_, i.name.c_str());
			count++;
			++itr;
		}
	}
#endif
}

void itemsChtml::title(ostream& stream) {
}

itemsChtml::itemsChtml() : PageContext("Items"), mPair(NULL) {}

itemsChtml::~itemsChtml() {
	if(mPair)
		delete mPair;
}
