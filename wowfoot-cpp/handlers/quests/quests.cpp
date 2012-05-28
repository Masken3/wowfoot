#define __STDC_FORMAT_MACROS
#include "quests.h"
#include <stdio.h>
#include <inttypes.h>
#include <string.h>

void questsChtml::httpArgument(const char* key, const char* value) {
	if(*value == 0)
		return;
	printf("p %s: %s\n", key, value);
}

struct QuestMaxRepTest {
	static bool have(const Quest& q) {
		return q.requiredMaxRepFaction != 0;
	}
};

void questsChtml::getResponse2(const char* urlPart, DllResponseData* drd, ostream& os) {
	gQuests.load();
	gFactions.load();

	getArguments(drd);

	mPair = new SimpleItrPair<Quests, QuestMaxRepTest>(gQuests.begin(), gQuests.end());
}

void questsChtml::title(std::ostream& os) {
}

questsChtml::questsChtml() : PageContext("Quests"), mPair(NULL) {}

questsChtml::~questsChtml() {
	if(mPair)
		delete mPair;
}
