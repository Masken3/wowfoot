#include "achievement.chtml.h"
#include "comments.h"

#include <string.h>
#include <stdlib.h>
#include <string>
#include <sstream>

using namespace std;

void achievementChtml::getResponse2(const char* urlPart, DllResponseData* drd, ostream& os) {
	gAchievements.load();
	gAchievementRewards.load();
	gTitles.load();
#if 0
	int points = 0;
	for(Achievements::citr itr = gAchievements.begin(); itr != gAchievements.end(); ++itr) {
		points += itr->second.points;
		printf("%i: %i points. (%s)\n", itr->first, itr->second.points, itr->second.name);
	}
	printf("%i points total.\n", points);
#endif

	int id = toInt(urlPart);
	a = gAchievements.find(id);
	ar = gAchievementRewards.find(id);
	if(a) {
		mTitle = a->name;
		mTabs.push_back(getComments("achievement", id));
	} else {
		mTitle = urlPart;
		drd->code = 404;
	}
	drd->code = run(os);
}

const char* achievementChtml::FACTION(int id) {
	switch(id) {
	case -1: return "Both";
	case 1: return "Alliance";
	case 0: return "Horde";
	default: return "Bad FACTION";
	}
}
