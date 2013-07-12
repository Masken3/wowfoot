#define __STDC_FORMAT_MACROS
#include "skill.h"
#include "Spell.index.h"
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include "dbcSkillLine.h"
#include "dbcLock.h"
#include "lockEnums.h"
#include "skillShared.h"
#include "Lock.index.h"
#include "db_gameobject_template.h"

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

	const SkillLine* sl = parseSkillId(os, urlPart);
	if(!sl) {
		mTitle = urlPart;
		drd->code = 404;
		return;
	}
	mTitle = sl->name;

	printf("Skill %s\n", sl->name);
	int lockType = lockTypeFromSkill(sl->id);
	int count = 0;
	for(auto p = LockIndex::findLock(lockType); p.first != p.second; ++p.first) {
		const Lock& l(*p.first->second);
		printf("%i: level %i\n", l.id, l.e[0].skill);
		count++;
		for(auto op = gObjects.findLock(l.id); op.first != op.second; ++op.first) {
			const Object& o(*op.first->second);
			printf("object %i\n", o.entry);
		}
	}
	printf("Found %i locks.\n", count);
}

void skillChtml::title(ostream& stream) {
	stream << mTitle;
}

skillChtml::skillChtml() : PageContext("Skill"), mPair(NULL) {}

skillChtml::~skillChtml() {
	if(mPair)
		delete mPair;
}
