#include "chtmlBase.h"
#include "chtmlUtil.h"
#include "tabTable.h"
#include "db_item.h"
#include "db_quest.h"
#include "dbcSpell.h"
#include "dbcFaction.h"
#include "money.h"

class questChtml : public tabsChtml, public PageContext {
public:
	int run(ostream&);
	void getResponse2(const char* urlPart, DllResponseData* drd, ostream&);
private:
	string mTitle;
	const Quest* a;
	int mRewXp;

	int rewRepValue(int index);
};
