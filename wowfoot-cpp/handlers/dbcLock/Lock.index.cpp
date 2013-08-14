#define __STDC_FORMAT_MACROS
#include "Lock.index.h"
#include "dbcLock.h"
#include "db_item.h"
#include "db_gameobject_template.h"
#include "db_creature_template.h"
#include <inttypes.h>
#include <stdio.h>
#include <unordered_set>
#include "util/criticalSection.h"
#include "util/arraySize.h"
#include "util/stl_map_insert.h"
#include "lockEnums.h"
#include "win32.h"

using namespace std;
using namespace LockIndex;

static CriticalSectionLoadGuard sCS;
static LockMap sLockMap;
static NpcMap sNpcMap;
static ObjectMap sObjectMap;
static ItemMap sItemMap;

void LockIndex::load() {
	LOCK_AND_LOAD;

	gLocks.load();
	gNpcs.load();
	gObjects.load();
	gItems.load();

	for(Locks::citr itr = gLocks.begin();
		itr != gLocks.end(); ++itr)
	{
		const Lock& l(itr->second);
		for(uint i=0; i<ARRAY_SIZE(l.e); i++) {
			const Lock::Entry& e(l.e[i]);
			if(e.type == LOCK_KEY_SKILL)
				insert(sLockMap, e.index, &l);
		}
	}
	printf("LockIndex: Loaded %" PRIuPTR " rows into %s\n",
		sLockMap.size(), "sLockMap");
	printf("LockIndex: Loaded %" PRIuPTR " rows into %s\n",
		sObjectMap.size(), "sObjectMap");
	printf("LockIndex: Loaded %" PRIuPTR " rows into %s\n",
		sNpcMap.size(), "sNpcMap");
	printf("LockIndex: Loaded %" PRIuPTR " rows into %s\n",
		sItemMap.size(), "sItemMap");
}

LockPair LockIndex::findLock(int type) {
	EASSERT(!sLockMap.empty());
	return sLockMap.equal_range(type);
}

NpcPair LockIndex::findNpc(int type) {
	EASSERT(!sNpcMap.empty());
	return sNpcMap.equal_range(type);
}
