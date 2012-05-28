#include "pageContext.h"
#include "chtmlUtil.h"
#include "tabTable.h"
#include "dbcCharTitles.h"
#include "db_achievement_reward.h"
#include "dbcAchievement.h"

class titleChtml : public tabsChtml, public PageContext {
public:
	titleChtml();
	void title(ostream&);
	int runPage(ostream&);
	void getResponse2(const char* urlPart, DllResponseData* drd, ostream&);
private:
	string mTitle;
	const Title* a;
};
