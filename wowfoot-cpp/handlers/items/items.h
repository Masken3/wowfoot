#include "pageContext.h"
#include "chtmlUtil.h"
#include "db_item.h"
#include "dbcItemSubClass.h"
#include "dbcItemClass.h"
#include "itrPair.h"
#include "item_shared.h"

class itemsChtml : public PageContext {
public:
	itemsChtml();
	void title(ostream&);
	int runPage(ostream&);
	void getResponse2(const char* urlPart, DllResponseData* drd, ostream& os);
	void httpArgument(const char* key, const char* value);
	virtual ~itemsChtml();
private:
	int mItemClass, mSubClass;
	ItrPair<Item>* mPair;
};
