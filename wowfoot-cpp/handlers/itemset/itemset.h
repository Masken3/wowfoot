#include "pageContext.h"
#include "chtmlUtil.h"
#include "dbcItemSet.h"
#include "dbcSpell.h"
#include "tabs.h"
#include "db_item.h"
#include "util/numof.h"
#include "item_shared.h"

class itemsetChtml : public tabsChtml, public PageContext {
public:
	itemsetChtml();
	void title(ostream&);
	int runPage(ostream&);
	void getResponse2(const char* urlPart, DllResponseData* drd, ostream& os);

	const char* mTitle;
	const ItemSet* a;
};
