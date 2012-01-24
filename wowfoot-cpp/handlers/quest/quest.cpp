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

		int rewMoney = a->rewMoneyMaxLevel;
		if(a->questLevel >= 65)
			mRewXp = rewMoney / 6;
		else if(a->questLevel == 64)
			mRewXp = int(rewMoney / 4.8);
		else if(a->questLevel == 63)
			mRewXp = int(rewMoney / 3.6);
		else if(a->questLevel == 62)
			mRewXp = int(rewMoney / 2.4);
		else if(a->questLevel == 61)
			mRewXp = int(rewMoney / 1.2);
		else //if(a->questLevel <= 60)
			mRewXp = int(rewMoney / 0.6);

		mTabs.push_back(getComments("quest", id));
	} else {
		mTitle = urlPart;
		drd->code = 404;
	}

	drd->code = run(os);
}

int questChtml::rewRepValue(int index) {
	if(a->rewRepValue[index] == 0) {
		int id = a->rewRepValueId[index];
		printf("rewRepValueId %i\n", id);
		EASSERT(abs(id) <= 9);
		if(id >= 0)
			return gQuestFactionRewards[1].rep[id].value;
		else	// (id < 0)
			return gQuestFactionRewards[2].rep[-id].value;
	} else {
		return a->rewRepValue[index] / 100;
	}
}
