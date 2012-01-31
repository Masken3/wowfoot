#include "chtmlBase.h"
#include "chtmlUtil.h"
#include "dbcItemSet.h"
#include "tabs.h"
#include "db_item.h"
#include "util/numof.h"
#include "item_shared.h"

class itemsetChtml : public tabsChtml, public PageContext {
public:
	int run(ostream& stream);
	void getResponse2(const char* urlPart, DllResponseData* drd, ostream& os);

	const char* mTitle;
	const ItemSet* a;
};
