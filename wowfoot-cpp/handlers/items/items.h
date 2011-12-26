#include "chtmlBase.h"
#include "chtmlUtil.h"
#include "db_item.h"
#include "dbcItemSubClass.h"
#include "dbcItemClass.h"
#include "itrPair.h"

class itemsChtml : public PageContext {
public:
	int run(ostream& stream);
	void getResponse2(const char* urlPart, DllResponseData* drd, ostream& os);
	void httpArgument(const char* key, const char* value);
	itemsChtml();
	virtual ~itemsChtml();
private:
	int mItemClass, mSubClass;
	ItrPair<Item>* mPair;
};
