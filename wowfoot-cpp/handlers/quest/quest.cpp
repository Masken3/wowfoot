#define __STDC_FORMAT_MACROS
#include "quest.chtml.h"
#include "comments.h"
#include "db_creature_template.h"
#include "money.h"
#include "dbcSpell.h"
#include "db_item.h"
#include "dbcQuestFactionReward.h"
#include <stdio.h>
#include <stdlib.h>

using namespace std;

void questChtml::getResponse2(const char* urlPart, DllResponseData* drd, ostream& os) {
	gCreatureQuestGivers.load();
	gObjectQuestGivers.load();
	printf("Loading finishers...\n");
	gCreatureQuestFinishers.load();
	gObjectQuestFinishers.load();
	printf("Finishers loaded.\n");
	gFactions.load();
	gQuestFactionRewards.load();
	gNpcs.load();
	gItems.load();
	gSpells.load();
	gQuests.load();

	int id = toInt(urlPart);
	a = gQuests.find(id);
	if(a) {
		mTitle = a->title;

		int rewMoney = a->rewardMoneyMaxLevel;
		if(a->level >= 65)
			mRewXp = rewMoney / 6;
		else if(a->level == 64)
			mRewXp = int(rewMoney / 4.8);
		else if(a->level == 63)
			mRewXp = int(rewMoney / 3.6);
		else if(a->level == 62)
			mRewXp = int(rewMoney / 2.4);
		else if(a->level == 61)
			mRewXp = int(rewMoney / 1.2);
		else //if(a->level <= 60)
			mRewXp = int(rewMoney / 0.6);

		mTabs.push_back(getComments("quest", id));
	} else {
		mTitle = urlPart;
		drd->code = 404;
	}
}

int questChtml::rewRepValue(int index) {
	if(a->rewardFactionValueIdOverride[index] == 0) {
		int id = a->rewardFactionValueId[index];
		EASSERT(abs(id) <= 9);
		if(id >= 0)
			return gQuestFactionRewards[1].rep[id].value;
		else	// (id < 0)
			return gQuestFactionRewards[2].rep[-id].value;
	} else {
		return a->rewardFactionValueIdOverride[index] / 100;
	}
}

void questChtml::title(ostream& stream) {
	ESCAPE(mTitle);
}
