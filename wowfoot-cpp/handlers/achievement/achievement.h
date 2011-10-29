#include "chtmlBase.h"
#include "chtmlUtil.h"
#include "dbcAchievement.h"
#include "tabs.h"

class achievementChtml : public tabsChtml, public PageContext {
public:
	int run(ostream& stream);
	void getResponse2(const char* urlPart, DllResponseData* drd, ostream& os);

	const char* mTitle;
	const Achievement* a;

	const char* FACTION(int id);
};
