#define __STDC_FORMAT_MACROS
#include "title.chtml.h"
#include "comments.h"
#include "dbcAchievement.h"
#include "tabs.h"
#include "db_achievement_reward.h"
#include "dbcCharTitles.h"

using namespace std;

void titleChtml::getResponse2(const char* urlPart, DllResponseData* drd, ostream& os) {
	gTitles.load();
	gAchievements.load();
	gAchievementRewards.load();

	int id = toInt(urlPart);
	a = gTitles.find(id);
	if(a) {
		mTitle = a->name;

		mTabs.push_back(getComments("title", id));
	} else {
		mTitle = urlPart;
		drd->code = 404;
	}
}

void titleChtml::title(ostream& stream) {
	ESCAPE(mTitle);
}
