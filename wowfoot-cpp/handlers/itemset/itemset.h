#include "chtmlBase.h"
#include "chtmlUtil.h"
#include "dbcItemSet.h"
#include "tabs.h"
#include "db_item.h"

class itemsetChtml : public tabsChtml, public PageContext {
public:
	int run(ostream& stream);
	void getResponse2(const char* urlPart, DllResponseData* drd, ostream& os);

	const char* mTitle;
	const ItemSet* a;
};
