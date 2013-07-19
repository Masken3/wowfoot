#define __STDC_FORMAT_MACROS
#include "skill.h"
#include "Spell.index.h"
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include "dbcLock.h"
#include "lockEnums.h"

void skillChtml::httpArgument(const char* key, const char* value) {
	if(*value == 0)
		return;
	printf("p %s: %s\n", key, value);
}

void skillChtml::getResponse2(const char* urlPart, DllResponseData* drd, ostream& os) {
	gLocks.load();
	LockIndex::load();
	gSkillLines.load();
	gObjects.load();

	printf("urlPart: %s\n", urlPart);
	getArguments(drd);

#if 0
	// dump locks
	//printf("%" PRIuPTR " locks:\n", gLocks.size());
	for(auto itr = gLocks.begin(); itr != gLocks.end(); ++itr) {
		const Lock& l(itr->second);
		printf("%i:", l.id);
		for(uint i=0; i<ARRAY_SIZE(l.e); i++) {
			const Lock::Entry& e(l.e[i]);
			if(e.type != LOCK_KEY_NONE) {
				printf(" %i.%i(%i)", e.type, e.index, e.skill);
			}
		}
		printf("\n");
	}
#endif

	mSL = parseSkillId(os, urlPart);
	if(!mSL) {
		mTitle = urlPart;
		drd->code = 404;
		return;
	}
	mTitle = mSL->name;
}

void skillChtml::title(ostream& stream) {
	stream << mTitle;
}

skillChtml::skillChtml() : PageContext("Skill") {}

skillChtml::~skillChtml() {
}
