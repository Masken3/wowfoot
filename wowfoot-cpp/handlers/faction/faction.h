#include "pageContext.h"
#include "chtmlUtil.h"
#include "tabTable.h"
#include "dbcFaction.h"

class factionChtml : public tabsChtml, public PageContext {
public:
	factionChtml();
	void title(ostream&);
	int runPage(ostream&);
	void getResponse2(const char* urlPart, DllResponseData* drd, ostream&);
private:
	string mTitle;
	const Faction* a;
};
