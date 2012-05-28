#include "pageContext.h"
#include "chtmlUtil.h"
#include "dbcAchievement.h"
#include "tabs.h"
#include "db_achievement_reward.h"
#include "dbcCharTitles.h"
#include "db_item.h"
#include "db_creature_template.h"

class achievementChtml : public tabsChtml, public PageContext {
public:
	achievementChtml();
	void title(ostream&);
	int runPage(ostream&);
	void getResponse2(const char* urlPart, DllResponseData* drd, ostream& os);

	const char* mTitle;
	const Achievement* a;
	const AchievementReward* ar;

	const char* FACTION(int id);
};
