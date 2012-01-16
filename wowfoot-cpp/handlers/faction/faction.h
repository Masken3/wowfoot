#include "chtmlBase.h"
#include "chtmlUtil.h"
#include "tabTable.h"
#include "dbcFaction.h"

class factionChtml : public tabsChtml, public PageContext {
public:
	int run(ostream&);
	void getResponse2(const char* urlPart, DllResponseData* drd, ostream&);
private:
	string mTitle;
	const Faction* a;
};
